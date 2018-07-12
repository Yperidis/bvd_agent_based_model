#include <FarmManager.h>
#include <CowWellFarmManager.h>
#include "System.h"
#include "Events.h"
#include "Farm.h"
#include "Cow.h"
#include "Herd.h"
#include "AdvancedOutput.h"
#include "Utilities.h"
#include "Market.h"
#include "projectImports/inih/cpp/INIReader.h"
#include "Events.h"
#include "BVDContainmentStrategy.h"
#include "BVDSettings.h"
System* System::_instance = 0;
INIReader* System::reader = 0;
System* System::getInstance(INIReader* reader){
	static CGuard g;   // Garbage Collection (Speicherbereinigung)
	if (!_instance){
		System::reader = reader;
		BVDSettings::sharedInstance(reader);
		double t_start = reader->GetReal("simulation", "t_start", 0.0);

		double dt_log  = reader->GetReal("simulation", "dt_log", 1.0);
		double dt_output = reader->GetReal("simulation", "dt_output", 100.0);
        //TODO Does this feature work as it should when set to true in parallel to PercentagesShallBeAppliedonWells? See CowWellFarmManager.cpp
		bool dynamic_reintroduction = reader->GetBoolean("modelparam", "PercentagesShallBeAppliedOnWellsDynamically", false);
		std::string dtmanage = reader->Get("trade" , "tradeRegularity", "DAILY");
		double dt_manage = bvd_const::tradingTimeIntervall.DAILY;
		if(dtmanage == "WEEKLY"){
			dt_manage = bvd_const::tradingTimeIntervall.WEEKLY;
		}else if(dtmanage == "BIWEEKLY"){
			dt_manage = bvd_const::tradingTimeIntervall.BIWEEKLY;
		}else if(dtmanage == "MONTHLY"){
			dt_manage = bvd_const::tradingTimeIntervall.MONTHLY;
		}else if(dtmanage == "QUARTERLY"){
			dt_manage = bvd_const::tradingTimeIntervall.QUARTERYEARLY;
		}else if(dtmanage == "HALFYEARLY"){
			dt_manage = bvd_const::tradingTimeIntervall.HALFYEARLY;
		}else if(dtmanage == "YEARLY"){
			dt_manage = bvd_const::tradingTimeIntervall.YEARLY;
		}


		//TODO Check for memory leaks here and in the System function below
	   	_instance = new System( t_start, dt_log, dt_output, dt_manage );
	   	//Step 4: Set custom log and write intervals (if desired) and start the simulation
		_instance->set_log_interval(dt_log);
		_instance->set_write_interval(dt_output);
		_instance->set_dynamic_reintroduction(dynamic_reintroduction);
		_instance->mySettings = BVDSettings::sharedInstance(reader);
		Cow::set_system(_instance);
	}
   return _instance;
}

System::System(double start_time , double dt_log , double dt_write, double dt_manage):_dt_manage(dt_manage)
{
	#ifdef _SYSTEM_DEBUG_
		std::cout << "creating system" << std::endl;
	#endif
	_current_time = start_time;
	queue        = Event_queue();

    //Here an unsigned integer is used so as to select a large seed number.
    //For the default case (-1), the outcome depends on the architecture
    //of the computer where the simulation will be run. For instance, in
    //a 32 bit architecture the complement of two for the number will be
    //generated, i.e. 2^32-1.
	unsigned int seed = System::reader->GetInteger("rng", "seed", -1);
	if(seed != -1){
		rng = Random_Number_Generator( seed );
	}else{
		rng = Random_Number_Generator(  );
	}

	set_log_interval(dt_log);
	set_write_interval( dt_write);
	//output       = new Output( output_filename , overwrite );
	output = new AdvancedOutput(*System::reader);
	no_of_events_processed =0;
	market = new Market(this);
  	#ifdef _DEBUG_
  		signal(SIGSEGV, handleSystemError);
      #endif
	_firstCowWellFarm = nullptr;
	this->activeStrategy = new BVDContainmentStrategy(BVDContainmentStrategyFactory::defaultStrategy);
}

System::~System()
{
	delete output;
	std::cout << "Instance of system is going to be deleted. Stats: " << no_of_events_processed ;
	std::cout << " events processed, " << Cow::total_number();
	std::cout << " cows went through the system." << std::endl;
	std::cout << "seed: " << this->rng.getSeed() << std::endl;
	delete market;
	for(auto farm: this->farms) {
		delete farm;
	}
	delete this->activeStrategy;

//  delete output;
}
double System::current_time(){ return _current_time; }


