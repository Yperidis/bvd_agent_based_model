#include "FarmManager.h"
#include <iostream>
#include "projectImports/inih/cpp/INIReader.h"
#include "Utilities.h"
#include "BVDContainmentStrategy.h"
FarmManager::FarmManager(Farm* farm, System *s):myFarm(farm), system(s){
	this->buyingMargin = System::reader->GetInteger("farmmanager" , "threshold_buy", 5);
	this->sellingMargin = System::reader->GetInteger("farmmanager" , "threshold_sell", 20);
	double replacement = System::reader->GetReal("trade", "percentage_replacement", 0.279);
	if(replacement > 1.0 || replacement < 0.0){
		std::cerr << "Replacement rate must be within range [0.0, 1.0]. Aborting" << std::endl;
		exit(22);
	}
	double manageTime = s->_dt_manage;
	std::string percentage_time = System::reader->Get("trade" , "percentage_time", "DAILY");
	double dt_replace;
		if(percentage_time.compare("DAILY") == 0){
			dt_replace =bvd_const::tradingTimeIntervall.DAILY;
		}else if(percentage_time.compare("WEEKLY") == 0){
			dt_replace =bvd_const::tradingTimeIntervall.WEEKLY;
		}else if(percentage_time.compare("BIWEEKLY") == 0){
			dt_replace =bvd_const::tradingTimeIntervall.BIWEEKLY;
		}else if(percentage_time.compare("MONTHLY") == 0){
			dt_replace =bvd_const::tradingTimeIntervall.MONTHLY;
		}else if(percentage_time.compare("QUARTERLY") == 0){
			dt_replace =bvd_const::tradingTimeIntervall.QUARTERYEARLY;
		}else if(percentage_time.compare("HALFYEARLY") == 0){
			dt_replace =bvd_const::tradingTimeIntervall.HALFYEARLY;
		}else if(percentage_time.compare("YEARLY") == 0){
			dt_replace =bvd_const::tradingTimeIntervall.YEARLY;
		}else{
			std::cerr << "The trading time is not a valid value" << std::endl;
			dt_replace =bvd_const::tradingTimeIntervall.DAILY;
		}
	this->replacementPercentage = replacement * manageTime / dt_replace ;  // if the farms get managed every month and the percentage is given per year, 1/12 (numDaysPerMonth/numDaysPerYear) of the percentage should be used at each managing process
	this->plannedNumberOfCows = nullptr;
	this->readjustToFarmSize();
	std::string sellStrat = System::reader->Get("farmmanager" , "standardOfferingMethod", "evenlyDistributed");
	sellingStrategy = FarmManager::iniInputToSellingStrategy(sellStrat);
	sellingPriorityList = new FarmManager::GroupPriorityList();
	FarmManager::CriteriaList* list;
	switch(sellingStrategy){
		case OLD_COWS_FIRST:
			list = new FarmManager::CriteriaList();
			list->insert(list->end(),INFERTILE);
			list->insert(list->end(),MALE_CALF);
			list->insert(list->end(),YOUNG_BULL);
			list->insert(list->end(),OLD_BULL);
			sellingPriorityList->insert(sellingPriorityList->end(), list);

			list = new FarmManager::CriteriaList();
			list->insert(list->end(),OLD_COW);
			sellingPriorityList->insert(sellingPriorityList->end(), list);

			list = new FarmManager::CriteriaList();
			list->insert(list->end(),HEIFER_PRE_BREEDING);
			sellingPriorityList->insert(sellingPriorityList->end(), list);

			list = new FarmManager::CriteriaList();
			list->insert(list->end(),HEIFER_RDY_BREEDING);
			sellingPriorityList->insert(sellingPriorityList->end(), list);

			list = new FarmManager::CriteriaList();
			list->insert(list->end(),CALF);
			sellingPriorityList->insert(sellingPriorityList->end(), list);

			list = new FarmManager::CriteriaList();
			list->insert(list->end(),PREGNANT);
			list->insert(list->end(),DAIRY_COW);
			sellingPriorityList->insert(sellingPriorityList->end(), list);
		default:
			list = new FarmManager::CriteriaList();
			for(int i = Cow_Trade_Criteria::NUMBEROFTYPES -1; i >= 0 ; i--){
				list->insert(list->end(),static_cast<Cow_Trade_Criteria>(i));
			}
			sellingPriorityList->insert(sellingPriorityList->end(), list);
	}
	registeredCowsToSell = new Cow::UnorderedSet();
}

