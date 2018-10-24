#include <ostream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <Model_Constants.h>
#include "Cow.h"
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


// static methods and members:
int  Cow::cow_id_counter = 0;
System* Cow::system = nullptr;
std::unordered_map< int , Cow* > Cow::all_living_cows = std::unordered_map< int , Cow* >();
int  Cow::total_number()          { return cow_id_counter; }
int  Cow::number_of_living_cows() { return all_living_cows.size(); }

void Cow::set_system( System* s ) { system = s; }
Cow* Cow::get_address( int search_id )
{
	try { return all_living_cows.at( search_id ); }
	catch ( const std::out_of_range& oor ) { return nullptr; }
}

double Cow::age() const{
	return System::getInstance(nullptr)->getCurrentTime() - this->birth_time;
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

//Function for initialising cows introduced in the system independently of the simulation
// TODO Introduce the external cows to existing farms
Cow::Cow(std::string cowName){

	double introTime = System::reader->GetReal(cowName, "introductiontime", 500);
	double cowAge = System::reader->GetReal(cowName, "cowAge", 500);
	double t = introTime - cowAge;

	init(t, nullptr, true);

	std::string infection_statusString = System::reader->Get(cowName, "infection_status", "SUSCEPTIBLE");
	std::string calf_statusString = System::reader->Get(cowName, "Calf_Status", "SUSCEPTIBLE");
	calf_status = stringToCalfStatus(calf_statusString);
	infection_status = stringToInfectionStatus(infection_statusString);

	if(calf_status != Calf_Status::NO_CALF){
		Cow::system->schedule_event( new Event( introTime , Event_Type::CONCEPTION , id() ) );
	}


}

void Cow::init( const double& time, Cow* my_mother, bool isFemale){
    ///Function for initialising the Cow functions
	_id                  =  cow_id_counter++;
	birth_time           =  time;
	female               =  isFemale;
	infection_status     =  Infection_Status::SUSCEPTIBLE;
	last_conception_time = -1;
	calving_number       =  Cow::system->rng.number_of_calvings();
	has_been_pregnant_at_all_so_far = false;
	planned_birth_event = nullptr;
	planned_abortion_event = nullptr;
    tradeQuery = nullptr;
    scheduledTest = nullptr;

	calf_status          =  Calf_Status::NO_CALF;
	mother               =  my_mother;

	//The herd to belong to for the born calf should be its mother's herd. Otherwise point to nowhere.
	if (my_mother != nullptr ){
		herd                 =  my_mother->herd;
	} else {
		herd = nullptr;
	}
	all_living_cows[ _id ]= this;
	// TODO Why calving_number+1?
	birthTimesOfCalves = new double[calving_number+1];
	for(int i=0; i < calving_number+1; i++){
		birthTimesOfCalves[i] = -1.0;    //initialisation of the times for each calf the cow is scheduled to give birth to
	}
	hasBeenTestedPositiveYet = false;
	knownStatus = KnownStatus::NOSTATUS;
	end_of_vaccination_event = nullptr;
	numberOfVaccinations = 0;
	vaccExpiry = nullptr;
	timeOfInfection = -1.0;
	timeOfLastCalving = -1.0;
	lastTestTime = -1.0;
	firstTestTime = -1.0;
}
Cow::~Cow()
{
	for ( auto const& c : children )
		c->mother = nullptr;
	if (mother != nullptr)
		mother->children.erase(this);
	herd->pull_cow( this );
	//delete[] birthTimesOfCalves;
	all_living_cows.erase( id() );
}


void Cow::execute_event( Event* e )
{
	Farm * f = herd->farm;  // We have to store the farm, because after the execution of a possible DEATH event, the cow will already have been deleted
                            // and similarly its farm reference
	future_irc_events_that_move.erase(e); // Similar reason for doing this here for the cow's future irc events.

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
			this->herd->farm->manager->registerCowForSale(this);  // This should lead to the slaughterhouse manager
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
		case Event_Type::VIRUSTEST:    // Presently we do not distinguish the effects of the different tests
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
	double date = system->getCurrentTime();
	double age = date - birth_time;
	static double calfMaxAge = bvd_const::age_threshold_calf;
	if(!this->female){
		if(age <= calfMaxAge){
			return MALE_CALF;
		}else if(age <= 15.*30.)
			return YOUNG_BULL;
		else
			return OLD_BULL;

	}
	if(calf_status == Calf_Status::INFERTILE){
		return INFERTILE;
	}


	if(calf_status !=  Calf_Status::NO_CALF && calf_status != Calf_Status::ABORT){
		return PREGNANT;
	}
	if( !children.empty() ){
		// TODO return OLD_COW
		if(age > 1440.) // 1488 =48*30. Setting the criterion for the cow to be old at about 4 years.
			return OLD_COW;
		return DAIRY_COW;
	}else{
		if(age <= calfMaxAge){
			return CALF;
		}else if(age <= 510.){  // 510=17*30 // TODO implement random distribution when breeding begins
			return HEIFER_PRE_BREEDING;
		}else
			return HEIFER_RDY_BREEDING;

	}
}

void Cow::handle_rest_time_after_ABORTION_or_BIRTH( double time )
{
	double execution_time = time + system->rng.time_of_rest_after_calving(calving_number);
	if ( calving_number <= 0 ){
		/// The cow's cycle ends and it is sent to the slaughterhouse
		/// Die Kuh hat ausgedient und wird auf Suppenkuh umgeschult.
		herd->farm->manager->registerCowForSale(this);
	}
	else
	{
		/// Die Kuh hat noch nicht ausgedient und kann ihre Position als Gebär- und Milchmaschine behalten.
		/// The cow has not yet fulfilled its usage cycle and thus retains its position as a birth and milk machine
		/// Der nächste Gebärauftrag wird aufgegeben

		// vaccTime is reinitialised here if the vaccination effect has expired, i.e. is a nullptr
		double vaccTime;
		// TODO Should the vaccExpiry variable play a role here as well?
		if (end_of_vaccination_event == nullptr) {
			vaccTime = -1.0;
		}
		else{
		    // if the vaccination effect has not expired, then allow the vaccination time to be set as already scheduled
			vaccTime = end_of_vaccination_event->execution_time;
		}
		/// Schedule the next insemination after the above defined rest time has elapsed
/*        if(this->id() == 244193) {
            std::cout << "At t=" << time << " resting for: " << execution_time - time
                      << " days and scheduled insem. at t=" << execution_time << std::endl;
            if (planned_birth_event != nullptr)
                std::cout << "At t=" << time << " and after birth, planned birth at t="
                                                << planned_birth_event->execution_time << ". Debug!" << std::endl;
        }*/
        this->scheduleInsemination(execution_time, vaccTime);
	}
}

void Cow::execute_BIRTH( const double& time  )
{
    /// At this block segment we are referring to the cow's status
	if ( calf_status == Calf_Status::NO_CALF || (time - last_conception_time) < bvd_const::pregnancy_duration.min ){

	    // FIXME Sometimes the pretty_print precedes the standard output prompt at runtime. May have to do with the standard output response.
		if (calf_status == Calf_Status::NO_CALF) {
			std::cerr << "WARNING! CALF STATUS NO_CALF CALLED AT BIRTH! NON-PREGNANT COW IS CALLED FOR BIRTH @t="
                      << time << std::endl;
            Utilities::pretty_print(this, std::cout);
            //Utilities::pretty_print(planned_birth_event, std::cout);
		}
		else{
			std::cerr << "WARNING! BIRTH TAKING PLACE AT ABORTION CONDITIONS AT t=" << time << std::endl;
            Utilities::pretty_print(this, std::cout);
		}
		// If the function is called for a non-pregnant cow or the carriage time is less than the minimum
        // pregnancy duration a calf is certainly not born, so we should exit the function.
		return;
	}

	calving_number--;    // counter for the number of calves the cow has given birth to (calving number)
	bool first_birth = !has_been_pregnant_at_all_so_far;  // This is indeed the first birth (calving) of the cow
	has_been_pregnant_at_all_so_far = true;
	planned_birth_event = nullptr;  // Reset the flag for pregnancy of the cow, as its next birth event has not been planned yet
	double calfVaccinationTime = -1.0;    // initialisation of the vaccination time (to be used for scheduling the insemination)
	handle_rest_time_after_ABORTION_or_BIRTH( time );

	/// Will the newborn calf be living? (This depends on whether the mother has reached previously this stage).
	if ( !system->rng.is_this_a_deadbirth( first_birth ) )
	{ /// Determine the status of the born calf. We assume it is not born in the TI state for simplicity. The initialised
	/// cows provide the first batch of calf statuses. Compare with the settings of Initializer.cpp
		Infection_Status is;
		switch ( calf_status )
		{
			// We assume that a calf cannot be transiently infected upon birth
			case Calf_Status::SUSCEPTIBLE:
				is = Infection_Status::SUSCEPTIBLE;
				break;
			case Calf_Status::IMMUNE:  // According to the current design, if the calf status is immune, then the cow
				is = Infection_Status::IMMUNE;  // has been infected during pregnancy (see the execute_INFECTION function)
				break;							// and the calf has therefore acquired permanent immunity.
			case Calf_Status::PERSISTENTLY_INFECTED:
				is = Infection_Status::PERSISTENTLY_INFECTED;
				break;
			case Calf_Status::CRIPPLE:
				calf_status = Calf_Status::NO_CALF;
				return; /// Culled right away in this model (equivalent to returning computationally).
			default: /// We will never land here, because NO_CALF has been excluded already at the beginning of the function.
				calf_status = Calf_Status::NO_CALF;
				return;
		}

		calf_status = Calf_Status::NO_CALF;    //The mother should be set to be non pregnant at this point

		/// Welcome to this world little calf!
		Cow* calf = new Cow( time , this ); // The sex of the calf is determined in the constructor

		double time_of_death;
		// If this remains practically infinite (1.79769e+308), the cow will die anyway at some point,
		//  because either it is male or if it is female it has a finite number of calvings
        if ( is == Infection_Status::PERSISTENTLY_INFECTED ){  // PI animals will generally die earlier than others.
            time_of_death = time + system->rng.lifetime_PI();
			system->schedule_event( new Event( time_of_death , Event_Type::DEATH , calf->id() ) );
        }
        else{
            time_of_death =  time + system->rng.time_of_death_as_calf(); // If this returns -1, the animal will not die as a calf.
            if ( time_of_death <= time ){    // If the calf survives, then it will never die as a cow (as S, T or R)
                time_of_death = std::numeric_limits<double>::max();
            } else{
				system->schedule_event( new Event( time_of_death , Event_Type::DEATH , calf->id() ) );
            }
        }
		double execution_time;
		if ( calf->female ){ /// Female calf: schedule first insemination if it doesn't die before.
			execution_time = time + system->rng.first_insemination_age();
			if ( time_of_death > execution_time ){    // you wouldn't inseminate an animal which is about to die or has already died now, would you?
				this->scheduleInsemination( execution_time, calfVaccinationTime, calf );
			}
		}
		else {/// Male cow: schedule culling if it survives.
			execution_time = time + system->rng.life_expectancy_male_cow();
			if ( time_of_death > execution_time ) {    // if the male calf survives beyond its life expectancy kill it
				system->schedule_event( new Event( execution_time, Event_Type::DEATH, calf->id() ) );
			}
		}
        // If the mother is immune, then the calf is protected by maternal antibodies for a while
		///This should be valid only if the mother is R before conception. See execute_conception().
		if ( infection_status == Infection_Status::IMMUNE && is == Infection_Status::SUSCEPTIBLE )
		{
			is = Infection_Status::IMMUNE;
			double ma_end = time+system->rng.duration_of_MA();
			if ( time_of_death > ma_end ){    // If the calf survives its MA period, then change its status to S after
				// that period's expiry
				system->schedule_event( new Event( ma_end , Event_Type::END_OF_MA , calf->id() ) );
			}
		}
		calf->infection_status = is;    // According to what has been already set that can be S, P or R

		children.insert( calf );  // An unordered set for each animal, where its newborn calves are recorded
		herd->farm->push_cow( calf );    // Add the calf in the herd of the farm of its mother
		herd->reevaluateGroup(this); // Resetting the trading criteria according to the new arrivals in case these are in effect
		// log time of birth to mother cow in order to use it in output
		int index = 0;
		while(birthTimesOfCalves[index] != -1.0) index++;    // Find the scheduled calf to be born at the present time
		birthTimesOfCalves[index] = time;    // The previous loop ascertains that the born calf receives its birth time at
		                                     // its right scheduled order in respect to the mother
		if(system->activeStrategy->usesEartag){
			double firstTestAge = system->rng.timeOfFirstTest();
			scheduledTest = new Event( system->getCurrentTime() + firstTestAge, Event_Type::TEST, calf->id() ); // note the test in case we need to invalidate it in case of prior testing due to trading
			system->schedule_event( new Event( system->getCurrentTime() + firstTestAge, Event_Type::TEST, calf->id() ) );
		}
		this->timeOfLastCalving = time;
		System::getInstance(nullptr)->addCow(calf);
	}  // Case of stillbirth. Do nothing.
}

void Cow::execute_ABORTION( const double& time )
{
	if (!female) {
        // According to the cow creation (scheduled events only for females) from the initializer this shouldn't be reached
        // for the first batch of pregnancies
		std::cerr << "WARNING! ABORTION ON MALE HEAD! DEBUG ME" << std::endl;
		return;
	}
	calf_status = Calf_Status::NO_CALF;
/*    if(this->id() == 94344)
        std::cout << "Passed" << std::endl;*/
	if (planned_birth_event != nullptr) {
/*        if(this->id() == 244193)
            std::cout << "Invalidating at t=" << time << std::endl;*/
		System::getInstance(nullptr)->invalidate_event(planned_birth_event);
		planned_birth_event = nullptr;
	}

	if (planned_abortion_event != nullptr)
	    planned_abortion_event = nullptr;

	// Determine whether the abortion will be counted as a calving
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
		return;
	}
/*	if (id() == 309290)
		std::cerr << "Insemination of 309290 at t=" << time << std::endl;*/
	bool conception;    //if not initialised, then this is False by default
	double execution_time = time + system->rng.insemination_result( !has_been_pregnant_at_all_so_far , &conception );
/*	if (id() == 100)
		std::cout << system->rng.insemination_result( !has_been_pregnant_at_all_so_far , &conception ) << std::endl;*/

	if (conception){
		// The cow will become pregnant.
		system->schedule_event( new Event( execution_time, Event_Type::CONCEPTION, id() ) );
	}
	else{   //The cow is infertile -> Cull it
		this->calf_status = Calf_Status::INFERTILE;
		this->herd->farm->manager->registerCowForSale(this);
	}
}

