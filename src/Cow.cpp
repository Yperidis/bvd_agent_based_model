#include <ostream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include "Cow.h"
//#include "System.h"
#include "System.h"
#include "BVD_Random_Number_Generator.h"
#include "Herd.h"
#include "Farm.h"
#include "FarmManager.h"
#include "Events.h"
#include "Utilities.h"
#include "BVDContainmentStrategy.h"
bool Cow_Pointer_Sort_Criterion::operator() (Cow const * const c1 , Cow const * const c2)
{
	return (c1->id() < c2->id() );
}


//static methods and members:
int  Cow::cow_id_counter = 0;
System* Cow::system = NULL;
std::unordered_map< int , Cow* > Cow::all_living_cows = std::unordered_map< int , Cow* >();
int  Cow::total_number()          { return cow_id_counter; }
int  Cow::number_of_living_cows() { return all_living_cows.size(); }

void Cow::set_system( System* s ) { system = s; }
Cow* Cow::get_address( int search_id )
{
	try { return all_living_cows.at( search_id ); }
	catch ( const std::out_of_range& oor ) { return NULL; }
}

double Cow::age() const{
	return System::getInstance(NULL)->getCurrentTime() - this->birth_time;
}

//non-static methods from here.
int   Cow::id() const { return _id; }

Cow::Cow( double time , Cow* my_mother )
{
	init(time, my_mother,  system->rng.is_calf_female());
}

Cow::Cow( double time, Cow* my_mother, bool isFemale){
	init(time, my_mother,  isFemale);
}

Cow::Cow(std::string cowName){

	double introTime = System::reader->GetReal(cowName, "introductiontime", 500);
	double cowAge = System::reader->GetReal(cowName, "cowAge", 500);
	double t = introTime - cowAge;

	init(t, nullptr, true);

	std::string infection_statusString = System::reader->Get(cowName, "infection_status", "SUSCEPTIBLE");
	std::string calf_statusString = System::reader->Get(cowName, "calf_status", "SUSCEPTIBLE");
	calf_status = stringToCalfStatus(calf_statusString);
	infection_status = stringToInfectionStatus(infection_statusString);

	if(calf_status != Calf_Status::NO_CALF){
		Cow::system->schedule_event( new Event( introTime , Event_Type::CONCEPTION , id() ) );
	}


}

void Cow::init( const double& time, Cow* my_mother, bool isFemale){
    ///Initialising the Cow constructor
	_id                  =  cow_id_counter++;
	birth_time           =  time;
	female               =  isFemale;
	infection_status     =  Infection_Status::SUSCEPTIBLE;
	last_conception_time = -1;
	calving_number       =  Cow::system->rng.number_of_calvings();
	has_been_pregnant_at_all_so_far = false;
	planned_birth_event = nullptr;

	calf_status          =  Calf_Status::NO_CALF;
	mother               =  my_mother;

	if (my_mother != NULL ){
		herd                 =  my_mother->herd;
	} else {
		herd = NULL;
	}
	all_living_cows[ _id ]= this;
	birthTimesOfCalves = new double[calving_number+1]; //using one more entry to have an END for the output class to look for
	for(int i=0; i < calving_number+1; i++){
		birthTimesOfCalves[i] = -1.0;
	}
	hasBeenTestedPositiveYet = false;
	knownStatus = KnownStatus::NOSTATUS;
	end_of_vaccination_event = nullptr;
	numberOfVaccinations = 0;
	timeOfInfection = -1.0;
	timeOfLastCalving = -1.0;
	lastTestTime = -1.0;
	firstTestTime = -1.0;
}
Cow::~Cow()
{
	for ( auto const& c : children )
		c->mother = NULL;
	if (mother != NULL)
		mother->children.erase(this);
	herd->pull_cow( this );
	delete[] birthTimesOfCalves;
	all_living_cows.erase( id() );
}


