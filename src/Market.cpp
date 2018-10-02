#include <BVDContainmentStrategy.h>
#include "Market.h"
#include "System.h"
#include "Herd.h"
#include "Farm.h"
#include "TradeFilter.h"
#include "CowWellFarm.h"
#include "Slaughterhouse.h"
#include "projectImports/inih/cpp/INIReader.h"
#include "Utilities.h"
Offer::Offer(Cow::UnorderedSet* cows, Farm* src):cows(cows),src(src){

}
Offer::~Offer(){
	delete this->cows;
}
Cow::UnorderedSet*  Offer::getCows(){
	return this->cows;
	//setOFCows = *(this->cows);
}

Demand::Demand(Cow_Trade_Criteria criteria, int number, Farm* source):crit(criteria), numberOfDemandedCows(number), src(source){
	if(number <=0)
		this->numberOfDemandedCows = 0;
}
Demand::~Demand(){}


Market::Market(System* system):s(system){
	cowqueue *cowl;
	demandqueue *demandl;
	cowQs = new cowQvec();
	demandQs = new cowDvec();
	lastID = -1;

	std::string slaughterhousetype = System::reader->Get("trade", "slaughterHouseType" , "dump");
	if(slaughterhousetype.compare("dump") == 0)
		dump = true;
	else
		dump = false;
	slaughterHouses = new std::vector<Slaughterhouse*>() ;
	sources = new std::vector<CowWellFarm*>();

	filters = new std::vector<TradingFilter*>();  // introducing filters to the market (defined in TradeFilter.cpp)
	filters->push_back( new WellSlaughterhouseFilter() );
	filters->push_back( new SameFarmFilter() );
	ignoreTypeOfDemand = System::reader->GetBoolean("trade", "ignoreTypeOfDemand" , false);
	if(ignoreTypeOfDemand){  // in this case the market will ignore the type of demands and apply those of this block
		cowl = new std::queue<Cow*>();
		demandl = new std::queue<Demand*>();
		cowQs->push_back(cowl);
		demandQs->push_back(demandl);

		// In the implementation below, if more trading criteria are added they will need to be included in the
		// following list for the market to ignore the registered types of demand.
		buyingQueueTradeCriteriaPriorities = new std::vector<Cow_Trade_Criteria>();
		buyingQueueTradeCriteriaPriorities->push_back(PREGNANT);
		buyingQueueTradeCriteriaPriorities->push_back(DAIRY_COW);
		buyingQueueTradeCriteriaPriorities->push_back(HEIFER_RDY_BREEDING);
		buyingQueueTradeCriteriaPriorities->push_back(HEIFER_PRE_BREEDING);
		buyingQueueTradeCriteriaPriorities->push_back(CALF);
		buyingQueueTradeCriteriaPriorities->push_back(YOUNG_BULL);
		buyingQueueTradeCriteriaPriorities->push_back(OLD_COW);
		buyingQueueTradeCriteriaPriorities->push_back(INFERTILE);
        buyingQueueTradeCriteriaPriorities->push_back(MALE_CALF);
		buyingQueueTradeCriteriaPriorities->push_back(OLD_BULL);
	}else{
		for(int i=0; i < NUMBEROFTYPES; i++){  // the market respects type demands
			cowl = new std::queue<Cow*>();
			demandl = new std::queue<Demand*>();
			cowQs->push_back(cowl);
			demandQs->push_back(demandl);
		}
	}

}

Market::~Market(){
	this->s = nullptr;
	this->flushQueues();
	for(auto cowl : *cowQs)
		delete cowl;
	for(auto demandl : *demandQs)
		delete demandl;
	delete cowQs;
	delete demandQs;
  	for(auto filter : *filters){
	  	delete filter;
  	}
  	delete filters;
  	delete slaughterHouses;
  	delete sources;
  	if(ignoreTypeOfDemand)
  		delete buyingQueueTradeCriteriaPriorities;
  	this->farms = nullptr;
}