FarmManager::~FarmManager(){
	this->myFarm = nullptr;
	this->system = nullptr;
	delete registeredCowsToSell;
}

void FarmManager::manage(){

	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "FarmManager: starting to manage farm" << std::endl;
	#endif
	this->resetGroupsOfAllCowsOfAllHerds();

	// Check if there is a quarantine restriction on selling and buying animals
	if( this->isUnderQuarantine() && System::getInstance(nullptr)->activeStrategy->applyQuarantineOnBuying) return;
	std::set<Demand*>* requests = new std::set<Demand*>();

	this->calculateDemand(requests);

	if(requests->size() > 0){

		this->postDemand(requests);
	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "FarmManager: posted demand" << std::endl;
	#endif
	}
	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "FarmManager: starting offering process" << std::endl;
    #endif

	// TODO What is the role of this conditional here?
	if (this->myFarm->getType() == SLAUGHTERHOUSE){
        delete requests;  // The newly created "requests" should now release its reserved memory
        return;
	}

	Cow::UnorderedSet* cowsToSell = new Cow::UnorderedSet();

	this->chooseCowsToOffer(cowsToSell);
	if(!this->isUnderQuarantine()){
		this->postOffer(cowsToSell);
	}else{
		//TODO: this is actually just a quick fix for a problem occurring when quarantine is used. Make this nice code!
		for(auto cow : *cowsToSell){
			this->system->getMarket()->sellDirectlyToSlaughterHouse(cow);
		}
		//TODO addressing the memory leak of the previously assigned container. To be changed with the rest of the block.
		delete cowsToSell;  // The newly created cowsToSell should now release its reserved memory
	}

#ifdef _FARM_MANAGER_DEBUG_
	//delete cowsToSell; //don't delete this here. It will be deleted in the market, when the offer has been processed
	//std::cout << "bought: " << requests->size() << " sold: " << cowsToSell->size() << " state: " << this->myFarm->total_number() << std::endl;
//	std::vector<Herd*>* herds = this->myFarm->getHerds();
//	if((*herds)[0]->total_number() < this->plannedNumberOfCows[0]){
//		for(auto request: *requests){
//			std::cout << "request:  with " << request->numberOfDemandedCows << " cows and crit " << (int) request->crit << std::endl;
//		}
//		std::cout << "trying to sell " << cowsToSell->size() << " cows" << std::endl;
//	}
#endif
	delete requests;  // The newly created "requests" should now release its reserved memory
	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "FarmManager: finished managing" << std::endl;
	#endif
}

void FarmManager::readjustToFarmSize(){

	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "readjusting to farm size of farm at "<< this->myFarm  << std::endl;
	#endif
	std::vector<Herd*>* herds = this->myFarm->getHerds();
	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "got to know the herds at " << herds  << std::endl;
			std::cout << "creating the array of size " << herds->size()  << std::endl;
	#endif

	if(this->plannedNumberOfCows != nullptr){
		delete[] this->plannedNumberOfCows;
		this->plannedNumberOfCows = nullptr;
		}
	this->plannedNumberOfCows = new int[ herds->size() ];
	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "filling the array" << std::endl;
	#endif
	for(int i = 0; i < herds->size(); i++){

		this->plannedNumberOfCows[i] = (*herds)[i]->total_number();
		#ifdef _FARM_MANAGER_DEBUG_
			std::cout << "setting planned herd size to " << this->plannedNumberOfCows[i] << std::endl;
		#endif
	}
    #ifdef _FARM_MANAGER_DEBUG_
	std::cout << "adjusted" << std::endl;
    #endif
}

void FarmManager::postDemand(std::set<Demand*>* requests){
	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "Processing " << requests->size() << " new requests" << std::endl;
		int i=0;
	#endif
	for (auto request : *requests){
		#ifdef _FARM_MANAGER_DEBUG_
			std::cout << "Farmmanager: Posting request for " << request->numberOfDemandedCows <<" cows of criteria " << request->crit << " at farm " << this->myFarm->id<< std::endl;
		#endif
		this->system->getMarket()->register_demand(request);
		#ifdef _FARM_MANAGER_DEBUG_
			std::cout << "Done." << std::endl;
		#endif
	}
}

inline void FarmManager::postOffer(Cow::UnorderedSet* cowsToSell){

	Offer* o = new Offer(cowsToSell, this->myFarm);    //This block of memory will be released at the end of
    // register_offer() in Market.cpp
	this->system->getMarket()->register_offer(o);
}