void Cow::execute_event( Event* e )
{
	Farm * f = herd->farm;  // We have to store the farm, because after the potential execution of a DEATH event, the cow will already have been deleted
                            // and thus its farm reference
	future_irc_events_that_move.erase(e); // Similar reason for doing this here.

	if(f->myType == SLAUGHTERHOUSE && !(e->type == Event_Type::SLAUGHTER) ){    //if the farm is a slaughterhouse and the
		                                                                        //event to be executed not a slaughter, then
		                                                                        //then there is nothing to execute
		return;
	}
	switch ( e->type ){
		case Event_Type::VACCINATE		:
			this->runVaccination(e->execution_time);
			break;
		case Event_Type::END_OF_VACCINATION		:
			if(this->end_of_vaccination_event != nullptr && this->end_of_vaccination_event == e){
				this->execute_END_OF_VACCINATION(e->execution_time);
			}
			break;
		case Event_Type::BIRTH        :
			execute_BIRTH( e->execution_time );
			break;
		case Event_Type::ABORTION     :
			execute_ABORTION( e->execution_time );
			break;
		case Event_Type::REMOVECOW:
			this->herd->farm->manager->registerCowForSale(this);
			break;
		case Event_Type::INSEMINATION :
//			if(e->execution_time - this->birth_time < 200.)
//  	std::cout << "insem\t" <<  this->birth_time << "\t" << e->execution_time << "\t" << this->id() << "\t" << e->id<<  std::endl;
			execute_INSEMINATION( e->execution_time );
			break;
		case Event_Type::CONCEPTION   :
			execute_CONCEPTION( e->execution_time );
			break;
		case Event_Type::SLAUGHTER	  :
		case Event_Type::CULLING	  :
		case Event_Type::DEATH        :
			execute_DEATH( e->execution_time );
			break;
		case Event_Type::END_OF_MA    :
			execute_END_OF_MA( e->execution_time );
			break;
		case Event_Type::INFECTION    :
			execute_INFECTION( e->execution_time );
			break;
		case Event_Type::RECOVERY     :
			execute_RECOVERY( e->execution_time );
			break;
		case Event_Type::VIRUSTEST:
		case Event_Type::ANTIBODYTEST:
		case Event_Type::TEST:
			this->testCow(e);
			break;

		default:
			std::cerr << "Unknown event type for Cow: " << Utilities::Event_tostr.at (e->type) << ". Exiting." <<std::endl ;
			exit(1);
	}
	if ( e->is_infection_rate_changing_event() ){
		//std::cout << "infection rate change event of type " << (int) (e->type) << std::endl;
		f->infection_rate_has_changed( e );
	}
}

Cow_Trade_Criteria Cow::getCowTradeCriteria(){
	double date = this->system->getCurrentTime();
	double age = date - this->birth_time;
	static double calfMaxAge = bvd_const::age_threshold_calf;
	if(!this->female){
		if(age <= calfMaxAge){
			return MALE_CALF;
		}else if(age <= 6.*31.)
			return YOUNG_BULL;
		else
			return OLD_BULL;

	}
	if(this->calf_status == Calf_Status::INFERTILE){
		return INFERTILE;
	}


	if(calf_status !=  Calf_Status::NO_CALF && calf_status != Calf_Status::ABORT){
		return PREGNANT;
	}
	if(this->children.size() > 0){
		//TODO return OLD_COW
		if(age > 1488.) // 1488 =48*31
			return OLD_COW;
		return DAIRY_COW;
	}else{
		if(age <= calfMaxAge){
			return CALF;
		}else if(age <= 527.){//527*17*31//TODO implement random distribution when breeding begins
			return HEIFER_PRE_BREEDING;
		}else
			return HEIFER_RDY_BREEDING;

	}
}
void Cow::handle_rest_time_after_ABORTION_or_BIRTH( double time )
{
	double execution_time = time + system->rng.time_of_rest_after_calving(calving_number);
	// TODO calving number is used subtractively.
	if ( calving_number <= 0 ){
		///The cow's cycle ends and it is sent to the slaughterhouse
		/// Die Kuh hat ausgedient und wird auf Suppenkuh umgeschult.
		this->herd->farm->manager->registerCowForSale(this);
	}
	else
	{
		/// Die Kuh hat noch nicht ausgedient und kann ihre Position als Gebär- und Milchmaschine behalten.
		///The cow has not yet fulfilled its usage cycle and thus retains its position as a birth and milk machine
		/// Der nächste Gebärauftrag wird aufgegeben

		//vaccTime is reinitialised here if the vaccination effect has expired, i.e. is a nullptr
		double vaccTime;
		if (this->end_of_vaccination_event == nullptr) {
			vaccTime = -1.0;
		} else{
		    //if the vaccination effect has not expired, then allow the vaccination time to be set as already scheduled
			vaccTime = this->end_of_vaccination_event->execution_time;
		}
		this->scheduleInsemination(execution_time,vaccTime);
	}
}

