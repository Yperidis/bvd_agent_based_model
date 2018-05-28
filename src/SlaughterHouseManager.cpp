#include "SlaughterHouseManager.h"
#include "System.h" 
#include "projectImports/inih/cpp/INIReader.h"
SlaughterHouseManager::SlaughterHouseManager(Farm* farm,System *s):FarmManager(farm,s){
	this->dumpingCapacityPerType = System::reader->GetInteger("farmmanager","dumpingCapacityPerType", 20);
	
}
SlaughterHouseManager::~SlaughterHouseManager(){}

void SlaughterHouseManager::calculateDemand(std::set<Demand*>* requests){

	int demand = this->dumpingCapacityPerType;
	
	Cow_Trade_Criteria crit = INFERTILE;
	Demand* d = new Demand(crit,demand, this->myFarm);
	requests->insert(d);
	crit = HEIFER_PRE_BREEDING;;
	d = new Demand(crit,demand, this->myFarm);
	requests->insert(d);
	crit = OLD_COW;
	d = new Demand(crit,demand, this->myFarm);
	requests->insert(d);
	crit = HEIFER_RDY_BREEDING;
	d = new Demand(crit,demand, this->myFarm);
	requests->insert(d);
	crit = CALF;
	d = new Demand(crit,demand, this->myFarm);
	requests->insert(d);
	crit = PREGNANT;
	d = new Demand(crit,demand, this->myFarm);
	requests->insert(d);
}

//TODO The following three functions override the ones from FarmManager.cpp. The slaughterhouse should never sell cows.
int SlaughterHouseManager::calculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria criteria,int overallNumber,int groupNum, Cow::UnorderedSet* cows){
	return this->dumpingCapacityPerType;
}
void SlaughterHouseManager::chooseCowsToOfferFromGroupAndAddToSellingGroup(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows){}

void SlaughterHouseManager::registerCowForSale(const Cow* cow){}
