#include "System.h"
#include "Cow.h"
#include "Events.h"
#include "Farm.h"
#include "Simple_One_Herd_Farm.h"
#include "Utilities.h"
#include <iostream>

//
// void print_firce( Cow* c )
// {
//   for (auto e : c->future_irc_events_that_move )
//     Utilities::pretty_print( e , std::cout );
// }
//
// int main ()
// {
//   System *s = new System(0);
//   s->set_log_interval(0.5);
//   Cow::set_system(s);
//
//   Farm*   f1 = new Simple_One_Herd_Farm(s);
//   Farm*   f2 = new Simple_One_Herd_Farm(s);
//
//   Cow* c0 = new Cow( 0 , NULL);
//   Cow* c1 = new Cow( 0 , NULL); c1->infection_status = Infection_Status::IMMUNE;
//   Cow* c2 = new Cow( 0 , NULL);
//   Cow* c3 = new Cow( 0 , NULL);
//   Cow* c4 = new Cow( 0 , NULL);
//   Cow* c5 = new Cow( 0 , NULL);
//   Cow* c6 = new Cow( 0 , NULL);
//
//   f1->push_cow( c0 );
//   f1->push_cow( c1 );
//   f1->push_cow( c2 );
//   f1->push_cow( c3 );
//
//   f2->push_cow( c4 );
//   f2->push_cow( c5 );
//   f2->push_cow( c6 );
//
//   s->schedule_event( new Event(  1  , Event_Type::BIRTH      , 5  ) );
//   s->schedule_event( new Event(  2  , Event_Type::END_OF_MA  , 1  ) );
//   s->schedule_event( new Event(  3  , Event_Type::INFECTION  , 5  ) );
//   s->schedule_event( new Event(  4  , Event_Type::INSEMINATION , 5  ) );
//
//   s->schedule_event( new Trade_Event(  4.2  , 0, f2  ) );
//
//   s->schedule_event( new Event(  5  , Event_Type::CONCEPTION , 0  ) );
//   s->schedule_event( new Event(  6  , Event_Type::END_OF_MA  , 0  ) );
//   s->schedule_event( new Event(  7  , Event_Type::BIRTH      , 0  ) );
//   s->schedule_event( new Event(  8  , Event_Type::END_OF_MA  , 5  ) );
//   s->schedule_event( new Event(  12  , Event_Type::DEATH      , 1  ) );
//
//
//   s->run_until( 4 );
//   s->schedule_event( new Event(  4.3  , Event_Type::INFECTION  , 0  ) );
//   //f1->print_state(std::cout);
//   //f2->print_state(std::cout);
//
//   s->execute_next_event();
//
//   //f1->print_state(std::cout);
//   //f2->print_state(std::cout);
//
//   s->execute_next_event();
//   s->run_until( 6 );
//
//   //  s->dump_queue();
//
//   //f1->print_state(std::cout);
//   //f2->print_state(std::cout);
//
//    s->schedule_event( new Event(  4.5, Event_Type::INFECTION  , 1  ) );
//    //s->schedule_event( new Event(  4.6, Event_Type::INFECTION  , 4  ) );
//
//
//   s->dump_queue();
//
//   //  s->execute_next_event()
//
//
//   //std::cout << std::endl;
//   //print_firce( c0 );
//
// }