void Cow::execute_BIRTH( const double& time  )
{
    //TODO check if this accounts only for abortions and stillbirths or if there is something else going on
    ///At these block segments we are referring to the embryo
	if ( calf_status == Calf_Status::NO_CALF || (time - last_conception_time) < bvd_const::minimum_pregnancy_duration ) {
		/// This probably happens if there has been an abortion.
		if (calf_status == Calf_Status::NO_CALF) {
			std::cout << "WARNING! CALF STATUS NOCALF CALLED AT BIRTH! POSSIBLE ABORTION." << std::endl;
		}
		//If the function is called for a non pregnant cow or the carriage time is smaller than the minimum
        //pregnancy duration a calf is certainly not born, so we should exit the function.
		return;
	}

	calving_number--;    //counter for the number of calfs the cow has given birth to (calving number)
	bool first_birth = !has_been_pregnant_at_all_so_far;
	has_been_pregnant_at_all_so_far = true;
	double calfVaccinationTime = -1.0;    //initialisation of the vaccination time
	handle_rest_time_after_ABORTION_or_BIRTH( time );

	/// Will the newborn calf be living? (This depends on the age of the mother).
	if ( !system->rng.is_this_a_deadbirth( first_birth ) )
	{ // Determine status of new calf. We assume it is not born in the TI state for simplicity
		Infection_Status is;
		switch ( calf_status )
		{
			//We assume that a calf cannot be transiently infected upon birth
			case Calf_Status::SUSCEPTIBLE:
				is = Infection_Status::SUSCEPTIBLE;
				break;
			case Calf_Status::IMMUNE:
				is = Infection_Status::IMMUNE;
				break;
			case Calf_Status::PERSISTENTLY_INFECTED:
				is = Infection_Status::PERSISTENTLY_INFECTED;
				break;
			case Calf_Status::CRIPPLE:
				calf_status = Calf_Status::NO_CALF;
				return; /// Culled right away in this model.
			default: /// We will never land here, because NO_CALF has been excluded already.
				calf_status = Calf_Status::NO_CALF;
				return;
		}

		calf_status = Calf_Status::NO_CALF;    //The calf cannot be pregnant itself at this point

		/// Welcome to this world little calf!
		Cow* calf = new Cow( time , this ); // The sex of the calf is determined in the constructor

		double time_of_death;
		// If this remains practically infinite (1.79769e+308), the cow will die anyway at some point,
		//  because either it is male or if it is female it has a finite number of calvings
        if ( is == Infection_Status::PERSISTENTLY_INFECTED ){ //PI animals will die earlier than others.
            time_of_death = time + system->rng.lifetime_PI();
        } else {
            time_of_death =  time + system->rng.time_of_death_as_calf(); // If this returns -1, the animal will not die as a calf.
            if ( time_of_death <= time ) {    //If the calf survives, then it will never die as a cow (as S, T or R)
                time_of_death = std::numeric_limits<double>::max();
            }
        }
        //FIXME DEATH not implemented, but deleted at execute_next_event() in System.cpp.
		system->schedule_event( new Event( time_of_death , Event_Type::DEATH , calf->id() ) );
		double execution_time;
		if ( calf->female ){/// Female calf: schedule first insemination if it doesn't die before.
			execution_time = time + system->rng.first_insemination_age();
			if ( time_of_death > execution_time ) {    //you wouldn't inseminate an animal which is about to die or has already died now, would you?
				this->scheduleInsemination(execution_time , calfVaccinationTime, calf);
			}
		}
		else {/// Male cow: schedule culling if it doesn't die before.
			execution_time = time + system->rng.life_expectancy_male_cow();
			if ( time_of_death > execution_time ) {    //if the male calf survives beyond its life expectancy kill it
                //TODO DEATH not implemented and memory leak?
				system->schedule_event( new Event( execution_time , Event_Type::DEATH , calf->id() ) );
			}
		}
        // If the mother is immune, then the calf is protected by maternal antibodies for a while
		if ( infection_status == Infection_Status::IMMUNE && is == Infection_Status::SUSCEPTIBLE )
        //If the calf status is susceptible, then it would have a period of MA protection due to the colostrum administered
        //if ( is == Infection_Status::SUSCEPTIBLE )
		{
			is = Infection_Status::IMMUNE;
			double ma_end = time+system->rng.duration_of_MA();
			if ( time_of_death > ma_end ){
				system->schedule_event( new Event( ma_end , Event_Type::END_OF_MA , calf->id() ) );
			}
		}
		calf->infection_status = is;    //According to what has been already set that can be S, P or R

		children.insert( calf );
        //TODO what does the push_cow implementation mean in the well and slaughter farms?
		herd->farm->push_cow( calf );    //Add the calf in the herd of the farm of its mother
		herd->reevaluateGroup(this); //Resetting the trading criteria according to the new arrivals
		//log time of birth to mother cow in order to use it in output
		int index = 0;
		while(birthTimesOfCalves[index] != -1.0) index++;    //Go through all the newly born calves and set their birth
                                                             // time to the present one
		birthTimesOfCalves[index] = time;
		if(system->activeStrategy->usesEartag){
			double firstTestAge = system->rng.timeOfFirstTest();
			system->schedule_event( new Event( system->getCurrentTime()+firstTestAge , Event_Type::TEST      , calf->id() )) ;

		}
		this->timeOfLastCalving = time;
		System::getInstance(NULL)->addCow(calf);

//      if(time - this->birth_time < 200)
//      	std::cout << time << "\t" << this->birth_time << std::endl;
	}// END if not deadbirth.

}