inline void FarmManager::resetGroupsOfAllCowsOfAllHerds(){
	std::vector<Herd*>* herds = this->myFarm->getHerds();
	for (auto herd : *herds)
		herd->reevaluateGroupsOfAllCows();
}

void FarmManager::chooseCowsToOffer(Cow::UnorderedSet* cowsToSell){
	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "FarmManager: choosing cows" << std::endl;
	#endif
	int num = 0;
	Cow_Trade_Criteria crit;
	GroupPriorityList::iterator it;
	int numberOfCowsToSell = this->standardCalculateOverallNumberToSell();
	if (numberOfCowsToSell < this->sellingMargin)
		return;
	FarmManager::CriteriaList::iterator it2;
	for(it = sellingPriorityList->begin(); it != sellingPriorityList->end(); it++){
		for(it2 = (*it)->begin(); it2 != (*it)->end(); it2++){
			crit = (*it2);
			int groupnum = (*it)->size();

			num = calculateNumberOfAnimalsPerGroup(crit, numberOfCowsToSell, groupnum, cowsToSell);
//			std::cout << num << std::endl;
			if(num > 0)
				this->chooseCowsToOfferFromGroupAndAddToSellingGroup(num,crit , cowsToSell);
		}
	}
}

void FarmManager::standardCalculateDemand(std::set<Demand*>* requests){
	// Ideally you would want to buy the difference between the number of cows you want to own and the number of cows you own
	// I thought this should be split up on the different groups in terms of age, but
	// most farms just buy pregnant cows on that step so we will just buy pregnant cows

	int numberOfMissingCows = this->standardCalculateOverallNumberToBuy();
	if( numberOfMissingCows > this->buyingMargin ){
		Demand *d = new Demand(PREGNANT, numberOfMissingCows, this->myFarm);
		requests->insert(d);
	}
}

int FarmManager::standardCalculateOverallNumberToSell(){
	int numberOfCowsToSell = 0;
	int difference = 0;
	//int numberOfAnimalsWithType = 0;
	double tmp = this->replacementPercentage;
	/// If under quarantine the farm cannot buy any animals for herd rejuvenation purposes
	if(this->myFarm->isUnderQuarantine()){
		this->replacementPercentage = 0.0;
	}
	std::vector<Herd*>* herds = this->myFarm->getHerds();

	for (int i=0; i < herds->size(); i++){
		// take the number of cows that we want to have. Subtract the percentage that we want to replace and the number
        // of existing cows. -> number of cows that are needed. If this is negative, the value of this number is the number of cows we want to sell.
		difference = (int) (this->plannedNumberOfCows[i]*(1.0-this->replacementPercentage)) - (*herds)[i]->total_number();  // implicit floor for performance
		if(difference < 0 ) {
            numberOfCowsToSell -= difference;
            //if(numberOfCowsToSell > 5000)
                //std::cout << "Farm ID: " << myFarm->id << std::endl;
            //std::cout << numberOfCowsToSell << std::endl;
        }
/*		else
			// TODO Sell more of your cows if you need more cows?
			numberOfCowsToSell += difference;*/
	}
	/// The replacement percentage has a global scope for all the farms, therefore, if changed, we reinstate it
	if(this->myFarm->isUnderQuarantine()){
		this->replacementPercentage = tmp;
	}
	return numberOfCowsToSell;
}

int FarmManager::standardCalculateOverallNumberToBuy(bool replace){
	std::vector<Herd*>* herds = this->myFarm->getHerds();
	int numberOfMissingCows = 0;
	int difference = 0;
	for (int i=0; i < herds->size(); i++){
		if(replace){
            // if the difference is negative, this herd does not need any new cows
			difference = (int) ceil( this->plannedNumberOfCows[i]*(1.0-this->replacementPercentage) ) - (*herds)[i]->total_number();
		}else{
			difference = this->plannedNumberOfCows[i] - (*herds)[i]->total_number();
		}
		if(difference > 0 )
			numberOfMissingCows += difference;
//		else
//			numberOfMissingCows -= difference;
	}
	return numberOfMissingCows;
}

