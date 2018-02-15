#include "CowWellFarmManager.h"
#include "Farm.h"
#include "System.h"
#include "projectImports/inih/cpp/INIReader.h"
#include "Cow.h"
CowWellFarmManager::CowWellFarmManager(Farm *f, System* s):FarmManager(f,s){
	this->numberOfCowsToTrade = System::reader->GetInteger("farmmanager", "numberOfCowsInWell", 20);

	if(System::reader->GetBoolean("modelparam", "PercentagesShallBeAppliedOnWells", true)){
		this->tis = System::reader->GetReal("modelparam", "populationPercentageTI", 0.02);
		this->pis = System::reader->GetReal("modelparam", "populationPercentagePI", 0.02);
	}else{
		this->tis = 0.0;
		this->pis = 0.0;
	}
}


void CowWellFarmManager::calculateDemand(std::set<Demand*>* requests){}//the farm doesn't have any demand
int CowWellFarmManager::calculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria crit,int overallNumber,int groupNum, Cow::UnorderedSet* cows){
	if(crit == PREGNANT)
		return this->numberOfCowsToTrade;
	
	return 0;
		
}
void CowWellFarmManager::chooseCowsToOfferFromGroupAndAddToSellingGroup(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows){
	if(crit != PREGNANT)
		return;
	Cow *c;
	do{

		c = new Cow(system->current_time(), NULL);
		c->calf_status = Calf_Status::SUSCEPTIBLE;
		double percent = system->rng.ran_unif_double(1.0,0.0);
		if(percent < this->tis){
			c->infection_status = Infection_Status::TRANSIENTLY_INFECTED;
		}else if(percent < this->tis + this->pis){
			c->infection_status = Infection_Status::PERSISTENTLY_INFECTED;
		}
		this->myFarm->push_cow(c);
		
	}while(this->myFarm->total_number() <= numberOfCowsToSell);
	for(auto herd: *this->myFarm->getHerds()){
		herd->getNRandomCowsFromGroup(numberOfCowsToSell, PREGNANT, cows);
	}
//	std::cout << "end" << std::endl;	
}
int CowWellFarmManager::getACowId(){
	Cow* c = new Cow(system->current_time(), NULL);
		c->calf_status = Calf_Status::SUSCEPTIBLE;
		double percent = system->rng.ran_unif_double(1.0,0.0);
		if(percent < this->tis){
			c->infection_status = Infection_Status::TRANSIENTLY_INFECTED;
		}else if(percent < this->tis + this->pis){
			c->infection_status = Infection_Status::PERSISTENTLY_INFECTED;
		}
		this->myFarm->push_cow(c);
	return c->id();
}
void CowWellFarmManager::registerCowForSale(const Cow* cow){}


void CowWellFarmManager::changeInfectionReplacement(double ti, double pi) {
		this->tis = ti;
		this->pis = pi;
}

std::string CowWellFarmManager::printInfectionValues() {
    return "PI: " + std::to_string(this->pis) + ",TI: " + std::to_string(this->tis);
}