void Market::register_offer( Offer * offer ){
	if(offer->src->getType() == SLAUGHTERHOUSE){    // We do not accept offers from slaughterhouses by definition
		delete offer;
		std::cerr << "WARNING: ATTEMPTED TO REGISTER OFFER FROM THE SLAUGHTERHOUSE." << std::endl;
		return;
	}

	Cow::UnorderedSet* cows = offer->getCows();
	cowqueue* offerQueue = nullptr;
	demandqueue* demandQueue = nullptr;
	for(auto cow : *cows){  // for every cow find the right queues depending on age and breeding activity
		switch(cow->knownStatus){

			case KnownStatus::POSITIVE:
			case KnownStatus::POSITIVE_ONCE:
			case KnownStatus::POSITIVE_TWICE:
			case KnownStatus::POSITIVE_MOTHER:

				this->scheduleTradeToSlaughterHouse(cow);
				continue;
			break;  // made redundant from the loop, but good to retain as a flow control reminder
			default:{
				std::pair< std::queue<Cow*>*, std::queue<Demand*>* > pair =
						this->getRelevantQueues( cow->getCowTradeCriteria() );
				offerQueue = pair.first;
				demandQueue = pair.second;

				Demand *d = nullptr;
				bool pushOnOfferQueue = true;
				int i = 0;
				if(offerQueue->size() == 0){
					demandqueue* dbuffQ = new demandqueue();
					while(demandQueue->size() > 0 && pushOnOfferQueue){  // search for a match between demanded groups of animals and offered ones
						i++;
						d=demandQueue->front();
						if(d->numberOfDemandedCows <= 0){
							demandQueue->pop();
							delete d;
							continue;
						}else{
							if(!this->doTheTrading(cow, d)){  // declining the trade due to self-matching
								demandQueue->pop();
								dbuffQ->push(d);
							}else{
								pushOnOfferQueue = false;  // end of search
							}
						}

					}
					this->cleanUpBufferQueue(demandQueue, dbuffQ);
				}
				if(pushOnOfferQueue) offerQueue->push(cow);

			}
			break;

		}
	#ifdef _MARKET_DEBUG_
		std::cout << "Market: handled offer with " << cows->size() << " cows" << std::endl;
	#endif
	}

	delete offer;
}

void Market::register_demand( Demand * demand ){
	if(dump && demand->src->getType() == SLAUGHTERHOUSE){  // don't accept demand from slaughterhouses when dumping
		delete demand;
		return;
	}

	std::pair< std::queue<Cow*>*, std::queue<Demand*>* > pair = this->getRelevantQueues(demand->crit); // the setting to ignore or not the type
                                                                                                        // of demand here is implicit
	std::queue<Cow*>* offerQueue = pair.first;
	std::queue<Demand*>* demandQueue = pair.second;
	//this->getRelevantQueues(demand->crit, offerQueue, demandQueue);
	demandQueue->push(demand);
	if(offerQueue->size() > 0 && demandQueue->size() > 0){
		this->matchTradingPartners(offerQueue, demandQueue);
	}
	#ifdef _MARKET_DEBUG_
		std::cout << "Market: received demand with " << demand->numberOfDemandedCows << " cows" << std::endl;
		std::cout << "Market: handled demand with crit" << demand->crit << std::endl;
	#endif
}

void Market::flushQueues(){  // Called after the end of a management period

	for(int i=0; i< NUMBEROFTYPES; i++){
		std::pair<std::queue<Cow*>*, std::queue<Demand*>*> pair = this->getRelevantQueues( static_cast<Cow_Trade_Criteria>(i) );
		while(!pair.first->empty()){
			if(this->dump){
				if(this->slaughterHouses->size() <= 0){
					std::cerr << "If you're using slaughterhouse type dump, you need to have slaughterhouses in the system" << std::endl;
					exit(1);
				}
				Cow* c = pair.first->front();
				this->scheduleTradeToSlaughterHouse(c);

			}
			pair.first->pop();
		}
		std::queue<Demand*>* demandQueue = pair.second;
		while(!demandQueue->empty()){
			Demand *d = demandQueue->front();
			if(this->dump && this->sources->size() > 0){
				while(d->numberOfDemandedCows > 0){  // fill up the needs of the farms

					Trade_Event *e = new Trade_Event(this->s->getCurrentTime() + bvd_const::standard_trade_execution_time, (*this->sources)[0]->getACowId(), d->src);
					if( !this->scheduleTrade(e) ){
						std::cerr << "ATTENTION! ATTEMPTED SELF-TRADE THROUGH THE QUEUE FLUSHING WHILE DUMPING" << std::endl;
						delete e;
						continue;
					}
					d->numberOfDemandedCows--;
					//delete d;
				}
				demandQueue->pop();
			}else
				demandQueue->pop();
			delete d;
		}
	}
}

void Market::matchTradingPartners(cowqueue* offerQueue, demandqueue* demandQueue){
	// while the demandQueue and offerQueue have entries, match them.
	cowqueue* bufferQueue = new cowqueue();
	Demand *d = nullptr;
	while(offerQueue->size() > 0 && demandQueue->size() > 0){
		d = demandQueue->front();
		this->matchDemandToOfferQueue(d, offerQueue);
		if(d->numberOfDemandedCows == 0){
			demandQueue->pop();
			delete d;
			d = nullptr;
		}
	}
	this->cleanUpBufferQueue(offerQueue, bufferQueue);
}

void Market::matchDemandToOfferQueue(Demand* demand, cowqueue* offerQueue){
	cowqueue* bufferQueue = new cowqueue();
	while(demand->numberOfDemandedCows > 0 && offerQueue->size() > 0){

		Cow* cow = offerQueue->front();
		offerQueue->pop();

		if(!this->doTheTrading(cow, demand)){  // assure that the realised trade is not self-trade
			bufferQueue->push(cow);
		}
	}
	this->cleanUpBufferQueue(offerQueue, bufferQueue);

}
template<typename T>
void Market::cleanUpBufferQueue(std::queue<T>* trueQ, std::queue<T>* bufferQ){  // this is where the actual offer-demand matching takes place
	while(bufferQ->size() > 0){
		// this is not optimal when it comes to timing, but should work for now.
		// FIXME: when timing problems arise: solve that problem
		T thing = bufferQ->front();
		trueQ->push(thing);  // The trueQ refers to the offer or demand queue, depending on where it was called from
		bufferQ->pop();
	}
	delete bufferQ;
}

