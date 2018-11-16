#include "Farm.h"
#include "Events.h"
#include "Cow.h"
#include "System.h"
#include "Herd.h"
#include "Model_Constants.h"
#include "BVD_Random_Number_Generator.h"
#include "Output.h"
#include "Utilities.h"
#include <limits>
#include <iostream>
#include <array>
#include "FarmManager.h"
#include "BVDContainmentStrategy.h"
int Farm::counter = 0;
Farm::Farm( System * s )
{
	this->id = Farm::counter++;
	#ifdef _DEBUG_
		std::cout << "creating basic farm" << std::endl;
	#endif
  system = s;
  quarantine = false;
  lastQuarantineEndEvent = nullptr;
  next_infection_event = nullptr;
//   life_expectancy_male_cow = 10;
}

Farm::~Farm()
{

}


double Farm::next_infection_rate_change( Event *current )
{
  // (1) We access the events in future_infection_rate_changing_events from the top
  //     until we find the current one (the one during the execution of which this method has been called.)
  //     The criterion is: e->execution_time==current_time && farm of that event == this.
  // (2) pop that event out of the queue
  // (3) the next event with farm==this has the time we want to know. Don't pop that event out of the queue.
  //     This is done, the next time this function is called.
  if ( future_infection_rate_changing_events.empty() )
	return std::numeric_limits<double>::max();

  const Event* event;
  event = future_infection_rate_changing_events.top();
  while( !its_the_one(event, current) && !future_infection_rate_changing_events.empty() )
    {
      future_infection_rate_changing_events.pop();
      event = future_infection_rate_changing_events.top();
    }
  if ( future_infection_rate_changing_events.empty() )
	return std::numeric_limits<double>::max();

  return event->execution_time;
}

void Farm::register_future_infection_rate_changing_event( Event* e )
{
  future_infection_rate_changing_events.push( e );
  if ( e->type == Event_Type::INFECTION )
    {
      	if (next_infection_event != nullptr){
	      	std::cerr << "More than one INFECTION event for a farm in the queue. This should not happen!! Aborting!!" << std::endl;
	      	std::cout << "The ID of the selected cow was " <<  e->id << std::endl;
      		Utilities::printStackTrace();
      		exit(1);
      }
		next_infection_event = e;
    }
}

void Farm::delete_infection_rate_change_event( Event* e){
	if(e == next_infection_event)
		next_infection_event = nullptr;
	Event_queue tmp;
	while(e != future_infection_rate_changing_events.top() && !future_infection_rate_changing_events.empty()){
		tmp.push(future_infection_rate_changing_events.top());
		future_infection_rate_changing_events.pop();
	}
	if(e == future_infection_rate_changing_events.top())
		future_infection_rate_changing_events.pop();
	while(!tmp.empty()) {
        future_infection_rate_changing_events.push(tmp.top());
        tmp.pop();
    }
}

void Farm::invalidate_next_infection_event()
{
  if ( next_infection_event == nullptr )
    return;
  system->invalidate_event( next_infection_event );
  next_infection_event = nullptr;
}

void Farm::getManaged(){
//	std::cout << this->system->getCurrentTime() << " d " << std::endl;
	this->manager->manage();
}

std::vector<Herd*>* Farm::getHerds(){

	return &(this->my_herds);

}

bool Farm::isUnderQuarantine(){
	return this->quarantine;
}

void Farm::putUnderQuarantine(){

	this->quarantine = true;
	double exTime = this->system->getCurrentTime() + System::getInstance(nullptr)->activeStrategy->quarantineTime;
	FARM_EVENT* e;
	e = new FARM_EVENT(exTime, Event_Type::QUARANTINEEND, this);

	if(this->lastQuarantineEndEvent == nullptr || this->lastQuarantineEndEvent->execution_time < e->execution_time)
		this->lastQuarantineEndEvent = e;

	this->system->schedule_event(e);
}

std::vector<Cow*>* Farm::getPIs(){
	int size = 0;

	for(auto herd : this->my_herds){
		size += herd->getPIs()->size();
	}
	std::vector<Cow*>* returnVec = new std::vector<Cow*>(size, nullptr);
	//returnVec->reserve(size);
	for(auto herd : this->my_herds){
		returnVec->insert( returnVec->end(), herd->getPIs()->begin(), herd->getPIs()->end() );
	}
	return returnVec;
}

FarmType Farm::getType(){
	return this->myType;
}
int Farm::number_of_PI() const
{
  int j=0;
  for ( auto const h : my_herds )
    j += h->getNumPI();
  return j;
}
int Farm::number_of_TI() const
{
  int j=0;
  for ( auto const h : my_herds )
    j += h->getNumTI();


  return j;
}
int Farm::number_of_R() const
{
  int j=0;
  for ( auto const h : my_herds )
    j += h->getNumR();
  return j;
}
int Farm::number_of_S() const
{
  int j=0;
  for ( auto const h : my_herds )
    j += h->getNumS();
  return j;
}
int Farm::total_number() const
{
  int j=0;
  for ( auto const h : my_herds )
    j += h->total_number();
  return j;
}