void Cow::execute_ABORTION( const double& time )
{
	if (!female) {
        // According to the cow creation (scheduled events only for females) from the initializer this shouldn't happen
        // for the first batch of pregnancies
		std::cout << "WARNING! ABORTION ON MALE HEAD! DEBUG ME" << std::endl;
		return;
	}
	calf_status = Calf_Status::NO_CALF;
	if (planned_birth_event != nullptr) {
		System::getInstance(NULL)->invalidate_event(planned_birth_event);
		planned_birth_event = nullptr;
	}
	if ( time - last_conception_time > bvd_const::threshold_abortion_counts_as_calving )
	{
		calving_number--;
		has_been_pregnant_at_all_so_far = true;
	}
	handle_rest_time_after_ABORTION_or_BIRTH( time );
}

void Cow::execute_INSEMINATION( const double& time )
{
	if (!female){
		return; // ;)
	}
	bool conception;    //if not initialised, then this is False by default
	double execution_time = time + system->rng.insemination_result( !has_been_pregnant_at_all_so_far , &conception );

	if (conception) {
		// The cow will become pregnant.
		system->schedule_event( new Event( execution_time , Event_Type::CONCEPTION , id() ) );
	} else{ // The cow won't become pregnant -> Cull it
		this->calf_status = Calf_Status::INFERTILE;
		this->herd->farm->manager->registerCowForSale(this);
	}
}

