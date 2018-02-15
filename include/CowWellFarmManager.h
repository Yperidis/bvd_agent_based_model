
#ifndef __CowWellFarmManager_h_
#define __CowWellFarmManager_h_
#include "FarmManager.h"
class System;
class Farm;
class CowWellFarmManager: public FarmManager{
	public:
		CowWellFarmManager(Farm* f, System *s);
		~CowWellFarmManager();
		void calculateDemand(std::set<Demand*>* requests);
		int calculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria crit,int overallNumber,int groupNum, Cow::UnorderedSet* cows);
		void chooseCowsToOfferFromGroupAndAddToSellingGroup(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows);
		int getACowId();
		void registerCowForSale(const Cow* cow);
	    void changeInfectionReplacement(double ti, double pi);
	    std::string printInfectionValues();
	private:
		int numberOfCowsToTrade;
		double tis;
		double pis;
};

#endif
