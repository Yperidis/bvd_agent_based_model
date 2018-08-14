#ifndef __farmManager_h_
#define __farmManager_h_
#include <set>
#include "Cow.h"
#include "System.h"
#include "Market.h"
#include "Farm.h"
#include "Herd.h"
#include <list>
class Demand;

enum FarmManagerSellChoosingStrategy{
 EQUALLY_DISTRIBUTED,
 OLD_COWS_FIRST,
 NONE
};



class FarmManager{
	public:
		FarmManager(Farm* farm,System *s);
        int *plannedNumberOfCows;

		~FarmManager();
		void manage();
		void readjustToFarmSize();
		typedef std::list<Cow_Trade_Criteria> CriteriaList;
		typedef std::list<FarmManager::CriteriaList*> GroupPriorityList;
		virtual void registerCowForSale(const Cow* cow);
	private:
		virtual void calculateDemand(std::set<Demand*>* requests)=0;  // requests which are created in this function will be deleted in the Market
		void postDemand(std::set<Demand*>* requests);
		virtual int calculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria crit, int overallNumber,int groupNum = 1, Cow::UnorderedSet* cows= nullptr)=0; //calculate the number of animals taht should be sold
		virtual void chooseCowsToOfferFromGroupAndAddToSellingGroup(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows)=0;
		void postOffer(Cow::UnorderedSet* cowsToSell);
		void resetGroupsOfAllCowsOfAllHerds();
		void chooseCowsToOffer(Cow::UnorderedSet* cowsToSell);


	protected:
		Farm* myFarm;
		System* system;
		FarmManagerSellChoosingStrategy sellingStrategy;

		int buyingMargin;
		int sellingMargin;
		GroupPriorityList* sellingPriorityList;
		double replacementPercentage;
		void standardCalculateDemand(std::set<Demand*>* requests);
		int standardCalculateOverallNumberToSell();
		int standardCalculateOverallNumberToBuy(bool replace=true);
		int standardCalculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria crit,int overallNumber, int groupNum = 1, Cow::UnorderedSet* cows = nullptr);
		void standardOfferingMethod(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows);
		bool isUnderQuarantine();
		static FarmManagerSellChoosingStrategy iniInputToSellingStrategy(std::string& input);
		
		Cow::UnorderedSet *registeredCowsToSell;
};
#endif
