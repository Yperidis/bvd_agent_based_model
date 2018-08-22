#ifndef __slaughter_house_manager_h_
#define __slaughter_house_manager_h_
#include "FarmManager.h"

class SlaughterHouseManager: public FarmManager{
	public:
		SlaughterHouseManager(Farm* farm,System *s);
		~SlaughterHouseManager();
		void registerCowForSale(const Cow* cow);
	private:
		int dumpingCapacityPerType;
		void calculateDemand(std::set<Demand*>* requests);  // requests which are created in this function will be deleted in the market
		int calculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria criteria,int overallNumber,int groupNum, Cow::UnorderedSet* cows);
		void chooseCowsToOfferFromGroupAndAddToSellingGroup(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows);
};

#endif
