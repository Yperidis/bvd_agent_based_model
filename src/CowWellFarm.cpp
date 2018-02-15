
#include "CowWellFarm.h"
#include "CowWellFarmManager.h"
#include "BVD_Random_Number_Generator.h"
CowWellFarm::CowWellFarm(System *s):Farm(s){
	my_herds.push_back(new Herd( this ) );
	this->manager = new CowWellFarmManager(this, s);
	this->myType = WELL;
	this->system->getMarket()->registerSourceFarm(this);
}
CowWellFarm::~CowWellFarm(){
	for (auto h : my_herds )
		delete h;
}
void CowWellFarm::push_cow(Cow* c){
	my_herds.front()->push_cow( c );
}

void CowWellFarm::pull_cow(Cow* c){

	Calf_Status calf_status;
	bool birth;
	double timeInFuture = system->rng.first_insemination_age() + system->rng.conception_result( system->current_time() - c->birth_time , c->infection_status , &birth ) ;
	switch(c->infection_status )
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
	c->calf_status = calf_status;
	system->schedule_event( new Event( system->current_time() + timeInFuture , Event_Type::BIRTH    , c->id() ) );
	my_herds.front()->pull_cow( c );
}

int CowWellFarm::getACowId(){
	
	return ((CowWellFarmManager *) this->manager)->getACowId();
}