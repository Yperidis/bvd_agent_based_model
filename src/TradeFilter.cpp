#include "TradeFilter.h"
#include "Events.h"
#include "Cow.h"
#include "Herd.h"
#include "Farm.h"
#include <ostream>
TradingFilter::TradingFilter(){}
TradingFilter::~TradingFilter(){}
bool TradingFilter::operator()(const Trade_Event* event){
	return this->check(event);
}

WellSlaughterhouseFilter::WellSlaughterhouseFilter():TradingFilter(){}
WellSlaughterhouseFilter::~WellSlaughterhouseFilter(){}

bool WellSlaughterhouseFilter::check(const Trade_Event* event){
	Cow* c = Cow::get_address(event->id);
	if(c->herd->farm->getType() == WELL && event->farm->getType() == SLAUGHTERHOUSE)
		return false;
		
		
	return true;
}

bool SameFarmFilter::check(const Trade_Event* event){
	Cow* c = Cow::get_address(event->id);
	//std::cout << "checking for same farm "  << c->herd->farm->id << " " << event->farm->id << " in event " << event << std::endl;
	if(c->herd->farm->id == event->farm->id)  // The farm's ID which is attempting the trade should not be the same with
		return false;						  // that of the farm to which the cow belongs to (i.e. self-trade).
		
		
	return true;
}