///This function places the event into the priority queue provided that some reasonable conditions are met
void System::schedule_event( Event* e )
{
    if (e->type == Event_Type::BIRTH) {
        Cow* c = Cow::get_address( e->id );
        if ( c != NULL ) //Actually, at this point, c==NULL could happen, because a trade can be scheduled after the offer for a cow that in the meantime has died.
        {
            c->planned_birth_event = e;
        }
    }

    Cow* c = Cow::get_address(e->id);
    if (e->type == Event_Type::ABORTION && c->calf_status == Calf_Status::NO_CALF) {
        std::cout << "Unreasonable situation" << std::endl;
    }

    if (e->type == Event_Type::BIRTH && c->planned_birth_event == nullptr) {
        std::cout << "Unreasonable situation" << std::endl;
    }
    // (1) put the event into the main queue
    queue.push( e );
    // (2) find the farm to which this event pertains and register the event there if it is an infection rate changing event.
    if( this->queue.top()->type == Event_Type::INFECTION && e->id == this->queue.top()->id){
        this->output->logResultingEventOfInfection(e);
    }
    if ( e->is_trade_event() )
    {
        Cow* c = Cow::get_address( e->id );
        if ( c != NULL ) //Actually, at this point, c==NULL could happen, because a trade can be scheduled after the offer for a cow that in the meantime has died.
        {

            e->farm->register_future_infection_rate_changing_event( e );
            if(c->herd != NULL && c->herd->farm != NULL)
                c->herd->farm->register_future_infection_rate_changing_event( e );

        }
    }
    else if ( e->is_infection_rate_changing_event() )
    { // So far, all infection_rate_changing events have dest == COW
        Cow* c = Cow::get_address( e->id );
        if ( c != nullptr ) //Actually, at this point, c==NULL should NEVER happen!!
        {
            ///If both the herd and the farm exist register a future infection rate changing event at the farm level
            if(c->herd != nullptr && c->herd->farm != nullptr) {
                c->herd->farm->register_future_infection_rate_changing_event( e );
            }
            ///...and register it at the cow level at any rate
            c->register_future_infection_rate_changing_event( e );
        }
    }
}
void System::scheduleFutureCowIntros(){
	int num = System::reader->GetInteger("modelparam","inputCowNum", 0);
	int no;


  	std::cout << "scheduling intros of " << num << " cows"<< std::endl;
	for(int a=0;a < num;++a){
		std::ostringstream oss;
		std::string cowName;
		Cow *c;
		Farm* f;
		std::vector<Farm*>::iterator it;
		int farmID;
		double intTime;
		f = NULL;
		no = a + 1;
		oss << "inputCow" << no;
		cowName = oss.str();
		c = new Cow(cowName);

		intTime = (double) System::reader->GetInteger(cowName, "introductiontime", 500);
		farmID = System::reader->GetInteger(cowName, "farmID", 1);
		if(farmID >= this->farms.size() || farmID < 0){
			std::cout << "the farm you wanted input cow " << no << " to (" << farmID << ") doesn't exist" << std::endl;
			delete c;
			continue;
		}
		for(it = this->farms.begin(); it != this->farms.end(); it++){
			f = *it;
			if(f->id +1 == farmID){//zero based hier, 1 based in der menschenwelt
				break;
			}

		}
		if(it != this->farms.end()){
			std::cout << "cow " << no << " is introduced to the system in farm " << farmID << " at time t=" << intTime << std::endl;
//			std::cout << f << std::endl;

			Trade_Event *e = new Trade_Event(intTime,c->id(),f);

			this->schedule_event(e);

		}else{
			delete c;
		}

	}
}

/*bool System::is_valid( Event* e)
{
  std::unordered_set< Event*>::const_iterator got_it = invalidated_events.find( e );
  return got_it != invalidated_events.end()  // => e is invalid.
  }*/