void Cow::execute_CONCEPTION(const double& time )
{
	last_conception_time = time;
	double execution_time ;

	switch( infection_status )
	{
		case Infection_Status::TRANSIENTLY_INFECTED:
			calf_status = system->rng.calf_outcome_from_infection ( 0 );
			break;
		case Infection_Status::PERSISTENTLY_INFECTED:
			calf_status = Calf_Status::PERSISTENTLY_INFECTED;          // p=1 for the birth of a PI calf by a PI mother.
			break;
		default:
			calf_status = Calf_Status::SUSCEPTIBLE; // Yes, SUSCEPTIBLE is right. An eventual immunity through MA is handled in the BIRTH routine.
			break;
	}
	if ( calf_status == Calf_Status::ABORT )
	{
		execution_time = time + system->rng.time_of_abortion_due_to_infection( 0 );
		system->schedule_event( new Event( execution_time , Event_Type::ABORTION , id() ) );
		return;
	}

	//At this point, the infection status of the mother can be anything.
	bool birth;
	execution_time = time + system->rng.conception_result( time - birth_time , infection_status , &birth );
	if (calf_status == Calf_Status::NO_CALF) {
		std::cout << "HELLO" << std::endl;
	}
	if ( birth ) {
		system->schedule_event( new Event( execution_time , Event_Type::BIRTH, id() ) );
	} else {
		system->schedule_event( new Event( execution_time , Event_Type::ABORTION, id() ) );
	}

}

void Cow::execute_DEATH( const double& time )
{
	//FIXME not implemented. However, an equivalent utility is in execute_next_event in System.cpp. Never called due to that. Redundant?
	std::cout << "WARNING! Tried to execute DEATH but not implemented" << std::endl;
}

//This function is intertwined with the function which ends the vaccination effect (execute_END_OF_VACCINATION)
void Cow::execute_END_OF_MA( const double& time )
{
    //if the cow has already been vaccinated, it shouldn't return to being susceptible here. This block should concern
    //only cows which are of age to be vaccinated.
	if(this->end_of_vaccination_event != nullptr && this->end_of_vaccination_event->execution_time > time)
		return;

	infection_status = Infection_Status::SUSCEPTIBLE;    //This block concerns any animal (also called from execute_END_OF_VACCINATION)
	herd->add_cow_to_susceptible( this );
	herd->remove_r_cow(this);

}

void Cow::execute_INFECTION( const double& time )
{
	//TODO the conditional here should never take place, but it does with the vaccination strategy. Check
	if ( infection_status !=Infection_Status::SUSCEPTIBLE )
	{
		std::cerr << "Non S cow getting infected at t=" <<time<< std::endl;
		std::cerr << "Infection status: " << Utilities::IS_tostr.at( infection_status ) << std::endl;
		return;  //exit(1)  //Exit the program upon return?
	}
	this->timeOfInfection = time;
	double execution_time;
	if ( time-birth_time < bvd_const::age_threshold_calf ) //It's a calf!
	{
		if (system->rng.will_TI_calf_die())	{
			execution_time = time + system->rng.time_of_death_infected_calf() ;
			system->schedule_event( new Event( execution_time , Event_Type::DEATH , id() ) );
		} else {
			//Calf will not die
			execution_time = time + system->rng.duration_of_infection();
			system->schedule_event( new Event( execution_time , Event_Type::RECOVERY , id() ) );
		}
	}
	else //It's not a calf.
	{// At this point, the calf status can only be NO_CALF or SUSCEPTIBLE (because the infection status before this event must have been SUSCEPTIBLE).
		if ( female && calf_status == Calf_Status::SUSCEPTIBLE ) // The cow is pregnant
		{
			double time_of_pregnancy = time-last_conception_time;
			calf_status = system->rng.calf_outcome_from_infection ( time_of_pregnancy );
			if (calf_status == Calf_Status::NO_CALF) {
				std::cout << "SOME ERROR HERE" << std::endl;
			}
			if (calf_status == Calf_Status::ABORT ){
				execution_time = time + system->rng.time_of_abortion_due_to_infection( time_of_pregnancy );
				system->schedule_event( new Event( execution_time , Event_Type::ABORTION , id() ) );
			}
		}
		// The cow is not pregnant

		//At this point it's still not a calf.
		//And it could be pregnant or not
		execution_time = time + system->rng.duration_of_infection();
		system->schedule_event( new Event( execution_time , Event_Type::RECOVERY , id() ) );
		/// NOTE: The reason to do schedule the RECOVERY separately for calves and non calves is not within the BVD model,
		///        There is just no other way to get the right program flow.

	}
	// At this point, it can be a calf or not and pregnant or not.
	infection_status = Infection_Status::TRANSIENTLY_INFECTED;
	herd->remove_cow_from_susceptible( this );
	herd->add_ti_cow(this);
}

