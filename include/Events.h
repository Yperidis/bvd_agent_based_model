#ifndef __events_h__
#define __events_h__
#include <queue>
#include <vector>

class Cow;  //Forward declaration.. Real declaration in Cow.h
class Herd; //Forward declaration.. Real declaration in Herd.h
class Farm; //Forward declaration.. Real declaration in Farm.h

/**
The possible events in our simulation
 */

enum class Event_Type
  { ABORTION     = 2,
      INSEMINATION = 3,
      CONCEPTION   = 4,
      BIRTH                 = 100,
      DEATH                 = 105,
      END_OF_MA             = 106,
      INFECTION             = 107,
      RECOVERY              = 108,
      TRADE                 = 300,//Trade Events should always be the last events to be handled on the same day -> they need to have the highest number for the event type
      REMOVECOW				= 301,
      SLAUGHTER				= 200,
      CULLING				= 201,
      VACCINATE				= 202,
      END_OF_VACCINATION 	= 203,
      LOG_OUTPUT            = -1,
      WRITE_OUTPUT          = -2,
      STOP                  = -3,
      MANAGE				= -4,
      EARTAG 				= -5,
      TEST					= -100,
      ANTIBODYTEST			= -101,
      VIRUSTEST				= -102,
      VACCINATION			= -103,
      QUARANTINEEND			= -104,
      JUNGTIER				= -105,
      JUNGTIER_EXEC			= -106,
      JUNGTIER_SMALL_GROUP = -107,
      ChangeContainmentStrategy = -108

      };

// At least the numbers of the system events should stay in this particular order because
// the second sorting criterion (when exec times are equal) is the id number.
// If there is log, write stop at the same time, they should be executed in that order (in order to also write the last achieved data point to file).

enum class Destination_Type { COW, HERD, FARM , SYSTEM };

class Event
{
 public:
  Event(double exec_time , Event_Type ev_type , int cow_id );

  /// Absolute time, when event is to be executed
  double execution_time;
  /// Type of event
  Event_Type type;
  const int id;     // id of the cow for this event
  Destination_Type dest;
  Herd* herd; // Only initialized if dest==HERD
  Farm* farm; // Only initialized if dest==FARM
  bool valid;
   bool is_infection_rate_changing_event()const;
   bool is_trade_event() const;
};


class Trade_Event : public Event
{
 public:
  Trade_Event( double exec_time, int cow_id , Farm* destination_farm );
};

class System_Event : public Event
{
 public:
  System_Event( double exec_time , Event_Type type );
};
class FARM_EVENT: public Event{
	public:
	FARM_EVENT(double exec_time, Event_Type type, Farm *farm);
};
class Event_Pointer_Sort_Criterion    //Compares two elements of the queue's container and sorts the accordingly
{
 public:
  bool operator() (Event const * const  e1 , Event const * const e2)    //const pointer to const Event type
  {
    if (e1->execution_time < e2->execution_time) return false;
    if (e2->execution_time < e1->execution_time) return true;
    //Reaching this point means that the execution times of e1 and e2 are the same.
    if (e2->type < e1->type) return false;
    if (e1->type < e2->type) return true;
    //Reaching this point means that the types of e1 and e2 are also the same
    return e1->id < e2->id;    //true if the cow id of e1 is smaller than that of e2
  }
};

typedef std::priority_queue< Event* , std::vector< Event*> , Event_Pointer_Sort_Criterion > Event_queue;


#endif