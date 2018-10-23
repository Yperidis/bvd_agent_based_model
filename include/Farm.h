#ifndef __farm_h_
#define __farm_h_
#include <ostream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "Events.h"
#include "AdvancedOutput.h"


class Cow;   //Forward declaration... Real declaration in Cow.h
class System;//Forward declaration... Real declaration in System.h
class FarmManager;
enum FarmType{
	NORMAL_FARM,
	SMALL_FARM,
	SLAUGHTERHOUSE,
	WELL
};
class Farm{
 public:
  Farm( System * );
  virtual ~Farm();

//   double life_expectancy_male_cow;
  int    number_of_herds();
  int id;
  void execute_event( Event * );
  
  virtual void pull_cow( Cow * )=0; // It's up to the subclasses of Farm, on how they manage their herds.
  virtual void push_cow( Cow * )=0;

  int number_of_PI() const; //Done
  int number_of_TI() const ; //Done
  int number_of_R() const;  //Done
  int number_of_S() const;  //Done
  int total_number() const; //Done
  //void get_state ( std::array<int , 4 > * input );
  
  void print_state( std::ostream& , int level_of_detail=0 );
  

  // The following methods are for the infection dynamics.
  // A Farm object keeps track of the events that are scheduled for it and that could change the infection rate.
 

  // This function yields the next time of the next scheduled event that could change the infection rate.
  // The pointer *e points to the event currently processed. It is used to distinguish between the current event and an irc event at the same time.
  double next_infection_rate_change( Event *e );         //Done

  // This function must be called, when an event that could have changed the ir has been (completely) executed.
  void   infection_rate_has_changed( Event *e );                 //Done

  //This function (called by an instance of System) registers the future infection rate changing events.
  void   register_future_infection_rate_changing_event( Event * );  //Done
	void delete_infection_rate_change_event( Event* e);
  //For each farm there is at most one(!) infection event in the queue (If not there is a bug).
  // There will never be an infection event scheduled to a timepoint after an irc event.
  // Thus, if there is an infection event in the queue, it is always the next event for the Farm.
  // However, when a trade happens there can be an infection event waiting in the queue of the destination farm.
  // This infection event needs to be invalidated and infection_rate_has_changed called afterwards.
  void  invalidate_next_infection_event();
  void 	getManaged();
  void 	holdSize();
  void jungtierCheck();

  void testAllCows();
  std::vector<Herd*>* getHerds();
  FarmType getType();
  System* system;
	bool isUnderQuarantine();
	void putUnderQuarantine();
	std::vector<Cow*>* getPIs();
	Event* next_infection_event;
	FarmManager* manager;
	FarmType myType;
	
 protected:
 	bool quarantine;
 	static int counter;
 	
  	
  	Event_queue future_infection_rate_changing_events;
	std::vector<Herd*> my_herds;
  	std::unordered_map< Herd* , std::unordered_map< Herd*, double > > 	inter_herd_transmission_coefficients; // Convention: transmission_coefficients[to][from]
  	void execute_TRADE_event( Event *e );
  	
  	
  	 Farm* farm_of_event(const Event* e);
  	 bool its_the_one(const Event* e,const Event* current);
  	 void executeQuarantineEndEvent(const Event* e);
  	 Event* lastQuarantineEndEvent;
  	 int getNumberOfCowsToTest() const;
};


#endif

// Bookkeeping of future infection-rate changing events is done via calls to the function 'infection_rate_changed'
// and the function 'register_future_infection_rate_change.