void Cow::execute_RECOVERY( const double& time )
{
	infection_status = Infection_Status::IMMUNE;
	herd->remove_ti_cow(this);
	herd->add_r_cow(this);
}

void Cow::register_future_infection_rate_changing_event( Event* e )
{
	// (1) Check if the events is irc and if it is one that moves with the cow
	// (2) if yes, register it.
	if (!(e->is_infection_rate_changing_event()))
	{
		std::cerr << "Tried to register a non irc event with a cow. Aborting" <<std::endl;
		Utilities::pretty_print( e, std::cerr);
		exit(1);
	}
	if ( e->type == Event_Type::INFECTION ) //Infection Events don't move with the cow.
		return;
	if ( e->id != id() )
	{
		std::cerr << "Tried to register an irc event pertaining to a different cow.. Aborting" <<std::endl;
		exit(1);
	}
	future_irc_events_that_move.insert((Event*) e );
}

Calf_Status Cow::stringToCalfStatus(const std::string& input){
	if(input.compare("NO_CALF") == 0)
		return Calf_Status::NO_CALF;
	else if(input.compare("SUSCEPTIBLE") == 0)
		return Calf_Status::SUSCEPTIBLE;
	else if(input.compare("PERSISTENTLY_INFECTED") == 0)
		return Calf_Status::PERSISTENTLY_INFECTED;
	else if(input.compare("IMMUNE") == 0)
		return Calf_Status::IMMUNE;
	else if(input.compare("CRIPPLE") == 0)
		return Calf_Status::CRIPPLE;
	else if(input.compare("ABORT") == 0)
		return Calf_Status::ABORT;

	return Calf_Status::NO_CALF;

}
Infection_Status Cow::stringToInfectionStatus(const std::string& input){
	if(input.compare("SUSCEPTIBLE") == 0)
		return Infection_Status::SUSCEPTIBLE;
	else if(input.compare("TRANSIENTLY_INFECTED") == 0)
		return Infection_Status::TRANSIENTLY_INFECTED;
	else if(input.compare("PERSISTENTLY_INFECTED") == 0)
		return Infection_Status::PERSISTENTLY_INFECTED;
	else if(input.compare("IMMUNE") == 0)
		return Infection_Status::IMMUNE;

	return Infection_Status::SUSCEPTIBLE;
}
//The end of the vaccination effect
inline void Cow::execute_END_OF_VACCINATION(const double& time){
	if(this->infection_status == Infection_Status::IMMUNE){
		this->execute_END_OF_MA(time);
	}
	this->end_of_vaccination_event = nullptr;    //For any infectious status the vaccination effect is annulled

}
//TODO should this function be const, i.e. funct(...) const {...}? There is a dependency in the scheduleInsemination function (c->scheduleVaccination(vaccTime))
inline void Cow::scheduleVaccination(const double& time) const{
	double vaccTime = time;
	if(time - this->birth_time - bvd_const::firstVaccAge < 0 )    //make sure that the animal does not receive a vaccination before it reaches the appropriate age
		vaccTime = this->birth_time + bvd_const::firstVaccAge + 1;

//If the animal can be vaccinated (i.e. passed the previous conditional of test of age) we do not check here
//whether it should (i.e. if it has already been vaccinated) at the running time.

	//std::cout << vaccTime << " for cow with ID " << this->id() << "\n";
        system->schedule_event( new Event( vaccTime, Event_Type::VACCINATE, this->id() ) );
}

