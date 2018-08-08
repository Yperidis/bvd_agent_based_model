#include "Herd.h"
#include "Cow.h"
#include "Events.h"
#include "Farm.h"
#include "System.h"
#include "BVD_Random_Number_Generator.h"
#include "Model_Constants.h"
#include "Utilities.h"
#include <iostream>
#include <algorithm>
#include <vector>


Herd::Herd( Farm* f ) 
{
  farm=f;
  number_of_PI = 0;
  number_of_TI = 0;
  number_of_R  = 0;
  number_of_S  = 0;
  cowgroups = new std::vector<Cow::UnorderedSet*>();
  for(int i=0; i< NUMBEROFTYPES; i++){
	  cowgroups->push_back(new Cow::UnorderedSet());
  }
}

Herd::~Herd()
{
	for(auto group: *this->cowgroups)
		delete group;
	delete cowgroups;
}

void Herd::pull_cow( Cow* c )
{
	c->Group->erase(c);
  c->herd= nullptr;
  c->Group = nullptr;
  this->removeCowFromUnknownList(c);
  switch (c->infection_status)
    {
    case Infection_Status::SUSCEPTIBLE:
      remove_cow_from_susceptible( c );
      
      break;
    case Infection_Status::TRANSIENTLY_INFECTED:
      this->remove_ti_cow(c); 
      break;
    case Infection_Status::PERSISTENTLY_INFECTED:
      this->remove_pi_cow(c);
      break;
    case Infection_Status::IMMUNE:
      this->remove_r_cow(c);   
      break;
    }
  all_my_cows.erase(c);
}

void Herd::push_cow( Cow* c )
{
  c->herd = this;
  this->putIntoRelevantGroup(c);
  this->addCowToUnknownList(c);
  switch (c->infection_status)
    {
    case Infection_Status::SUSCEPTIBLE:
      this->add_cow_to_susceptible( c );
      
      break;
    case Infection_Status::TRANSIENTLY_INFECTED:
      this->add_ti_cow(c);
      break;
    case Infection_Status::PERSISTENTLY_INFECTED:
      this->add_pi_cow(c);
      break;
    case Infection_Status::IMMUNE:
      this->add_r_cow(c);  
      break;
    }
  all_my_cows.insert(c);
}

void Herd::execute_event(const Event* )
{
  // So far, no Herd-related events exist.
}

/*void Herd::infection_rate_has_changed( double time )
{
  farm->infection_rate_has_changed(  time );
}
*/

int Herd::getNumTI(){return this->number_of_TI;}
int Herd::getNumPI(){return this->number_of_PI;}
int Herd::getNumR(){return this->number_of_R;}
int Herd::getNumS(){return this->number_of_S;}

int Herd::total_number()
{
  return   this->number_of_PI +
    this->number_of_TI +
    this->number_of_S  +
    this->number_of_R;
}
inline void Herd::add_pi_cow(Cow* cow){this->pi_cows.push_back ( cow );this->number_of_PI_increase();}

inline void Herd::remove_pi_cow(Cow* cow){
  std::vector<Cow*>::iterator it;
  it = std::find( this->pi_cows.begin() ,this->pi_cows.end() , cow );
  if ( it != this->pi_cows.end() ){
    this->pi_cows.erase ( it );
    this->number_of_PI_decrease();
    }
  else
    std::cerr << "Error: Tried removing cow from PI but could not find it. This should not happen!!"<<std::endl;
}
inline void Herd::add_r_cow(Cow*){this->number_of_R++;}
inline void Herd::remove_r_cow(Cow*){this->number_of_R--;}
inline void Herd::add_ti_cow(Cow*){this->number_of_TI_increase();}
inline void Herd::remove_ti_cow(Cow*){this->number_of_TI_decrease();}
inline void Herd::add_cow_to_susceptible( Cow* c ) { this->susceptible_cows.push_back ( c );this->number_of_S++; }
inline void Herd::remove_cow_from_susceptible( Cow* c )
{
  std::vector<Cow*>::iterator it;
  it = std::find( this->susceptible_cows.begin() , this->susceptible_cows.end() , c );
  if ( it != susceptible_cows.end() ){
    this->susceptible_cows.erase ( it );
    this->number_of_S--;
    }
  else
    std::cerr << "Error: Tried removing cow from S but could not find it. This should not happen!!"<<std::endl;
}

std::vector<Cow*>* Herd::getPIs(){
	return &(this->pi_cows);
}

Cow* Herd::random_S_cow()
{
  //Select an element from susceptible_cows randomly.
  
  if ( number_of_S != susceptible_cows.size() )
    {
      std::cerr << "Bookkeeping Error @t=" << farm->system->current_time() << ": number_of_S = " <<
				number_of_S << ", susceptible_cows.size()=" << susceptible_cows.size() << std::endl;
      if ( susceptible_cows.empty() )
	  std::cerr << "susceptible_cows is actually empty.";
      else
	for (auto c : susceptible_cows )
	  std::cerr << Utilities::IS_tostr.at(c->infection_status) << ",";
      std::cerr<<std::endl;
    }
    
  int id = farm->system->rng.ran_unif_int( number_of_S );
  Cow* ran_cow = susceptible_cows [id];
  if(ran_cow->infection_status != Infection_Status::SUSCEPTIBLE){
	  std::cerr << "the randomly chosen cow is not susceptible" << std::endl;
  }
  return ran_cow;
}