void Farm::infection_rate_has_changed( Event * e ){ // e points to the event by which this just happened.

    if ( e == next_infection_event)
        next_infection_event= nullptr;  // making space for the infection event queue of this farm after the current one's execution

    double time = e->execution_time;
    // (1) Calculate the individual infection rates of the herds, using the inter-herd transmission coefficients
    double ir_temp;
    std::unordered_map< Herd* , double> infection_rates;
    for ( auto const h_to : my_herds )
    {
        if(h_to->getNumS() == 0)
        {
            infection_rates[h_to] = 0;
            continue;
        }

        ir_temp  = 0;
        ir_temp += bvd_const::lambda_TI * h_to->getNumTI() / h_to->total_number();
        ir_temp += bvd_const::lambda_PI * h_to->getNumPI() / h_to->total_number();
        for ( auto const h_from : my_herds )
        {
            if (h_from == h_to ) {continue;}  // transmission coefficient of the same herd
            ir_temp += inter_herd_transmission_coefficients[h_to][h_from] * h_from->getNumPI() / ( h_to->total_number() * h_from->total_number() );
        }
        ir_temp *= h_to->getNumS();
        infection_rates[ h_to ] = ir_temp;
    }

    // (2) Calculate the times of the next infection for each herd according to these rates
    // (3) Take the earliest of these times and the pertaining herd
    Herd*  infection_herd=nullptr;
    double infection_time = std::numeric_limits<double>::max();
    for ( auto const h_to : my_herds )
    {
        // Calculate the (absolute) time at which an infection in a herd would happen at this herd's rate.
        ir_temp = time + system->rng.time_of_next_infection( infection_rates[h_to] );
        if ( ir_temp < infection_time )
        {
            infection_time = ir_temp;
            infection_herd = h_to;
        }
    }

    // (4) Check if there is an infection rate change before in any of the herds.
    double next_irc = next_infection_rate_change( e );
    if ( next_irc < infection_time )
    {  // (5a) If yes, do nothing, STOP!
        return;
    }
    else if ( infection_herd == nullptr )
        // This can happen, when there is no irc event in the queue for this farm and there are no susceptible cows in this farm.
        // Then, the previous loop will not set infection_herd, so that it remains NULL.
        // Moreover, next_irc==infection_time and thus the above condition does not hold.
        return;
    else
    {
        if(next_infection_event != nullptr){  // if there is already a present infection event
            //std::cerr << "Trying to schedule a new infection even although there is already a present infection rate event" << std::endl;
            if(next_infection_event->execution_time > infection_time){  // and it happens later then the new one, abort it
                this->invalidate_next_infection_event();
            }
            else  // or just don't schedule a new event
                return;
        }
        // (5b) If not, draw an S cow randomly from this herd
        Cow* infection_victim = infection_herd->random_S_cow();

        // (6) Schedule INFECTION event.
        system->schedule_event( new Event( infection_time, Event_Type::INFECTION, infection_victim->id() ) );
    }
}

void Farm::execute_event( Event *e )
{

  // At the moment the only events at this hierarchical level are the TRADE and QUARANTINEEND events.
	if ( e-> type == Event_Type::TRADE)
		execute_TRADE_event( e );
	else if( e-> type == Event_Type::QUARANTINEEND)
		this->executeQuarantineEndEvent(e);
	else
    {
      std::cerr << "Farm::execute_event called for Event:";
      Utilities::pretty_print((Event*) e , std::cerr );
      std::cerr << "Aborting." <<std::endl;
      exit(1);
    }

}

void Farm::execute_TRADE_event( Event* e )
{
    // The farm calling this event is the destination farm of the trade.
    Cow* c = Cow::get_address( e->id );
    if ( c == nullptr ) {
        // Dead cows are not traded. Only dead parrots are.
        return;
    }

    Farm* source;
    if (c->herd != nullptr) {
        source = c->herd->farm;
    }
    else {    // presumably this is the case of a slaughterhouse or a well farm
        // TODO should the source be null in the case of external introductions?
        source = nullptr;
    }

    if (source == this  )    // This checks whether the defined source farm is the same as the farm calling this event
    {
        if(source->myType != SLAUGHTERHOUSE){// it's quite hacky to disable the trade at this position, but it will fix issue #6
            std::cerr << " Encountered trade with source==destination. This should not happen. Aborting. " << std::endl;
            std::cout << this->myType << std::endl;
            std::cout << source->id << std::endl;
            Utilities::printStackTrace(15);
            std::cout << e << std::endl;
            exit(1);
        }
        else{
            return;
        }
    }

    // The pull_cow and push_cow methods ensure that the number or S, TI, PI and R in both farms are correct,
    //  the list of susceptible cows are up to date and the cows get the pointer to the right herd.
    if(source != nullptr){
        source->pull_cow( c );
        source->invalidate_next_infection_event();
        source->infection_rate_has_changed( e );
    }
    //TODO Should the push_cow be in the above source conditional?
    push_cow( c );

    // If there is an infection event in either of the farms (which can happen if the trade has been scheduled after the infection has been scheduled)
    // this infection event has to be invalidated because a trade is an infection rate changing event (for both farms) and there should never be an infection event
    // after an infection rate changing event in the queue.

    this->invalidate_next_infection_event();

    // if the tradeQuery pointer has been assigned to an event and the animal got traded in the end, now is the pointer's end of life
    //delete c->tradeQuery;


    for ( auto c_e : c->future_irc_events_that_move )
        future_infection_rate_changing_events.push( c_e );

    this->infection_rate_has_changed( e );
}