inline const std::pair<Market::cowqueue*, Market::demandqueue*> Market::getRelevantQueues(const Cow_Trade_Criteria crit){

	std::pair<Market::cowqueue*, Market::demandqueue*> retPair = std::pair<cowqueue*, demandqueue*>();
	if(ignoreTypeOfDemand){
		retPair.second = (*demandQs)[0];
		retPair.first = (*cowQs)[0];
	}else{
		retPair.second = (*demandQs)[crit];
		retPair.first = (*cowQs)[crit];
	}
	return retPair;

}

bool Market::doTheTrading(Cow* cow, Demand* d){
	Trade_Event *e = new Trade_Event( this->s->getCurrentTime() + bvd_const::standard_trade_execution_time, cow->id(), d->src );
	//TODO The testing prior to trade should be incorporated in the scheduleTrade() function from a design consistency point of view. However, cow is not passed there and there are several dependencies to be considered if this change is to be made.
	if(s->activeStrategy->usesEartag){
		if(cow->age() <= 30.) {
			s->schedule_event( new Event( s->getCurrentTime(), Event_Type::TEST, cow->id() ) ); // schedule an ear tag test before the trade (less than a month of age)
		}
		else{
			s->schedule_event( new Event( s->getCurrentTime(), Event_Type::VIRUSTEST, cow->id() ) ); // schedule an blood test before the trade (over 1 month of age)
		}
		if (cow->knownStatus != KnownStatus::NEGATIVE && d->src->getType() != FarmType::SLAUGHTERHOUSE){ // if the animal is not tested negative do not commit the trade unless the destination farm is the slaughterhouse
			bool ret = false;
			delete e;
			(d->numberOfDemandedCows)--;
			return ret;
		}
	}
	bool ret = this->scheduleTrade(e);
	if(ret){
		#ifdef _MARKET_DEBUG_
			std::cout << "Market: schedule new trade" << std::endl;
		#endif

	}else{
		delete e;
	}
	(d->numberOfDemandedCows)--;
	return ret;
}

bool Market::scheduleTrade(Trade_Event* event){
	for(TradingFilter* filter : *this->filters){
		if(!(*filter)(event))  // case of self-trade or direct trade from well to slaughterhouse
			return false;
	}
	// TODO This came up with an input farm size distribution of two farms, each with 20 animals.
	if(this->lastID == event->id){  // case of animal being doubly scheduled for trade. Should be redundant from the above self-trade. Check!
//		std::cerr << "I don't remember, why I'm doing this" <<std::endl;
//		Utilities::printStackTrace(15);
	}
	lastID = event->id;

	s->schedule_event( (Trade_Event*) event );    // explicit type-casting
	return true;
}

void Market::registerFarm(Farm * f){
	if(f->getType() == WELL)
		this->registerSourceFarm((CowWellFarm*) f);
	else if(f->getType() == SLAUGHTERHOUSE)
		this->registerSlaughterHouse((Slaughterhouse*) f);
	else
		this->farms->push_back(f);
}

void Market::registerSourceFarm(CowWellFarm* f){
	this->sources->push_back(f);
}

void Market::registerSlaughterHouse(Slaughterhouse* sl){
	this->slaughterHouses->push_back(sl);
}

// TODO Make necessary changes if you want to define more than 1 slaughterhouse from the ini file
void Market::scheduleTradeToSlaughterHouse(Cow* c){
	if(c->herd->farm->getType() != WELL && c->herd->farm->getType() != SLAUGHTERHOUSE){    // Check: if not source and not drain then go to the drain
		Trade_Event *e = new Trade_Event( this->s->getCurrentTime() + bvd_const::standard_trade_execution_time, c->id(),
										  (*this->slaughterHouses)[0] );
		if( !this->scheduleTrade(e) ){
			std::cerr << "for some reason flushing to the slaughterhouse did not work" << std::endl;
			Utilities::pretty_print(e, std::cout);
			delete e;
		}
	}
}

void Market::sellDirectlyToSlaughterHouse(const Cow* cow){
	this->scheduleTradeToSlaughterHouse((Cow *) cow);
}

void Market::setFarms(std::vector<Farm*>* newfarms){

    this->farms = newfarms;
    for (auto f : *newfarms){
        if(f->getType() == WELL)
            this->registerSourceFarm((CowWellFarm*) f);
        else if(f->getType() == SLAUGHTERHOUSE){
            this->registerSlaughterHouse((Slaughterhouse*) f);
        }
    }
}
