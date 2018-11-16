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
  { ABORTION     = 2,  // Cow event
      INSEMINATION = 3,  // Cow event
      CONCEPTION   = 4,  // Cow event
      BIRTH                 = 100,  // Cow event
      DEATH                    = 105,  // System event. Also appears in Cow, but is not implemented there.
      END_OF_MA             = 106,  // Cow event
      INFECTION             = 107,  // Cow event appearing in the infection rate change of the Farm as well
      RECOVERY              = 108,  // Cow event
      TRADE                 = 300,  // Trade Events should always be the last events to be handled on the same day. Farm event
      REMOVECOW				= 301,  // Cow event (movement event and therefore similar to TRADE)
      SLAUGHTER				= 200,  // System event
      CULLING				= 201,  // System event
      VACCINATE				= 202,  // Cow event
      END_OF_VACCINATION 	= 203,  // Cow event
      LOG_OUTPUT            = -1,  // System event
      WRITE_OUTPUT          = -2,  // System event
      STOP                  = -3,  // System event
      MANAGE				= -4,  // System event
      EARTAG 				= -5,  // Redundant by the TEST event
      TEST					= -100,  // Cow event. Tissue test (scheduled at birth and realised only once). Antigen test.
      ANTIBODYTEST			= -101,  // Cow event. So far equivalent to JUNGTIER_SMALL_GROUP. Is not otherwise implemented.
      VIRUSTEST				= -102,  // Cow event. Appears at the herd level. Blood test (more expensive than TEST-->ear tag, antigen). Periodic implementation
      VACCINATION			= -103,  // Redundant by the VACCINATE event
      QUARANTINEEND			= -104,  // Farm event
      JUNGTIER				= -105,  // Not implemented
      JUNGTIER_EXEC			= -106,  // System event
      JUNGTIER_SMALL_GROUP = -107,  // Farm event. Equivalent to ANTIBODYTEST. Appears at the Farm level.
      ChangeContainmentStrategy = -108  // System event (should have the highest priority to have an effect)
      };

// At least the numbers of the system events should stay in this particular order because
// the second sorting criterion (when the execution times are equal) is the id number.
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

class TEST_EVENT: public Event{
public:
    TEST_EVENT(double exec_time, Event_Type type, int cow_id, Farm *farm);
};

class Event_Pointer_Sort_Criterion    //Compares two elements of the queue's container and sorts them accordingly
{
public:
    bool operator() (Event const * const  e1 , Event const * const e2)    //const pointer to const Event type
    {
      ///First sorting criterion for the pair of events: according to their execution time
      if (e1->execution_time < e2->execution_time) return false;
      if (e1->execution_time > e2->execution_time) return true;
      //Reaching this point means that the execution times of e1 and e2 are the same.
      ///Second sorting criterion for the pair of events: according to their type (see event enumerable class)
      if (e1->type > e2->type) return false;
      if (e1->type < e2->type) return true;
      ///Reaching this point means that the types of e1 and e2 are also the same. In this case the event with the
      ///smallest (animal) id will be executed first
      return e1->id < e2->id;
    }
};

typedef std::priority_queue< Event* , std::vector< Event*> , Event_Pointer_Sort_Criterion > Event_queue;


#endif