//void Farm::get_state( std::array< int , 4 > * input )
//{
//  if (input !=NULL )
//    {
//      *input = { number_of_S()  ,
//		 number_of_TI() ,
//		 number_of_PI() ,
//		 number_of_R()  };
//    }
//}
void Farm::print_state( std::ostream& out , int level_of_detail)
{
  for (auto h : my_herds )
    h->print_info(out);
}

void Farm::holdSize(){
	this->manager->readjustToFarmSize();
}

// This function extracts and returns the farm that pertains to an event.
Farm* Farm::farm_of_event(const Event* e)
{
	Farm* f = this;
	if ( e->dest == Destination_Type::COW )
	{
		Cow* c = Cow::get_address( e->id );
		if ( c != nullptr && c->herd != nullptr)
		f = c->herd->farm;
		else
		f= nullptr; // Happens if the cow is null, which means that the event pertains to a dead cow and thus won't be executed.
	}
	return f;
}

//Find first event with executiontime > current time and farm==this
bool Farm::its_the_one(const Event *e,const Event* current )
{
	return ((e->execution_time >= current->execution_time) && (this->farm_of_event(e)==this) && (e != current ));
}

void Farm::executeQuarantineEndEvent(const Event* e){
	if(e == this->lastQuarantineEndEvent){
		this->quarantine = false;
		this->lastQuarantineEndEvent = nullptr;
	}
}

void Farm::jungtierCheck(){  // This thing is so quick and dirty that even your mum feels clean
	if(this->myType == SLAUGHTERHOUSE || this->myType == WELL) return;
	// TODO Generalise the test mechanisms of any herds
	int num = this->getNumberOfCowsToTest();
	bool onePositiveTest = false;
    {
        //int i = 0;
        //TODO The my_herds[0]->total_number() exceeds the my_herds[0]->all_my_cows.size() for some small number. Find out why and fix!
        for (auto cow : this->my_herds[0]->getNUnknownCows(num)) {  // selection from an unordered set of animals-->for all intents and purposes random
            TEST_EVENT *e = new TEST_EVENT( this->system->getCurrentTime(), Event_Type::JUNGTIER_SMALL_GROUP, cow->id(), this );

            //++i;

            //if(my_herds[0]->total_number() >= 500)
                //std::cout << "Farm " << id << " selected animals: " << num << std::endl;

            if ( cow->isTestedPositive(e) ) {
                onePositiveTest = true;
                cow->knownStatus = KnownStatus::POSITIVE;  // set the AB tested cow to positive from that test
                this->system->output->logEvent(e);
                delete e; // in case the antibody test is positive, since the event has been logged we don't need it anymore and it does not go to the priority queue to be deleted
                break;
            }
            cow->knownStatus = KnownStatus::NEGATIVE; // if it is not tested positive, the animal should be declared negative
            // As the event is to end its lifetime now that we have a confirmed status for the animal we need to log it
            this->system->output->logEvent(e);
            delete e;
        }
/*        if (my_herds[0]->total_number() > my_herds[0]->all_my_cows.size())
            std::cout << my_herds[0]->total_number() << " " << my_herds[0]->all_my_cows.size() << std::endl;*/
        //std::cout << "Farm " << id << " with pop: " << my_herds[0]->total_number() << " and " << num << " selected animals, tested animals: " << i << std::endl;
    }
	if(onePositiveTest) this->testAllCows();  // an animal which is immune by the JTF protocol implicates historically the existence of a BVD-infected animal in the herd
}

void Farm::testAllCows(){
	for(auto herd: this->my_herds)
		herd->testAllCows();
}

int Farm::getNumberOfCowsToTest() const{  // For the young calf window with 95% confidence (upper limit for the # of the
                                          // infected animals of the herd size N)
	int N = this->total_number();
	int n =0;
	if(N > 180) n= 14;
	else if(N <= 180 &&  N > 60) n = 13;
	else if(N <= 60 && N > 30) n = 12;
	else if(N <= 30 && N > 20) n = 11;
	else if(N <= 20 && N > 10) n = 10;
	else n = 8;

	return n;
}