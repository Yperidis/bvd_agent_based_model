
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
	static CGuard g;   // Speicherbereinigung
	if (!_instance){
		System::reader = reader;
		BVDSettings::sharedInstance(reader);
		double t_start = reader->GetReal("simulation", "t_start", 0.0);

		double dt_log  = reader->GetReal("simulation", "dt_log", 1.0);
		double dt_output = reader->GetReal("simulation", "dt_output", 100.0);
		std::string dtmanage = reader->Get("trade" , "tradeRegularity", "DAILY");
		double dt_manage;
		if(dtmanage.compare("DAILY") == 0){
			dt_manage = bvd_const::tradingTimeIntervall.DAILY;
		}else if(dtmanage.compare("WEEKLY") == 0){
			dt_manage = bvd_const::tradingTimeIntervall.WEEKLY;
		}else if(dtmanage.compare("BIWEEKLY") == 0){
			dt_manage = bvd_const::tradingTimeIntervall.BIWEEKLY;
		}else if(dtmanage.compare("MONTHLY") == 0){
			dt_manage = bvd_const::tradingTimeIntervall.MONTHLY;
		}else if(dtmanage.compare("QUARTERLY") == 0){
			dt_manage = bvd_const::tradingTimeIntervall.QUARTERYEARLY;
		}else if(dtmanage.compare("HALFYEARLY") == 0){
			dt_manage = bvd_const::tradingTimeIntervall.HALFYEARLY;
		}else if(dtmanage.compare("YEARLY") == 0){
			dt_manage = bvd_const::tradingTimeIntervall.YEARLY;
		}


	   	_instance = new System( t_start ,  dt_log ,  dt_output,  dt_manage);
	   	//Step 4: Set custom log and write intervals (if desired) and start the simulation
		_instance->set_log_interval(dt_log);
		_instance->set_write_interval(dt_output);
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
  for(auto farm: this->farms)
  	delete farm;
	delete this->activeStrategy;

//  delete output;
}
double System::current_time(){ return _current_time; }

void System::schedule_event( Event* e )
{

  // (1) put the event into the main queue
  queue.push( e );
  // (2) find the farm that this event pertains to and register the event there if it is an infection rate changing event.
	if( this->queue.top()->type == Event_Type::INFECTION && e->id == this->queue.top()->id){ //the event just being processed is from
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
      if ( c != NULL ) //Actually, at this point, c==NULL should NEVER happen!!
	{

		if(c->herd != NULL && c->herd->farm != NULL)
	  c->herd->farm->register_future_infection_rate_changing_event( e );
	  c->register_future_infection_rate_changing_event( e );
	}
    }


}
void System::scheduleFutureCowIntros(){
	int num = System::reader->GetInteger("modelparam","inputCowNum", 0);
	int no;
  	std::ostringstream oss;
  	std::string cowName;
  	Cow *c;
  	Farm* f;
  	std::vector<Farm*>::iterator it;
  	int farmID;
  	double intTime;

  	std::cout << "scheduling intros of " << num << " cows"<< std::endl;
	for(int a=0;a < num;++a){
		f = NULL;
		no = a + 1;
		oss << "inputCow" << no;
		cowName = oss.str();
		c = new Cow(cowName);
		intTime = System::reader->GetReal(cowName, "introductiontime", 500);
		farmID = System::reader->GetInteger(cowName, "farmID", 1);
		if(farmID >= this->farms.size() || farmID < 0){
			std::cout << "the farm you wanted input cow " << no << "to(" << farmID << ") doesn't exist" << std::endl;
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
  if (queue.empty())
    {
      std::cout << "No more events in queue!" << std::endl << std::endl;
      return;
    }

  no_of_events_processed++;

  Event* e = queue.top();

  queue.pop();
  if (e->execution_time < _current_time){
    std::cerr << "Error, got an event that is earlier than the current time. Exiting" << std::endl;
  	Utilities::pretty_print(e, std::cout);

  }
//  Cow* c = Cow::get_address( e->id );
//  Farm *f = NULL;
//  if(c != NULL)
//	f = c->herd->farm;
//  std::unordered_set< Event*>::const_iterator got_it = invalidated_events.find( e );
//  if ( got_it != invalidated_events.end() ) // => e is invalid.
//      invalidated_events.erase( got_it );
  if(e->valid) // => e is valid.
    {
	  Cow* c = Cow::get_address( e->id );
      _current_time = e->execution_time;
     if(e->is_trade_event())
		this->output->logEvent(e);
      switch ( e->dest )
	{
	case Destination_Type::COW:
	  {

	    if ( c != NULL  && c->id() == e->id){
		    c->execute_event( e );
  		} //Event is not pertaining to a dead cow. Could  this happen?
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
	if(!e->is_trade_event())
		this->output->logEvent(e);
	if(e->type == Event_Type::DEATH || e->type == Event_Type::CULLING || e->type == Event_Type::SLAUGHTER )
		delete c;
    }
//   if(f != NULL &&e->is_infection_rate_changing_event())
//    	f->delete_infection_rate_change_event(e);
	if(e->is_infection_rate_changing_event())
		memorySaveQ.push(e);
	else
		delete e;


	Event* event;
	while(memorySaveQ.size() > 0 && ( (event = memorySaveQ.top()) != nullptr) &&(event->execution_time + 500. < this->_current_time )){
		delete event;
		memorySaveQ.pop();
	}

}

void System::invalidate_event( Event* e )
{
	e->valid = false;
  //invalidated_events.insert( e );
  if(memorySaveQ.size() > 10000000){
  	std::cout << (int) e->type << std::endl;
  		Utilities::printStackTrace(15);
  	}
}

void System::register_farm( Farm* f )
{
  farms.push_back( f );
  if(f->getType() == WELL)
  	this->market->registerSourceFarm((CowWellFarm*) f);
  else if(f->getType() == SLAUGHTERHOUSE)
  	this->market->registerSlaughterHouse((Slaughterhouse*) f);
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

  schedule_event( new System_Event( end_time ,                  Event_Type::STOP ) );
  schedule_event( new System_Event( _current_time + _dt_log   , Event_Type::LOG_OUTPUT   ) );
  schedule_event( new System_Event( _current_time + _dt_write , Event_Type::WRITE_OUTPUT ) );
  schedule_event( new System_Event( _current_time + _dt_manage , Event_Type::MANAGE ) );
  if(this->mySettings->strategies.size() > 0)
	schedule_event( new System_Event( this->mySettings->strategies.top()->startTime, Event_Type::ChangeContainmentStrategy));
  stop=false;
  while( !(stop || queue.empty())  )
    execute_next_event();

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
			schedule_event(new System_Event( _current_time + _dt_write , Event_Type::WRITE_OUTPUT ) );

			break;
    case Event_Type::MANAGE:
    	for (auto farm : farms){

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