void System::execute_next_event()
{
    if (queue.empty()){
        std::cout << "No more events in queue!" << std::endl << std::endl;
        return;
    }

    no_of_events_processed++;    // event counter

    Event* e = queue.top();    // Accessing the top element (event) of the queue

    queue.pop();    // Removing the top element (event) from the queue
    if (e->execution_time < _current_time){    //the current time should be initialized from the ini file
        std::cerr << "Error, got an event that is earlier than the current time. Exiting" << std::endl;
        Utilities::pretty_print(e, std::cout);

    }

    if(e->valid) // => e is valid.
    {
        Cow* c = Cow::get_address( e->id );
        _current_time = e->execution_time;    //the current time becomes the execution time of the event from the main queue
        // Calls of events are always logged regardless of what is happening in their member functions
        this->output->logEvent(e);
        if(e->type == Event_Type::DEATH || e->type == Event_Type::CULLING || e->type == Event_Type::SLAUGHTER ) {
            delete c;    //Freeing the reserved memory of a born animal upon its culling, slaughter or other cause of death
        }
        else{
            switch ( e->dest )
            {
                case Destination_Type::COW:
                {
                    if ( c != nullptr  && c->id() == e->id){    // the cow has to exist and correspond to the event to be executed
                        c->execute_event( e );
                    } // Event does not pertain to a dead cow. Could  this happen?
                    break;
                }
                case Destination_Type::HERD:
                    e->herd->execute_event( e );
                    break;
                case Destination_Type::FARM:
                    e->farm->execute_event( e );
                    break;
                case Destination_Type::SYSTEM:
                    _execute_event( e );
            }
        }
        //According to Cow.cpp an event can only be invalid if
        //(1) A planned_birth_event in not nullptr with calf_status = Calf_Status::NO_CALF, i.e. a birth from a
        //    non pregnant cow has been scheduled.
        //(2) An end_of_vaccination_event is not nullptr for a susceptible animal with a successful vaccination working
        //    probability, i.e. there has been
        //
        //According to Farm.cpp an event can only be invalid if
        //(1) An infection rate change occurs
        //(2) A trade event is executed
    }
    else{
        //TODO Why is this sort of invalid event unacceptable? The possible initialization events are BIRTH and (first) INSEMINATION
        if (e->type != Event_Type::INFECTION && e->type != Event_Type::BIRTH) {
            std::cerr << "Error, got an event that is invalid and of type infection or birth. Exiting" << std::endl;
            Utilities::pretty_print(e, std::cout);
        }
    }

    if(e->is_infection_rate_changing_event()) {    // if the current event changed the infection rate, add it to a buffer priority queue
        memorySaveQ.push(e);
    }
    else {
        delete e;    // otherwise delete the running event (end of its course)
    }

    Event* event;
    while(memorySaveQ.size() > 0 && ( (event = memorySaveQ.top()) != nullptr) && (event->execution_time + 500. < this->_current_time )){
        delete event;    //delete the events of the buffer queue and its elements if they are scheduled for anytime less than the current time
                        //plus 500 and they are actual events (nullptr)
        memorySaveQ.pop();
    }

}

void System::invalidate_event( Event* e )
{
	e->valid = false;
	//invalidated_events.insert( e );
    //TODO Potential memory leak. Since the event has been invalidated, shouldn't we delete as well? Check the invalidation purpose at README.org
	if(memorySaveQ.size() > 10000000){
		std::cout << (int) e->type << std::endl;
		Utilities::printStackTrace(15);
	}
}

void System::register_farm( Farm* f )
{
  farms.push_back( f );
  if(f->getType() == WELL) {
	  _firstCowWellFarm = (CowWellFarmManager*) f->manager;
	  this->market->registerSourceFarm((CowWellFarm*) f);
  } else if(f->getType() == SLAUGHTERHOUSE) {
	  this->market->registerSlaughterHouse((Slaughterhouse*) f);
  }
  //output->set_number_of_farms( farms.size() );

}
void System::unregister_farm( Farm* f )
{
  std::vector<Farm*>::iterator got_it = std::find( farms.begin() , farms.end() , f );
  if ( got_it != farms.end()){

    farms.erase( got_it );

    }
  //output->set_number_of_farms( farms.size() );
}

void System::register_market( Market* m )
{
	if(market != NULL){
		delete market;}
  market = m;
}
void System::print_state( std::ostream& out )
{
  out << "********************************************************************************"<<std::endl;
  out << "System state at t="<<std::setprecision(3)<< _current_time << " days."             <<std::endl;
  out << "********************************************************************************"<<std::endl;
  out << "   Begin" <<std::endl;
  for ( auto const& f : farms )
      f->print_state( out );
  out << "   End"<<std::endl;
}


void System::dump_queue()
{
  std::cout<<"Dumping queue: "<<std::endl;
  if (queue.empty())
    {
      std::cout << "Queue is empty!"<<std::endl<<std::endl;
      return;
    }
  while(!queue.empty())
    {
      Event* e = queue.top();
      queue.pop();
      std::cout << "Execution time: "<< e->execution_time << ", type: " << Utilities::Event_tostr.at(e->type) << ", id: " << e->id<<". Is irc? " << e->is_infection_rate_changing_event() << std::endl;
    }
}

