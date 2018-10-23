#ifndef __herd_h_
#define __herd_h_
#include <vector>
#include <ostream>
#include "Cow.h"

//class Cow;         // Forward declaration.. Real declaration in Cow.h
class Event;       // Forward declaration.. Real declaration in Events.h
class Farm;


class Herd
{
 public:
  Herd( Farm * ); //Done
  ~Herd();
  void pull_cow( Cow * ); //Done
  void push_cow( Cow * ); //Done
  
  void execute_event(const Event* ); //Done
  void number_of_PI_increase(int n=1);
  void number_of_TI_increase(int n=1);
  void number_of_PI_decrease(int n=1);
  void number_of_TI_decrease(int n=1);

	int getNumTI();
	int getNumPI();
	int getNumR();
	int getNumS();
	
  int total_number();//Done
  int number_of_younglings();
  void add_pi_cow(Cow* cow);
  void remove_pi_cow(Cow* cow);
  void add_r_cow(Cow*);
  void remove_r_cow(Cow*);
  void add_ti_cow(Cow*);
  void remove_ti_cow(Cow*);
  void add_cow_to_susceptible( Cow* );    //Done
  void remove_cow_from_susceptible( Cow* );//Done
	std::vector<Cow*>* getPIs();
  //  void infection_rate_has_changed( double time );
  Cow* random_S_cow();//Done
  
  Farm* farm;

  void print_susceptible_ids(std::ostream& );
  void print_all_cows( std::ostream& );
  
  void print_info(std::ostream& );
  void reevaluateGroupsOfAllCows();
  void reevaluateGroup( Cow* cow) ;
  int getNumberOfCowsInGroup(Cow_Trade_Criteria crit);
  void getNRandomCowsFromGroup(int n,Cow_Trade_Criteria crit, Cow::UnorderedSet* setOfCows);
  Cow::UnorderedSet all_my_cows;
  
  void testAllCows();
  void addCowToUnknownList(Cow* cow);
  void removeCowFromUnknownList(Cow* cow);
  Cow::UnorderedSet getNUnknownCows(int N);
 private:
  inline Cow::UnorderedSet* getRelevantGroup(Cow_Trade_Criteria crit);
  void putIntoRelevantGroup(Cow* cow);
  std::vector<Cow*> susceptible_cows;
	std::vector<Cow*> pi_cows;
  std::vector<Cow::UnorderedSet*>* cowgroups;
  Cow::UnorderedSet unknownCows;

  unsigned int number_of_PI;
  unsigned int number_of_TI;
  unsigned int number_of_R;
  unsigned int number_of_S;
  
};

// The bookkeeping of the infection stats within the herd is done partially by this class and partially by the cow class.
// The rule is:
// When a cow is introduced to or taken from the herd, this is to be done via the push_cow and pull_cow methods.
// Then the herd class will take care of this.
// This applies to BIRTH, DEATH and trade events.
// When a cow changes its status, the cow takes care of the bookkeeping by changing the int variables with the number of cows with different stats
// and modifying the susceptible_cow set.
// This applies to INFECTION, RECOVERY and END_OF_MA events.



#endif