#include "SimpleFarmManager.h"

SimpleFarmManager::SimpleFarmManager(Farm* farm,System *s):FarmManager(farm,s){
	
	#ifdef _DEBUG_
		std::cout << "creating simple farm manager" << std::endl;
	#endif
	

}
SimpleFarmManager::~SimpleFarmManager(){}
		

void SimpleFarmManager::calculateDemand(std::set<Demand*>* requests){
	this->standardCalculateDemand(requests);
}
// calculate number of groups to sell
int SimpleFarmManager::calculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria criteria, int overallNumber, int groupNum, Cow::UnorderedSet* cows){
	return this->standardCalculateNumberOfAnimalsPerGroup(criteria, overallNumber, groupNum, cows);
}
void SimpleFarmManager::chooseCowsToOfferFromGroupAndAddToSellingGroup(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows){
	
	this->standardOfferingMethod(numberOfCowsToSell, crit, cows);
}