inline void Cow::runVaccination(const double& time){
    //TODO Ensure that the scheduling of the vaccination takes place only if end_of_vaccination event == nullptr
	//Actions to be taken if the animal is susceptible and the vaccination will have the desired effect
    if( this->infection_status == Infection_Status::SUSCEPTIBLE && system->rng.vaccinationWorks()){
        this->infection_status = Infection_Status::IMMUNE;
        this->herd->remove_cow_from_susceptible( this );

        this->herd->add_r_cow(this);
//		if(this->herd->farm->next_infection_event != nullptr && this->herd->farm->next_infection_event->id == this->id() && this->herd->farm->next_infection_event->execution_time < time + bvd_const::timeOfVaccinationPersistance)
//			this->herd->farm->invalidate_next_infection_event();
        //consistency check. If this has any other effect, it may lead to problems.

        if(this->end_of_vaccination_event != nullptr){
            system->invalidate_event(this->end_of_vaccination_event);    //This seems to disallow the end_of_vaccination_event to enter the main event queue, but it seems to be already there!
            this->end_of_vaccination_event = nullptr;}

        //Creating and scheduling the end of the vaccination's effect
        this->end_of_vaccination_event = new Event( system->getCurrentTime()+ System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense  , Event_Type::END_OF_VACCINATION      , this->id() );
        //this->end_of_vaccination_event = new Event( time + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense  , Event_Type::END_OF_VACCINATION      , this->id() );
        system->schedule_event(this->end_of_vaccination_event);    //schedule the expiry of the vaccine's protective effect
        //std::cout << time + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense << "\n";
        this->scheduleVaccination(time + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense);    //schedule the next vaccination
        this->numberOfVaccinations++;
    }
        //At this point the animal's infection status is not checked and it is scheduled for vaccination.
        //The only thing that is taken into account in the call of the scheduleVaccination function is
        //the current time and the duration of the vaccination's effect.
        //std::cout << time << "\n";
        //this->scheduleVaccination(time);
    this->scheduleVaccination(time + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense);    //schedule the next vaccination
    this->numberOfVaccinations++;

    //For debugging purposes
/*    if(buf_time == 0)    //for the first encounter set at current time
        buf_time = time;
    else if(time - buf_time < System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense) {    //if the difference in time is greater than 365. ok
		std::cout << "Called before the vacc. end" << "\n";
		cntr = ++cntr;
        std::cout << cntr << "  " << buf_time << "\n";
	}*/
}

bool Cow::testCow(const Event* e){
	if(firstTestTime <= 0.0)
		this->firstTestTime = system->getCurrentTime();
	lastTestTime = system->getCurrentTime();
	if(this->isTestedPositive(e)){
		if(System::getInstance(nullptr)->activeStrategy->quarantineAfterPositiveTest)
			this->herd->farm->putUnderQuarantine();

		this->knownStatus = this->hasBeenTestedPositiveYet ? KnownStatus::POSITIVE_TWICE : KnownStatus::POSITIVE_ONCE;
		bool testOnce = e->type == Event_Type::VIRUSTEST ? true : !(this->testAgain());
		bool testASecondTime = !(this->hasBeenTestedPositiveYet || testOnce) ; // !A && !B
		if(testASecondTime)
			this->scheduleNextTest();
		else if(testOnce)
			system->schedule_event( new Event( system->getCurrentTime()+0.5 , Event_Type::REMOVECOW, this->id() ) );//the statistics show that cows which have only been tested once and then removed, have been removed immediately
		else{
			double removeTime = system->rng.removeTimeAfterSecondTest();
			system->schedule_event( new Event( system->getCurrentTime()+removeTime , Event_Type::REMOVECOW, this->id() ) );
			if(this->hasBeenTestedPositiveYet)
				for(auto calf : this->children)
					calf->knownStatus = KnownStatus::POSITIVE_MOTHER;
		}

		this->hasBeenTestedPositiveYet = true;

	}else{
		this->knownStatus = KnownStatus::NEGATIVE;
		if(this->mother != NULL)
			this->mother->knownStatus = KnownStatus::NEGATIVE;
	}
	this->herd->removeCowFromUnknownList(this);
	return this->hasBeenTestedPositiveYet;
	//if the cow is not detected as infected: do nothing
	//else{}
}

