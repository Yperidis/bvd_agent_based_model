#include "SmallFarmManager.h"

SmallFarmManager::SmallFarmManager(Farm* farm,System *s):FarmManager(farm,s){
	
	#ifdef _DEBUG_
		std::cout << "creating simple farm manager" << std::endl;
	#endif
	

}
SmallFarmManager::~SmallFarmManager(){}
		

void SmallFarmManager::calculateDemand(std::set<Demand*>* requests){
	int numToBuy = this->standardCalculateOverallNumberToBuy(false);
	Demand *d = new Demand(PREGNANT, numToBuy, this->myFarm);
	requests->insert(d);
}

int SmallFarmManager::calculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria criteria,int overallNumber,int groupNum, Cow::UnorderedSet* cows){  // calculate number of groups to sell
	return this->standardCalculateNumberOfAnimalsPerGroup(criteria,overallNumber, groupNum, cows);
}

void SmallFarmManager::chooseCowsToOfferFromGroupAndAddToSellingGroup(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows){
	
	this->standardOfferingMethod(numberOfCowsToSell, crit, cows);
}