void Herd::print_all_cows(std::ostream& out )
{
  
  for ( auto c : all_my_cows )
    {
      Utilities::pretty_print((Cow *) c , out );
    }
  out << std::endl;
  out << "--------------------------------------------------------------------------------"<<std::endl<<std::endl;
}

void Herd::print_susceptible_ids( std::ostream& out )
{
  std::cout << "Susceptible cows: ";
  for ( auto c : susceptible_cows )
    std::cout << c->id() << ", ";
  std::cout << std::endl;
}

void Herd::print_info(std::ostream& out )
{
  print_all_cows( out );
  out << "Numbers: PI="<<number_of_PI<<", TI="<<number_of_TI<<", R="<<number_of_R<<", S="<<number_of_S<<std::endl;
  print_susceptible_ids( out );
  out << std::endl;
}

void Herd::reevaluateGroupsOfAllCows(){
	for (auto cow : all_my_cows){
		this->reevaluateGroup(cow);
	}
}

inline void Herd::reevaluateGroup( Cow* cow) {
	 cow->setGroup(this->getRelevantGroup(cow->getCowTradeCriteria()));
}
int Herd::getNumberOfCowsInGroup(Cow_Trade_Criteria crit){
	Cow::UnorderedSet* group = this->getRelevantGroup(crit);
	return group->size();
}

void Herd::getNRandomCowsFromGroup(int n,Cow_Trade_Criteria crit, Cow::UnorderedSet* setOfCows){
	#ifdef _HERD_DEBUG_
		std::cout << "HERD: trying to select " << n << " cows for offer for crit "<< crit << std::endl;
		std::cout << "HERD: By now the number of cows that is already in this set is " << setOfCows->size() << std::endl;
	#endif
	Cow::UnorderedSet *group = this->getRelevantGroup(crit);
	if(n >= group->size()){
		n = group->size();
		setOfCows->insert(group->begin(), group->end());
	}
	int * indices = new int[n];
	this->farm->system->rng.getNRandomNumbersInRange(n,0,group->size(), indices);
	#ifdef _HERD_DEBUG_
		std::cout << "got a group of " << group->size() << "cows" << std::endl;
	#endif
	std::sort(indices, indices + n);
	int j = 0;
	int i = 0;
	Cow::UnorderedSet::iterator it;
	for(it = group->begin(); it != group->end(); it++){
		#ifdef _HERD_DEBUG_
			//std::cout << "Herd: index j " << indices[j] << " i " << i << std::endl;
			#endif
		if(indices[j] == i){
			#ifdef _HERD_DEBUG_
			std::cout << "Herd: adding cow " << std::endl; 
			#endif
			setOfCows->insert(*it); 
			j++;
			if(j >= n) break;
					
		}
		i++;
	}
	delete[] indices;
	#ifdef _HERD_DEBUG_
		std::cout << "HERD: selected" << n << " cows for offer for crit "<< crit << std::endl;
		std::cout << "HERD: By now the number of cows that is already in this set is " << setOfCows->size() << std::endl;
	#endif
}

inline Cow::UnorderedSet* Herd::getRelevantGroup(Cow_Trade_Criteria crit){
	
		return (*this->cowgroups)[crit];
}

void Herd::putIntoRelevantGroup(Cow* cow){
	Cow::UnorderedSet* relevantGroup = this->getRelevantGroup(cow->getCowTradeCriteria());
	cow->Group = relevantGroup;
	relevantGroup->insert(cow);
}

inline void Herd::number_of_PI_increase(int n){
	this->number_of_PI += n;
}

inline void Herd::number_of_TI_increase(int n){
	this->number_of_TI += n;
}

inline void Herd::number_of_PI_decrease(int n){
	if(	this->number_of_PI >= n)
		this->number_of_PI -= n;
	else
		this->number_of_PI = 0;
}

inline void Herd::number_of_TI_decrease(int n){
	if(	this->number_of_TI >= n)
		this->number_of_TI -= n;
	else
		this->number_of_TI = 0;
}

Cow::UnorderedSet Herd::getNUnknownCows(int N){
	N = N > unknownCows.size() ? unknownCows.size() : N;

	Cow::UnorderedSet retSet;
	Cow::UnorderedSet::iterator it = unknownCows.begin();
	Cow* c;
	while(it != unknownCows.end()){
		c = *it;
		if(c->age() >= bvd_const::time_of_first_test.min)  // Ascertain that the picked cows will not be younger than
			retSet.insert(c);								// the minimum first test age...
		unknownCows.erase(it);  // ... otherwise erase that iterator and redefine it at the beginning of the unordered set
		it=unknownCows.begin();

		if(--N <= 0)
			break;
	}

	return retSet;
}

void Herd::addCowToUnknownList(Cow* cow){
	unknownCows.insert(cow);
}

void Herd::removeCowFromUnknownList(Cow* cow){
	Cow::UnorderedSet::iterator myIter = unknownCows.find(cow);
	if(myIter != unknownCows.end())
		unknownCows.erase(myIter);
}

void Herd::testAllCows(){
	for(auto cow:all_my_cows){
		System* s = this->farm->system;  // It is assumed that the tests for all the animals will take place within half a day
		s->schedule_event( new Event( s->getCurrentTime() + 0.5, Event_Type::VIRUSTEST, cow->id() ) );  // from the time
	}						  			// of the infected's identification through the jungtierCheck scheme
}