bool Cow::isTestedPositive(const Event* e){
	bool resultIsCorrect = system->rng.bloodTestRightResult();

	bool correctHealthState = false;
	switch(e->type){
		case Event_Type::JUNGTIER_SMALL_GROUP:
		case Event_Type::ANTIBODYTEST:
			correctHealthState = this->infection_status == Infection_Status::IMMUNE;
			break;
		case Event_Type::TEST:
		case Event_Type::VIRUSTEST:
			correctHealthState = (this->infection_status == Infection_Status::PERSISTENTLY_INFECTED) || (this->infection_status == Infection_Status::TRANSIENTLY_INFECTED);
			break;
		default: std::cerr << "non test given" << std::endl;
			exit(9184712);

			break;
	}

	return !resultIsCorrect ^ correctHealthState; // (not A) XOR B
}
inline bool Cow::testAgain(){
	return system->rng.cowGetsASecondChance();
}
inline void Cow::scheduleNextTest(){
	double retestTime = system->rng.retestTime();
	system->schedule_event( new Event( system->getCurrentTime()+retestTime , Event_Type::TEST      , this->id() )) ;
}

void Cow::scheduleInsemination(const double& time, double& vaccTime, const Cow* c){
    //If there are null pointer remnants in memory the memory where c points to, dereference the passed calf
	if(c == nullptr)
		c = this;
	//TODO Memory leak of the new memory allocation?
	system->schedule_event( new Event( time , Event_Type::INSEMINATION, c->id() ) );
#ifdef _VACCINATION_DEBUG_
	std::cout << "vaccination enabled: "<< system->activeStrategy->usesVaccination << std::endl;
#endif

    //If on top of the insemination we have a vaccination strategy, we might need to schedule a future vaccination
	if(system->activeStrategy->usesVaccination){
		//First scheduling of vaccination for the newly born calf. Here vaccTime should have its initialisation value.
        //and we are assigning to it the time of vaccination before the scheduled insemination's time which is given.
		vaccTime = time - System::getInstance(NULL)->activeStrategy->vaccinationTimeBeforeInsemination;    //when the animal is to be vaccinated

		//Check if the cow has been vaccinated before
		//see if the vaccination will take place within the time before the insemination. Otherwise
        // invalidate it and schedule a new one.
		//So far it's sufficient to check if the approximate end of the vaccination effect is in the given time frame.
		//TODO if distributions for the time of a working vaccination are introduced, another test to check if a cow has been vaccinated before needs to be introduced
		if(c->end_of_vaccination_event != nullptr){
			// this needs to be in the future and at the latest at the vaccination time before the insemination
			const double diff = c->end_of_vaccination_event->execution_time - vaccTime;


            //TODO is the && diff condition meaningful? Shouldn't a new vaccination be scheduled only after the end of the previous?
			if(diff >= 0.0 && diff <= System::getInstance(nullptr)->activeStrategy->vaccinationTimeBeforeInsemination){
			    //&& diff <= System::getInstance(nullptr)->activeStrategy->vaccinationTimeBeforeInsemination
				//vaccination is happening in the desired time frame
				//leave everything as is
				return;
			}
		}else{
			//the cow has either not been vaccinated before or its previous vaccination's effect has worn off
            //(see the implementation of end_of_vaccination_event at the execute_END_OF_MA function). The previously defined
            //vaccTime takes into account that the vaccination and the following insemination have to be a specified interval
            //in time apart.
			c->scheduleVaccination(vaccTime);
		}

	}

}

void Cow::setGroup(Cow::UnorderedSet* set){
	if(set == this->Group)
		return;
	Cow::UnorderedSet::iterator myIterator;
	if(this->Group != nullptr && (myIterator = this->Group->find(this)) != this->Group->end()){
		this->Group->erase(myIterator);
	}
	this->Group = set;

	if(this->Group != nullptr  && (myIterator = this->Group->find(this)) == this->Group->end()) {
		this->Group->insert(this);
	}
}
Cow::UnorderedSet* Cow::getGroup(){
	return this->Group;
}
//TODO: Implement the bookkeeping for infection rate changes within a farm
//      There is a function for registering future infection rate changing events.
//      This function will be called by the scheduling routine of system.


//(*) Even if the infection rate actually did not change, for any event where it *could* have changed, the farm has to be notified.
//    The reason is that only infections up to this point have been scheduled by the farm, because the infection rate could have changed.
