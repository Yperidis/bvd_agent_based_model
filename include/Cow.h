#ifndef __cow_h_
#define __cow_h_
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "Events.h"
#include <string>
#include "projectImports/inih/cpp/INIReader.h"
/**
A Cow in our model of BVD.
*/

class System; // Forward declaration. Actual declaration in System.h
//class Event;  // Forward declaration. Actual declaration in Events.h
class Herd;   // Forward declaration. Actual declaration in Herd.h
//class Event_queue; // Forward declaration. Actual declaration in Events.h
class Cow;

//
enum Cow_Trade_Criteria{
	CALF = 0,
	HEIFER_PRE_BREEDING = 1,
	HEIFER_RDY_BREEDING =2,
	INFERTILE =3,
	PREGNANT =4,
	DAIRY_COW = 5,
	OLD_COW = 6,
	MALE_CALF = 7,
	YOUNG_BULL = 8,
	OLD_BULL = 9,
	NUMBEROFTYPES = 10//if you add more cow trade criteria, you need to change this number according to the number of criteria
};

/// The possible states of an embryo. (NO_CALF meaning that there is no embryo at the moment)
enum class Calf_Status
  { NO_CALF               = 0,
    SUSCEPTIBLE           = 1,
    PERSISTENTLY_INFECTED = 2,
    IMMUNE                = 4,
    CRIPPLE               = 5,
    ABORT                 = 6,
    INFERTILE 				= 7
    };
/// The possible infection states of a cow.
enum class Infection_Status
  { SUSCEPTIBLE           = 1,
    PERSISTENTLY_INFECTED = 2,
    TRANSIENTLY_INFECTED  = 3,
    IMMUNE                = 4};

enum class KnownStatus{
	NOSTATUS = 1,
	NEGATIVE = 2,
	POSITIVE = 3,
	POSITIVE_ONCE = 4,
	POSITIVE_TWICE = 5	,
	POSITIVE_MOTHER = 6
};




class Cow_Pointer_Sort_Criterion
{
 public:
  bool operator() (Cow const * const c1 , Cow const * const c2);
};



class Cow
{
	public:
		Cow( double birth_time , Cow* my_mother ) ; /// Constructor
		Cow( double birth_time, Cow* mother, bool female);
		Cow( std::string cowName);
		~Cow();
		typedef std::set<Cow*,Cow_Pointer_Sort_Criterion> Set;
		typedef std::unordered_set<Cow*> UnorderedSet;
		int               id() const;                  /// Identification number. This is unique for the entire simulation
		double            birth_time;            /// Time of birth
		bool              female;                /// Whether the calf is female
		Infection_Status  infection_status;
		double            last_conception_time;  /// Time of last conception. Negative value means: no conception so far.;
		int               calving_number;        /// Number of remaining calvings
		bool              has_been_pregnant_at_all_so_far;    // Self explanatory, innit?
		Calf_Status       calf_status;           /// State of the calf, can be NO_CALF which means that this Cow is not pregnant at the moment
		Herd *            herd	;                  /// Pointer to the herd, this cow belongs to
		double            buf_time=0;    //For debugging purposes on the vaccination times
        int               cntr=0;    //For debugging purposes

		Cow *             mother;
		Cow::UnorderedSet          children;
		Cow::UnorderedSet*		  Group; // the group in the herd to which the cow belongs
		void execute_event( Event * );
		Cow_Trade_Criteria getCowTradeCriteria();
		void register_future_infection_rate_changing_event( Event * e );
		//Even though the farm keeps track of these as well for all cows, these are necessary as a cow might carry future infection rate changing events which would apply to a different farm after a trade.
		std::unordered_set<Event* > future_irc_events_that_move;
		double *birthTimesOfCalves;

		bool hasBeenTestedPositiveYet;
        Event* planned_birth_event;

		double age() const;
		KnownStatus knownStatus;
		int numberOfVaccinations;

		static Cow *   get_address( int search_id ); /// This returns the memory adress of a cow. Will return null if the cow is not there (anymore).
		static int     total_number();
		static int     number_of_living_cows();
		static System* system;
		static void    set_system( System * );         /// This must be called once, before the simulation can run.

		Cow::UnorderedSet* getGroup();
		void setGroup(Cow::UnorderedSet* set);
		double timeOfInfection;
		double timeOfLastCalving;
		double lastTestTime, firstTestTime;
		bool testCow(const Event* e);
		bool isTestedPositive(const Event* e);

	private:
		Event* end_of_vaccination_event;

		void init(const double& birth_time, Cow *mother, bool female);
		int _id;
		void execute_BIRTH(        const double& time   );//Done
		void execute_ABORTION(     const double& time   );//Done
		void execute_INSEMINATION( const double& time   );//Done
		void execute_CONCEPTION(   const double& time   );//Done
		void execute_DEATH(        const double& time   );//Done
		void execute_END_OF_MA(    const double& time   );//Done
		void execute_INFECTION(const double& time   );//Done
		void execute_RECOVERY(     const double& time  );//Done
		void execute_END_OF_VACCINATION(const double& time);
		void handle_rest_time_after_ABORTION_or_BIRTH( double time );


		void scheduleNextTest();
		void scheduleVaccination(const double& time) const;
		void runVaccination(const double& time);
		static std::unordered_map< int , Cow* > all_living_cows;
		static int cow_id_counter;
		static Calf_Status stringToCalfStatus(const std::string& input);
		static Infection_Status stringToInfectionStatus(const std::string& input);
		void scheduleInsemination(const double& time, double& vaccTime, const Cow* c=NULL);

		bool testAgain();
};

#endif
