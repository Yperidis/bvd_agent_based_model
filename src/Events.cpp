#include "Events.h"
#include <queue>
#include <vector>


Event::Event(double exec_time , Event_Type event_type , int _id  ) 
  :execution_time{exec_time},
   type{event_type},
   id{_id},
  dest(Destination_Type::COW),farm(nullptr),valid(true){}  //An event is set to be valid upon initialization

///Return a Boolean, derived from the defined Event enumerable class, for all cases of infection rate changing events
/// (according to Viet 2004 this should change for every change in the PI, TI and total population of the herd in the
/// running time). Therefore, every event set in the priority queue in Events.h related to the herd/farm population or
/// the infectious status of the animal should be taken into account as a potentially infection rate changing event.
bool Event::is_infection_rate_changing_event() const {
    return type >= Event_Type::BIRTH;

     //type != Event_Type ::END_OF_MA && type != Event_Type::VACCINATE &&type != Event_Type::END_OF_VACCINATION
    //( (type >= Event_Type::END_OF_MA) && (type <= Event_Type::RECOVERY) ) || ( (type >= Event_Type::VACCINATE) && (type <= Event_Type::END_OF_VACCINATION) )
    //Original working version: return type >= Event_Type::BIRTH
}

bool Event::is_trade_event() const { return type==Event_Type::TRADE; }

Trade_Event::Trade_Event( double exec_time , int cow_id , Farm* destination_farm ) : 
  Event( exec_time , Event_Type::TRADE , cow_id )
{
  dest = Destination_Type::FARM;
  farm = destination_farm;
}

System_Event::System_Event( double exec_time , Event_Type type ) :
  Event ( exec_time , type , -1 )
{
	farm = nullptr;
  dest = Destination_Type::SYSTEM;
}

FARM_EVENT::FARM_EVENT(double exec_time, Event_Type type, Farm *f):Event( exec_time , type , -1 ){
	dest = Destination_Type::FARM;
	farm = f;
	
}
