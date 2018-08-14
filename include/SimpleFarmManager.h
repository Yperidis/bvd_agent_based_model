#ifndef __one_herd_farm_manager_h_
#define __one_herd_farm_manager_h_
#include "FarmManager.h"
#include "projectImports/inih/cpp/INIReader.h"
class SimpleFarmManager: public FarmManager{
	public:
		SimpleFarmManager(Farm* farm,System *s);
		~SimpleFarmManager();
		
	private:
		void calculateDemand(std::set<Demand*>* requests);  // requests which are created in this function will be deleted in
		int calculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria criteria,int overallNumber,int groupNum, Cow::UnorderedSet* cows);
		void chooseCowsToOfferFromGroupAndAddToSellingGroup(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows); 
		
};

#endif