void Cow::execute_CONCEPTION(const double& time )
{
	last_conception_time = time;
	double execution_time;

	switch( infection_status )
	{
		case Infection_Status::TRANSIENTLY_INFECTED:
			calf_status = system->rng.calf_outcome_from_infection ( 0 );  // Upon conception we're at the 1st pregnancy period
			break;
		case Infection_Status::PERSISTENTLY_INFECTED:
			calf_status = Calf_Status::PERSISTENTLY_INFECTED;          // p=1 for the birth of a PI calf by a PI mother.
			break;
		default:
			calf_status = Calf_Status::SUSCEPTIBLE; // Yes, SUSCEPTIBLE is right. A possible immunity through MA
                                                    // (for an IMMUNE mother) is handled in the BIRTH routine.
			break;
	}
	if ( calf_status == Calf_Status::ABORT )  // This might occur from the outcome of a TI mother upon infection. See above.
	{
		execution_time = time + system->rng.time_of_abortion_due_to_infection( 0 );  // Again, the offset for the abortion is upon conception
		system->schedule_event( new Event( execution_time, Event_Type::ABORTION, id() ) );
		return;
	}

	// At this point, the infection status of the mother can be anything.
	bool birth;
	execution_time = time + system->rng.conception_result( time - birth_time, infection_status, &birth );
	if (calf_status == Calf_Status::NO_CALF) {
		std::cerr << "NO_CALF appeared upon conception while this should be forbidden" << std::endl;
        Utilities::pretty_print(this, std::cout);
	}
	// The outcome of a conception can be either a birth or an abortion
	if ( birth ){
/*        if(this->id() == 244193) {
            std::cout << "At t=" << last_conception_time << " scheduled birth for t=" << execution_time << " through conception"
                      << std::endl;
            std::cout << "Calf status: " << (int) calf_status << std::endl;
        }*/
/*        if (planned_birth_event != nullptr)
            std::cout << "Event from birth: " << (int) planned_birth_event->type << std::endl;*/
		system->schedule_event( new Event( execution_time, Event_Type::BIRTH, id() ) );
	}
	else{
/*        if(this->id() == 244193)
            std::cout << "At t=" << time << " scheduled abortion for t=" << execution_time << " through conception" << std::endl;
        if (planned_abortion_event != nullptr)
            std::cout << "Event from abortion: " << (int) planned_abortion_event->type << std::endl;*/
		system->schedule_event( new Event( execution_time, Event_Type::ABORTION, id() ) );
	}

}

