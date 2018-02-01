#ifndef __system_h_
#define __system_h_

#include <iostream>
#include <unordered_set>
#include <vector>
#include <iomanip>
#include <signal.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include "BVD_Random_Number_Generator.h"


class AdvancedOutput;
class Farm;	   //Forward declaration. Real declaration in Farm.h
class Market;      //Forward declaration. Real declaration in Market.h
class INIReader;
class Cow;

class Event;
struct BVDContainmentStrategy;
class BVDSettings;

class System
{
 public:
  //static System* getInstance();
  static System* getInstance(INIReader* reader);//(double start_time , std::string output_filename="test.h5" , bool overwrite=true , double dt_log = 1 , double dt_write=10, double dt_manage = bvd_const::tradingTimeIntervall.MONTHLY );


  void schedule_event( Event* );//Done

  void execute_next_event();//Done
  void invalidate_event( Event* );//Done

  void register_farm( Farm * );//Done
  void unregister_farm( Farm * );

  void register_market( Market * );//Done

  void print_state( std::ostream& );

  double current_time();//Done

  void dump_queue();

  void print_next_event();

  void run_until( double end_time );

  void set_log_interval( double dt_log );
  void set_write_interval( double dt_write );


  void log_state();

  	AdvancedOutput *output;
	BVDContainmentStrategy* activeStrategy;
  	Random_Number_Generator rng;
	double getCurrentTime();
	Market* getMarket();
	static INIReader* reader;
	double _dt_manage;
	void scheduleFutureCowIntros();

	void addCow(Cow* c);
  //Testing relevant methods
  Event_queue getEventQueue();
 private:

 	void scheduleContainmentEvents();
 	void scheduleEartagEvents();
 	void scheduleVaccinationEvents();
 	void scheduleJungtierfensterEvents();
	Event_queue queue;
	Event_queue memorySaveQ;
	double _current_time;
	std::unordered_set< Event* > invalidated_events;
	std::vector< Farm* >         farms;
	Market* market;
	void _execute_event(Event* e );
	double _dt_log;
	double _dt_write;

	bool stop;
	unsigned int no_of_events_processed;
	static void handleSystemError(int sig);
	static System* _instance;

  //Initializer Stuff
	System(double start_time , double dt_log = 1 , double dt_write=10, double dt_manage = bvd_const::tradingTimeIntervall.MONTHLY );//Done
  ~System();
    BVDSettings* mySettings;

	class CGuard
	{
	public:
	   ~CGuard();

	};

};
#endif