void System::print_next_event()
{
  if (queue.empty()) {return;}
  Event* e = queue.top();
  std::cout << "--------------------------- next Event: ----------------------------------------"<<std::endl;
  Utilities::pretty_print( e , std::cout );
  std::cout << "--------------------------------------------------------------------------------"<<std::endl;
}

void System::log_state()
{
	this->output->logFarms(_current_time, &(this->farms));
//  Output::Datapoint* state = new  Output::Datapoint();
//  for ( auto const& f : farms )
//    {
//      std::array< int , 4 >* temp = new std::array< int , 4 >();
//      f->		( temp );
//      state->push_back( temp );
//
//    }
//  output->log_datapoint( _current_time , state );
//  for ( auto t : *state )
//    delete t;
//  delete state;
}

void System::run_until( double end_time )
{

  schedule_event( new System_Event( end_time,                   Event_Type::STOP ) );
  schedule_event( new System_Event( _current_time + _dt_log,    Event_Type::LOG_OUTPUT   ) );
  schedule_event( new System_Event( _current_time + _dt_write,  Event_Type::WRITE_OUTPUT ) );
  schedule_event( new System_Event( _current_time + _dt_manage, Event_Type::MANAGE ) );
  if(this->mySettings->strategies.size() > 0)
	schedule_event( new System_Event( this->mySettings->strategies.top()->startTime, Event_Type::ChangeContainmentStrategy));
  stop=false;
  while( !( stop || queue.empty() ) ){    //The simulation continues until it either reaches the end time or the main event queue is empty
	  execute_next_event();
  }
}

void System::set_log_interval( double dt_log )
{
  if (dt_log > 0)
    _dt_log = dt_log;
}
void System::set_write_interval( double dt_write )
{
  if (dt_write > 0)
    _dt_write = dt_write;
}
double System::getCurrentTime(){
	return this->_current_time;
}
Market* System::getMarket(){
	return this->market;
}
void System::_execute_event( Event* e )
{

  switch( e->type )
    {
	// case Event_Type::EARTAG:
	// 	std::cout << "set eartag" << std::endl;
	// 	strategies.eartag = !strategies.eartag;
	// 	break;
	// case Event_Type::VACCINATION:
	// 	std::cout << "set vaccination" << std::endl;
	// 	strategies.vaccination = !strategies.vaccination;
	// 	break;
	// case Event_Type::JUNGTIER:
	// 	std::cout << "set jungtier" << std::endl;
	// 	strategies.jungtierfenster = !strategies.jungtierfenster;
	// 	if(strategies.jungtierfenster){
	//
	// 		schedule_event(new System_Event(e->execution_time + jungtierzeit,Event_Type::JUNGTIER_EXEC));
	// 	}
	// 	break;
	case Event_Type::JUNGTIER_EXEC:
		for (auto farm : farms){

	    	farm->jungtierCheck();

    	}
		if(this->activeStrategy->usesJungtierFenster){
			schedule_event(new System_Event(e->execution_time + this->activeStrategy->jungtierzeit,Event_Type::JUNGTIER_EXEC));
		}
		break;
	case Event_Type::ChangeContainmentStrategy:
		if(this->mySettings->strategies.size() > 0){
			delete this->activeStrategy;
			this->activeStrategy = this->mySettings->strategies.top();
			this->mySettings->strategies.pop();
			schedule_event( new System_Event( this->mySettings->strategies.top()->startTime, Event_Type::ChangeContainmentStrategy));
			if(this->activeStrategy->usesJungtierFenster){
				schedule_event(new System_Event(e->execution_time + this->activeStrategy->jungtierzeit,Event_Type::JUNGTIER_EXEC));
			}
		}
		break;
    case Event_Type::STOP:
    	output->write_to_file(_current_time);
      	stop = true;
      	break;
    case Event_Type::LOG_OUTPUT:

      log_state();
      schedule_event(new System_Event( _current_time + _dt_log , Event_Type::LOG_OUTPUT ) );
      break;
    case Event_Type::WRITE_OUTPUT:
			std::cout << "Writing file after " << _current_time << " days:\t" ;
			output->write_to_file(_current_time);
			std::cout << "Done. " << std::endl;
#ifdef _RUNNING_DEBUG_
            std::cout << "Total number of heads is " << this->countCows() << std::endl;
			std::cout << "Farm demands met: " << this->countHappyFarms() << std::endl;
            std::cout << "Event queue size is " << this->queue.size() << std::endl;
            std::cout << "PI prevalence at reintroduction is now " << this->getFirstWell()->printInfectionValues() << std::endl;
#endif
			schedule_event(new System_Event( _current_time + _dt_write , Event_Type::WRITE_OUTPUT ) );

			break;
    case Event_Type::MANAGE:
    	for (auto farm : farms){

			if (this->_dynamic_reintroduction) {    //This block regulates the introduction of PIs from the source farm
			    //according to the current PI population
				CowWellFarmManager* ptr =  dynamic_cast<CowWellFarmManager*> (farm->manager);
				if (ptr != nullptr) {
					std::tuple<double, double> res = calculatePrevalence();
					ptr->changeInfectionReplacement(std::get<0>(res), std::get<1>(res));
				}
			}

	    	farm->getManaged();

    	}
    	this->market->flushQueues();
    	#ifdef _SYSTEM_DEBUG_
				std::cout << "System: schedule new management event at " << (_current_time + _dt_manage)  << std::endl;
		#endif
		schedule_event( new System_Event( _current_time + _dt_manage , Event_Type::MANAGE ) );
		#ifdef _SYSTEM_DEBUG_
				std::cout << "System scheduled next management event" << std::endl;
		#endif
    break;
    default:
      break;
    }

}
void System::handleSystemError(int sig){
	std::cerr << "received an error at time " << System::_instance->_current_time << std::endl;
	fprintf(stderr, "Error: signal %d:\n", sig);
	Utilities::printStackTrace(15);
}