void Cow::execute_DEATH( const double& time )
{
	// FIXME not implemented. However, an equivalent utility is in execute_next_event in System.cpp. Never called due to that. Redundant?
	//std::cout << "WARNING! Tried to execute DEATH but not implemented" << std::endl;
}

//This function is intertwined with the function which ends the vaccination effect (execute_END_OF_VACCINATION)
void Cow::execute_END_OF_MA( const double& time )
{
    //if the cow has already been vaccinated, it shouldn't return to being susceptible here. This block should concern
    //only cows which are of age to be vaccinated.
	if(this->end_of_vaccination_event != nullptr && this->end_of_vaccination_event->execution_time > time) {
        std::cerr << "END Of MA CALLED WHILE A VACCINATION IS IN EFFECT" << std::endl;
        return;
    }

	infection_status = Infection_Status::SUSCEPTIBLE;    //This block concerns any animal (also called from execute_END_OF_VACCINATION)
	herd->add_cow_to_susceptible( this );
	herd->remove_r_cow(this);

}

void Cow::execute_INFECTION( const double& time )
{
    //TODO the conditional here should never take place, but it does with the vaccination strategy. Check
    if ( infection_status !=Infection_Status::SUSCEPTIBLE )
    {
        std::cerr << "Non S cow getting infected at t=" << time << std::endl;
        std::cerr << "Infection status: " << Utilities::IS_tostr.at( infection_status ) << std::endl;
        return;  //exit(1)  // Exit the program upon return?
    }
    this->timeOfInfection = time;
    double execution_time;
    if ( time-birth_time < bvd_const::age_threshold_calf ) // It's a calf!
    {
        if (system->rng.will_TI_calf_die())	{
            execution_time = time + system->rng.time_of_death_infected_calf();
            system->schedule_event( new Event( execution_time, Event_Type::DEATH, id() ) );  // A TI calf which dies from the infection
        }
        else{
            // Calf will not die
            execution_time = time + system->rng.duration_of_infection();
            system->schedule_event( new Event( execution_time, Event_Type::RECOVERY, id() ) );  // A TI calf scheduled for recovery
        }
    }
    else // It's not a calf.
        // At this point, the calf status can only be NO_CALF, INFERTILE or SUSCEPTIBLE (because the infection status before this event must have been SUSCEPTIBLE).
    {    // INFERTILE is taken care of directly, within the standard trading time though, so effectively only NO_CALF and SUSCEPTIBLE are possible here.
        if ( female && calf_status == Calf_Status::SUSCEPTIBLE ) // The animal is a cow and susceptible
        {
            if (planned_birth_event != nullptr) {  // Ascertain that the cow is indeed pregnant and not in its resting time,
                // otherwise the stage of the pregnancy (time_of_pregnancy) variable will have an unpredictable behaviour
                double time_of_pregnancy = time - last_conception_time;
                // Determine the outcome of the infection on the pregnancy/embryo. A cripple status is taken care of at the birth.
                calf_status = system->rng.calf_outcome_from_infection(time_of_pregnancy);  // Note that the instant
																// infection determines the calf outcome. Approximation.
                if (calf_status == Calf_Status::NO_CALF) {
                    std::cerr << "NO_CALF status upon infection of the pregnant cow. DEBUG." << std::endl;
                }
                if (calf_status == Calf_Status::ABORT) {
                    execution_time = time + system->rng.time_of_abortion_due_to_infection(time_of_pregnancy);
                    if (execution_time > planned_birth_event->execution_time) {  // If the abortion is to take place
                        // after the birth, then execute the abortion immediately
                        execution_time = time;
                        if (planned_abortion_event != nullptr){  // In case of a planned abortion upon conception...
                            if (planned_abortion_event->execution_time > execution_time) {  // if it is to be executed
                                // after the abortion due to infection invalidate the abortion from the conception to avoid
                                // double abortion events and their consequences.
                                System::getInstance(nullptr)->invalidate_event(planned_abortion_event);
                                planned_abortion_event = nullptr;
                            }
                        }
                        system->schedule_event( new Event(execution_time, Event_Type::ABORTION, id() ) );
                    }
                    else if (execution_time < planned_birth_event->execution_time){  // If the abortion is to take place
                        // before the birth, then schedule it at the defined execution time
                        if (planned_abortion_event != nullptr){  // In case of a planned abortion upon conception...
                            if (planned_abortion_event->execution_time > execution_time) {  // if it is to be executed
                                // after the abortion due to infection invalidate the abortion from the conception to avoid
                                // double abortion events and their consequences.
                                System::getInstance(nullptr)->invalidate_event(planned_abortion_event);
                                planned_abortion_event = nullptr;
                            }
                        }
                        system->schedule_event(new Event(execution_time, Event_Type::ABORTION, id()));
                    }  // In case that the planned abortion is to be executed simultaneously with the birth we assume
                       // that the birth will prevail. Therefore we leave everything as is.
/*                    if(this->id() == 244193)
                        std::cout << "At t=" << time << " abortion scheduled through infection for t=" << execution_time << std::endl;
                    system->schedule_event(new Event(execution_time, Event_Type::ABORTION, id()));*/
                }
            }
        }
        // The cow is not pregnant.
        // At this point it's not a calf.
        execution_time = time + system->rng.duration_of_infection();
        system->schedule_event( new Event( execution_time, Event_Type::RECOVERY, id() ) );
        /// NOTE: The reason to schedule the RECOVERY separately for calves and non calves is not part of the BVD model.
        /// There is just no other way to get the right program flow.

    }
    // At this point, the animal can be a calf or not and pregnant or not.
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
	// (1) Check if the event is an irc
    // (2) Check if it is one that moves with the cow
	// (3) If it is, then register it.
	if ( !( e->is_infection_rate_changing_event() ) )
	{
		std::cerr << "Tried to register a non irc event with a cow. Aborting" <<std::endl;
		Utilities::pretty_print( e, std::cerr);
		exit(1);
	}
	if ( e->type == Event_Type::INFECTION ) // Infection Events don't move with the cow because the cow changes neighbours.
		return;
	if ( e->id != id() )
	{
		std::cerr << "Tried to register an irc event pertaining to a different cow... Aborting" <<std::endl;
		exit(1);
	}
	future_irc_events_that_move.insert( e );
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

inline void Cow::execute_END_OF_VACCINATION(const double& time){
	if(this->infection_status == Infection_Status::IMMUNE){  // This should not concern non-S animals prior to vaccination
	                                                         // see the implementation at runVaccination()
		this->execute_END_OF_MA(time);
	}
	this->end_of_vaccination_event = nullptr;    // For any animal the vaccination effect is annulled

}

inline void Cow::scheduleVaccination(const double& time) const{
    if(vaccExpiry != nullptr){  // An ad hoc check that a vaccination will not take place while the previous is still in effect
        if(*vaccExpiry > time)
            return;
    }
	double vaccTime = time;
	if(time - this->birth_time - bvd_const::firstVaccAge < 0 )  // make sure that the animal does not receive a vaccination before it reaches the appropriate age
		vaccTime = this->birth_time + bvd_const::firstVaccAge + 1;
    system->schedule_event( new Event( vaccTime, Event_Type::VACCINATE, this->id() ) );
}

inline void Cow::runVaccination(const double& time){
/*    if(this->end_of_vaccination_event != nullptr){
		std::cerr << "VACCINATION EXECUTION WHILE PREVIOUS VACCINATION STILL IN EFFECT" << std::endl;
		return;
	}*/
	///Actions to be taken if the animal is susceptible and the vaccination will have the desired effect
    if( this->infection_status == Infection_Status::SUSCEPTIBLE && system->rng.vaccinationWorks() ){
        this->infection_status = Infection_Status::IMMUNE;
        this->herd->remove_cow_from_susceptible( this );
        this->herd->add_r_cow(this);

        // This should not happen as excluded from the beginning of the function. For debugging purposes.
        if(this->end_of_vaccination_event != nullptr){      //Setting the temporal offset for the vaccination event
            system->invalidate_event(this->end_of_vaccination_event);
            this->end_of_vaccination_event = nullptr;
            std::cerr << "A PREVIOUSLY S COW HAS A NON NULL END OF VACC. EVENT! NOT DEFINED SO." << std::endl;
        }

        // Creating and scheduling the end of the vaccination's effect
        this->end_of_vaccination_event = new Event( system->getCurrentTime() +
                                                    System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense,
                                                    Event_Type::END_OF_VACCINATION, this->id() );
        system->schedule_event(this->end_of_vaccination_event);  // schedule the expiry of the vaccine's protective effect (push into the system queue)
		vaccExpiry = &vaccBuf;  // Assigning an address to a pointer for the correct vaccination time-framing.
		*vaccExpiry = time + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense;
        this->scheduleVaccination(time + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense);  //schedule the next vaccination
        this->numberOfVaccinations++;
		return;
    }
    else{
        /// Vaccination scheme in all the rest of the cases. Simply vaccinate the animal with the vaccination
        /// having no effect on its health status. Note that we are not using the end_of_vaccination_event, because
        /// that would require R animals to go back to the S state upon the call of the end of MA function through the
        /// call of the end of vaccination function. We are merely scheduling the next vaccination after the predefined
        /// vaccination time of defence value. Therefore the end_of_vaccination_event cannot determine whether an animal
        /// has already been vaccinated or not in this case.
		vaccExpiry = &vaccBuf;  // As above
        *vaccExpiry = time + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense;
		//std::cout << *vaccExpiry << std::endl;
        this->scheduleVaccination(time + System::getInstance(
                nullptr)->activeStrategy->vaccinationTimeOfDefense);  // schedule the next vaccination
        this->numberOfVaccinations++;
    }
}

bool Cow::testCow(const Event* e){
	if(firstTestTime <= 0.0)    // initialisation value is -1
		this->firstTestTime = system->getCurrentTime();
	lastTestTime = system->getCurrentTime();
	if(this->isTestedPositive(e)){    // True or false positive

		// If an animal is tested positive prior to it being traded, invalidate the scheduled trade event
		if (tradeQuery != nullptr) {
			System::getInstance(nullptr)->invalidate_event(tradeQuery);
			tradeQuery = nullptr;
		}

		if(System::getInstance(nullptr)->activeStrategy->quarantineAfterPositiveTest)
			this->herd->farm->putUnderQuarantine();

		// If the cow has not been tested positive yet set its known status to "POSITIVE_ONCE".
		this->knownStatus = this->hasBeenTestedPositiveYet ? KnownStatus::POSITIVE_TWICE : KnownStatus::POSITIVE_ONCE;
		// If the test is a virus test, indeed test the cow once. Otherwise, if the cow is to be tested again, make this
		// variable false. If the cow is not to be tested again, indeed this variable should be true.
		bool testOnce = e->type == Event_Type::VIRUSTEST ? true : !(this->testAgain());  // For ear tag, only TEST events reach here
		// for non young-calf window strategy (individual basis). Only for YTF is the VIRUSTEST possible (for the whole herd).
		bool testASecondTime = !(this->hasBeenTestedPositiveYet || testOnce); // Do not test again (false) if the cow
		// has already been tested positive (so this is the second time we test it) or if it should be tested only once.
		if(testASecondTime)
			this->scheduleNextTest();
		else if(testOnce) {
			double removeTime = system->rng.removeTimeAfterFirstTest();
			system->schedule_event(new Event(system->getCurrentTime() + removeTime,
											 Event_Type::REMOVECOW, this->id()));  // statistics show that cows
			// which have only been tested once and then removed, have been removed immediately
		}
		else{  // fate of the animal after the second positive test
			double removeTime = system->rng.removeTimeAfterSecondTest();
			system->schedule_event( new Event( system->getCurrentTime() + removeTime, Event_Type::REMOVECOW, this->id() ) );
			if(this->hasBeenTestedPositiveYet)  // Sanity check. To have reached here is to have been tested positive already
				for(auto calf : this->children)
					calf->knownStatus = KnownStatus::POSITIVE_MOTHER;
			}

		this->hasBeenTestedPositiveYet = true;

	}else{
		this->knownStatus = KnownStatus::NEGATIVE;
		if(this->mother != nullptr)  // If the cow has given birth at some point in the past
			this->mother->knownStatus = KnownStatus::NEGATIVE;  // This accounts for calves being generated after the
                                                                // declaration of a mother as positive. If the offer
        // to the slaughterhouse has been registered before the time of this (the first test) it is possible that a
        // non PI calf might end up to the slaughterhouse due to its mother's positive status.
        // TODO Consider the effect of the above comment on the calves and the positive mother
	}
	//this->herd->removeCowFromUnknownList(this);  // this would remove animals being already tested from the pool selection for the JTF tests
	return this->hasBeenTestedPositiveYet;
}

bool Cow::isTestedPositive(const Event* e){
	bool resultIsCorrect = system->rng.bloodTestRightResult();    // This sets the outcome of the sensitivity success probability.
                                                                    // The specificity is set to 1 by default.
	bool correctHealthState;
	switch(e->type){
		case Event_Type::JUNGTIER_SMALL_GROUP:  
		case Event_Type::ANTIBODYTEST:			
			correctHealthState = this->infection_status == Infection_Status::IMMUNE;
			break;
		case Event_Type::TEST:
		case Event_Type::VIRUSTEST:
			correctHealthState = (this->infection_status == Infection_Status::PERSISTENTLY_INFECTED) ||
								 (this->infection_status == Infection_Status::TRANSIENTLY_INFECTED);
			break;
		default: std::cerr << "no test given" << std::endl;
			exit(9);
	}
	return resultIsCorrect && correctHealthState;  // AND for failure of the sensitivity success rate in all cases except
                                                // for its concurrent success and the animal being actually sick in the virus test (ear tag) or being immune in the antibody test.
	//return !resultIsCorrect ^ correctHealthState; // (not A) XOR B. Operation at the bit level due to the
	// class enum values. The returned value will only be true for a true positive or a false positive.
}

inline bool Cow::testAgain(){
	return system->rng.cowGetsASecondChance();    // The upper probability limit for that is set at the model constants.h
}

inline void Cow::scheduleNextTest(){
	double retestTime = system->rng.retestTime();    // uniform random number chosen between 20. and the defined
    // retestingTimBlood float (must be > 20.)
	system->schedule_event( new Event( system->getCurrentTime() + retestTime, Event_Type::VIRUSTEST, this->id() ) ); // If a second round of testing is underway
																													// let the second test be VIRUSTEST
}

void Cow::scheduleInsemination(const double& time, double& vaccTime, const Cow* c){
    // If there are null pointer remnants in the memory where c points to, dereference the passed c (calf)
	if(c == nullptr)
		c = this;
	system->schedule_event( new Event( time, Event_Type::INSEMINATION, c->id() ) );
/*	if(c->id() == 203458)
		std::cout << "Insem. scheduled at t= " << time << std::endl;*/
#ifdef _VACCINATION_DEBUG_
	std::cout << "vaccination enabled: "<< system->activeStrategy->usesVaccination << std::endl;
#endif

    // If on top of the insemination we have a vaccination strategy, we might need to schedule a future vaccination
    if(system->activeStrategy->usesVaccination){
        // First scheduling of vaccination for the newly born calf. vaccTime is the vaccination time and we are
        // assigning to it the time of vaccination before the scheduled insemination's time which is given. That
        // only if vaccTime has its initialised value.
        if(vaccTime == -1)  //initialisation value
            vaccTime = time - System::getInstance(nullptr)->activeStrategy->vaccinationTimeBeforeInsemination;    //when the animal is to be vaccinated
        // Note that vaccTime is assigned by reference. Therefore its value outside the function will also change.

        // Check if the cow has been vaccinated before
        // OPTIONAL:
        // if distributions for the time of a working vaccination are introduced, another test to check if a cow
        // has been vaccinated before needs to be introduced
        if(c->end_of_vaccination_event != nullptr){
            // We examine whether the expiry of the vaccination lies in the future and before the minimum time-span
            // between insemination and vaccination with diff
            const double diff = c->end_of_vaccination_event->execution_time - vaccTime;

            // We first check if the vaccination will still be in effect at the time of vaccTime
            if(diff >= 0.0){
                // If yes, then the vaccination is already scheduled properly and we shouldn't do anything.
                return;
            }
                // If not, if the insemination time's difference with the vaccTime is at or before the vaccination time before
                // insemination, then schedule the vaccination at vaccTime
            else if(time - vaccTime >= System::getInstance(nullptr)->activeStrategy->vaccinationTimeBeforeInsemination){
				vaccExpiry = &vaccBuf;  // As above (see runVaccination() )
            	*vaccExpiry = vaccTime + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense;
				c->scheduleVaccination(vaccTime);  //The vaccination is far enough from the insemination event. Do it!
			}
            else
                // If the vaccination is not in effect at the time of vaccTime and the insemination will happen in a time
                // frame smaller than the temporal distance between itself and the vaccination do not vaccinate.
                return;
        }
            // If the animal has not been previously vaccinated of its vaccine has expired, vaccinate if we're
            // looking at the right time frame between insemination and vaccination (see previous comments)
        else{
            if(time - vaccTime >= System::getInstance(nullptr)->activeStrategy->vaccinationTimeBeforeInsemination){
				vaccExpiry = &vaccBuf;  // As above (see runVaccination() )
				*vaccExpiry = vaccTime + System::getInstance(nullptr)->activeStrategy->vaccinationTimeOfDefense;
				c->scheduleVaccination(vaccTime);
			}
        }
    }
}

void Cow::setGroup(Cow::UnorderedSet* set){
	if(set == this->Group)
		return;
	Cow::UnorderedSet::iterator myIterator;
	if(this->Group != nullptr && (myIterator = this->Group->find(this) ) != this->Group->end()){
		this->Group->erase(myIterator);
	}
	this->Group = set;  // Setting the relevant group...

	if(this->Group != nullptr  && (myIterator = this->Group->find(this)) == this->Group->end()) {
		this->Group->insert(this);  // ...and inserting the cow at hand in it
	}
}

Cow::UnorderedSet* Cow::getGroup(){
	return this->Group;
}
// TODO: Implement the bookkeeping for infection rate changes within a farm
//      There is a function for registering future infection rate changing events.
//      This function will be called by the scheduling routine of system.


//(*) Even if the infection rate actually did not change, for any event where it *could* have changed, the farm has to be notified.
//    The reason is that only infections up to this point have been scheduled by the farm, because the infection rate could have changed.