int FarmManager::standardCalculateNumberOfAnimalsPerGroup(Cow_Trade_Criteria crit,int overallNumber,int groupNum, Cow::UnorderedSet* cows){
	#ifdef _FARM_MANAGER_DEBUG_
			std::cout << "FarmManager: calculating how many cows to sell for the current criteria " << crit << std::endl;
		#endif

	int numberOfAnimalsWithType = 0;
	std::vector<Herd*>* herds = this->myFarm->getHerds();
	for (int i=0; i < herds->size(); i++)
		numberOfAnimalsWithType = (*herds)[i]->getNumberOfCowsInGroup(crit);

	int datnum = (int) ((double)(overallNumber - cows->size())/groupNum);
	switch(this->sellingStrategy){
		case OLD_COWS_FIRST:
			if(cows == nullptr){
				std::cerr << "With selling strategy OLD_COWS_FIRST you actually need to give me the damn pointer to a list of cows that is supposed to be sold, tard!";
				exit(9000);
			}
			// either return the number of animals of that group or the difference between the overall needed number of cows and the number of cows that is already going to be sold

			return datnum > numberOfAnimalsWithType ? numberOfAnimalsWithType : datnum;

		break;
		default: // sellingStrategy NONE or EQUALLY_DISTRIBUTED

		// we want to split up the number of cows, that we want on all kinds of cows
		// #ignore int quotient problems
		#ifdef _FARM_MANAGER_DEBUG_
			std::cout << "FarmManager: returning the calculated number" << std::endl;
		#endif
//		if((int) (overallNumber*(double ) numberOfAnimalsWithType / (double) this->myFarm->total_number()) <= 0){
//		std::cout << this->standardCalculateOverallNumberToBuy() << "\t" << cows->size() << "\t" ;
//		std::cout << "calc: " <<  overallNumber << "\t" << numberOfAnimalsWithType << std::endl;}
		return (int) (overallNumber*(double ) numberOfAnimalsWithType / (double) this->myFarm->total_number());
		}
}

void FarmManager::standardOfferingMethod(int numberOfCowsToSell, Cow_Trade_Criteria crit, Cow::UnorderedSet* cows){
	// you get the number of cows you want to sell
	// calculate the number of cows of that type you can get

	std::vector<Herd*>* herds = this->myFarm->getHerds();
	int numberToSell[herds->size()];
	int totalNumber = 0;

	for (int i=0; i < herds->size(); i++){
		numberToSell[i] = (*herds)[i]->getNumberOfCowsInGroup(crit);
		totalNumber += numberToSell[i];
	}
	#ifdef _FARM_MANAGER_DEBUG_
		std::cout << "FarmManager: got the number of cows to sell: " << totalNumber << std::endl;
	#endif
	// TODO probably react if the number of cows to sell of that type is higher than the number of cows that are there
	if(totalNumber < numberOfCowsToSell){
		#ifdef _FARM_MANAGER_DEBUG_
		std::cerr << "there is not enough cows of that type " << std::endl;
		#endif
		//numberOfCowsToSell = totalNumber;

		}

	totalNumber = numberOfCowsToSell;
	if(totalNumber > 0){
		for (int i=0; i < herds->size(); i++){
		//	std::cout << "FarmManager: choosing "<<totalNumber << "  cows from herd "<< (*herds)[i] <<  " with size " << (*herds)[i]->total_number() << " for criteria " << crit << "from farm with size " << this->myFarm->total_number() <<  std::endl;
			//for every herd: take the appropraite number of cows from each herd by splitting up
			#ifdef _FARM_MANAGER_DEBUG_
				std::cout << "FarmManager: choosing "<< totalNumber << "  cows from herds "<< herds << " for criteria " << crit << std::endl;
			#endif
			// get random cows from that group from that herd
			(*herds)[i]->getNRandomCowsFromGroup(totalNumber, crit, cows);
			totalNumber -= cows->size();
			if(totalNumber <= 0)
				break;
			//after knowing how many cows to take, generate random indices, which kinds of cows you will choose
			//add those cows to the set
		}
	}
}

FarmManagerSellChoosingStrategy FarmManager::iniInputToSellingStrategy(std::string& input){
	if(input.compare("evenlyDistributed") == 0)
		return EQUALLY_DISTRIBUTED;
	else if(input.compare("OldCowsFirst") == 0)
		return OLD_COWS_FIRST;
	else if(input.compare("NONE") == 0)
		return NONE;

	return NONE;
}

void FarmManager::registerCowForSale(const Cow* cow){
	if(this->myFarm->myType == SLAUGHTERHOUSE)    // A slaughterhouse does not sell cows
		return;
	this->system->getMarket()->sellDirectlyToSlaughterHouse(cow);
}

inline bool FarmManager::isUnderQuarantine(){
	return this->myFarm->isUnderQuarantine() && System::getInstance(nullptr)->activeStrategy->respectQuarantine;
}