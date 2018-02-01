// #include <ostream>
// #include <iostream>
// #include "Simple_One_Herd_Farm.h"
// #include "System.h"
// #include "Herd.h"
// #include "Cow.h"
// #include "Events.h"
// using namespace std;
#include <iostream>
#include "projectImports/Catch/catch.hpp"
#include "projectImports/FakeIt/fakeit.hpp"
#include "Cow.h"
#include "System.h"
#include "projectImports/inih/cpp/INIReader.h"
#include "Farm.h"
#include "Simple_One_Herd_Farm.h"
#include "Events.h"
#include "BVD_Random_Number_Generator.h"
using namespace fakeit;
TEST_CASE("Cows can be created", "[Cow]"){
  //Set up a system with a single cow in a single farm
  INIReader reader("./iniFiles/unitTestCow.ini");
  System * s = System::getInstance(&reader);
  Farm* f = new Simple_One_Herd_Farm(s);
  f->holdSize();
  Cow::set_system(s);
  Cow *erna;
  //We mock the random number generator to make sure it returns predictable results
  Mock<Random_Number_Generator> fakeRNG((s->rng));
  //make sure every calf is female
  When(Method(fakeRNG, is_calf_female)).AlwaysReturn(true);
  //make sure every vaccination works
  When(Method(fakeRNG, vaccinationWorks)).AlwaysReturn(true);
  SECTION("Waiting for the end of MA before vaccinating for the first time"){
    //set up the mother just about to give birth to a calf
    erna = new Cow( 0.0 , nullptr, true );
    f->push_cow(erna);
    erna->calf_status = Calf_Status::SUSCEPTIBLE;
    //let the mother give birth to a calve at day 500
    s->run_until(499);
    Event *birth = new Event( 500 , Event_Type::BIRTH , erna->id() );
    erna->execute_event(birth);
    //check if the scheduled first vaccination takes place after the end of MA
    //born at day 500 erna's calve will get out of MA around day 680 so make sure
    //that the vaccination event scheduled is happening after that time
    Event_queue q = s->getEventQueue();
    while (!q.empty()){//iterate over all events in that queue
      Event* e = q.top();
      if (e->type == Event_Type::VACCINATE && e->id == 1){//check for the calf, it the first insemination is scheduled after the approximate end of its MA
        REQUIRE(e->execution_time > 680);
        // return;
      }
      q.pop();
    }

  }
  SECTION("A successful vaccination schedules a vaccination end event and a new vaccination and makes the cow IMMUNE"){

    //since System is a singleton, it will persist the state after the first section so we have to start a day later
    //schedule the vaccination
    Event *vacc = new Event( 500 , Event_Type::VACCINATE , erna->id() );
    s->schedule_event(vacc);
    // run until the vaccination is Done
    s->run_until(501);
    //make sure the cow is immune now
    REQUIRE(erna->infection_status == Infection_Status::IMMUNE);
    Event_queue q = s->getEventQueue();
    //keep track of other events in there
    int numberOfVaccinationEndEvents = 0;
    int numberOfVaccinationEvents = 0;
    while (!q.empty()){//iterate over all events in that queue
      Event* e = q.top();
      if (e->type == Event_Type::END_OF_VACCINATION && e->id == 2){//check if a vaccination end has been scheduled: the entry should be exactly one
        numberOfVaccinationEndEvents++;
      }
      if (e->type == Event_Type::VACCINATE && e->id == 2){//check if a new vaccination event has been scheduled
        numberOfVaccinationEvents++;
      }
      q.pop();
    }
    //naje sure there is exactly one of each
    REQUIRE(numberOfVaccinationEvents == 1);
    REQUIRE(numberOfVaccinationEndEvents == 1);

  }
  SECTION("If the vaccination failed a new vaccination get scheduled a year later"){}
  SECTION("On schedule of a new insemination: Vaccinate a cow 40 days before insemination if it hasn't been vaccinated before."){}
  SECTION("On schedule of a new insemination: If the cow has been vaccinated in the last 200 days, do nothing"){}
  SECTION("On schedule of a new insemination: If the cow has been vaccinated before the last 200 days, invalidate the old vaccination end event, the old next vaccination event and schedule a new vaccination 40 days ahead of insemination"){}
}
//
//
// void print_info( Cow::Set& v )
// {
//   for ( auto c : v )
//     {
//       cout << "id: " << c->id() << ", Birth time: " << c->birth_time  << " female: " << c->female << endl;
//       cout << "    Infection_status:" << (int)c->infection_status << ", last_conception_time: " << c->last_conception_time ;
//       cout << " calving_number: " << c-> calving_number << ", Calf_status: " << (int)c->calf_status << endl<<endl;
//     }
//   (*v.begin())->herd->print_info(cout);
//   cout << "--------------------------------------------------------------------------------"<<endl<<endl;
// }
//
//
//
// int main(int argc, char** argv)
// {
//
//   System *s = new System(0);
//   s->set_log_interval(0.5);
//   Cow::set_system(s);
//
//   Farm*   f = new Simple_One_Herd_Farm(s);
//
//   Cow *elsa  = new Cow( 0 , NULL );
//   Cow *Gerda = new Cow( 1 , elsa );
//   Cow *Trude = new Cow( 2 , elsa );
//
//
//
//   elsa->infection_status = Infection_Status::PERSISTENTLY_INFECTED;
//   elsa->calf_status      =      Calf_Status::SUSCEPTIBLE;
//   Gerda->infection_status = Infection_Status::IMMUNE;
//   Gerda->calf_status      =      Calf_Status::SUSCEPTIBLE;
//   Gerda->calving_number   = 2;
//
//   f->push_cow(elsa);
//   f->push_cow(Gerda);
//   f->push_cow(Trude);
//
//   Event *e = new Event ( 1 , Event_Type::END_OF_MA, 2 );
//   s->schedule_event( e );
//
//   e = new Event ( 10 , Event_Type::BIRTH , 1 );
//
//
//   s->schedule_event( e );
//
//   s->run_until( 10 );
//   exit(0);
//
//   f->print_state(cout);
//   s->print_next_event();
//
//   s->execute_next_event();
//
//   f->print_state(cout);
//   s->print_next_event();
//
//   s->execute_next_event();
//
//   f->print_state(cout);
//   s->print_next_event();
//
//   s->execute_next_event();
//
//   f->print_state(cout);
//   s->print_next_event();
//
//   s->execute_next_event();
//
//   f->print_state(cout);
//   s->print_next_event();
//
//   s->execute_next_event();
//
//   f->print_state(cout);
//   s->print_next_event();
//
//   s->execute_next_event();
//
//   f->print_state(cout);
//   s->print_next_event();
//
//   s->execute_next_event();
//
//   f->print_state(cout);
//
//   s->dump_queue();
// }