System::CGuard::~CGuard(){
	if(System::_instance != NULL){
		delete System::_instance;
		System::_instance = NULL;
	}
}

void System::addCow(Cow* c){
	this->output->logBirth(c);
}

//Testing
Event_queue System::getEventQueue(){
	return this->queue;
}

void System::set_dynamic_reintroduction(bool dynamic_reintroduction) {
	_dynamic_reintroduction = dynamic_reintroduction;
}

std::tuple<double, double> System::calculatePrevalence() {
	int num_pi = 0;
	int num_ti = 0;
	int num_heads = 0;
	std::vector<Farm*>::iterator it;
	Farm * f;

	for(it = this->farms.begin(); it != this->farms.end(); it++){
		f = *it;
		num_heads += f->total_number();
		num_pi += f->number_of_PI();
		num_ti += f->number_of_TI();
	}

	double ti;
	double pi;
	if (num_heads > 0) {
		ti = num_ti / (double) num_heads;
		pi = num_pi / (double) num_heads;
	} else {
		ti = 0.0;
		pi = 0.0;
	}

    #ifdef _MARKET_DEBUG_
        std::cout << "System: prevalence of ti is " << ti << "; pi prevalence is " << pi << std::endl;
    #endif

	return {ti, pi};
}

std::vector<Farm *> System::getFarms() {
	return this->farms;
}

int System::countCows() {
    int cows = 0;
    for (auto f : this->farms){
        if (f->myType != SLAUGHTERHOUSE) {
            cows += f->total_number();
        }
    }
    return cows;
}

std::string System::countHappyFarms() {
    int less_than = 0;
    int more_than = 0;
    int satisfied_farms = 0;
    int moreorless = 0;
    for (auto f : this->farms){
        if (f->myType != SLAUGHTERHOUSE) {
            int wanted_cows = *f->manager->plannedNumberOfCows;
            if (wanted_cows == f->total_number()) {
                satisfied_farms++;
            } else if (f->total_number() < (wanted_cows * 0.90)) {
                less_than++;
            } else if (f->total_number() > wanted_cows * 1.10) {
                more_than++;
            } else {
                moreorless++;
            }
        }
    }
    return "under: " + std::to_string(less_than) + ", OK: " + std::to_string(satisfied_farms) + ", over: " + std::to_string(more_than) + ", more or less OK: " + std::to_string(moreorless);
}

//Calls the existing well farm out of the farm list. If it doesn't exist
//the system gets nothing (null pointer).
CowWellFarmManager* System::getFirstWell() {
    if (_firstCowWellFarm == nullptr) {
        for(auto f: this->farms) {
            if (f->myType == WELL) {
                _firstCowWellFarm = dynamic_cast<CowWellFarmManager *> (f->manager);
            }
            return _firstCowWellFarm;
        }
    } else {
        return _firstCowWellFarm;
    }
}
