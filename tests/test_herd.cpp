#include "Herd.h"
#include "Cow.h"
#include "Events.h"
//
// void print_cow_set( const Cow_Set& s )
// {
//   for ( auto bella : s )
//     {
//       cout << "Cow ID: " << bella.id << ", birth_time: " << bella.birth_time << ", Calf State: ";
//       switch (bella.calf_status)
// 	{
// 	case NO_CALF : cout << "NO_CALF";
// 	  break;
// 	case SUSCEPTIBLE : cout << "SUSCEPTIBLE";
// 	  break;
// 	case IMMUNE : cout <<"IMMUNE";
// 	  break;
// 	case PERSISTENTLY_INFECTED : cout << "PERSISTENTLY_INFECTED";
// 	  break;
// 	case CRIPPLE : cout << "CRIPPLE";
// 	  break;
// 	case ABORT : cout << "ABORT";
// 	  break;
// 	}
//       cout << ", Calving number: " << bella.calving_number << ", last conception time: " << bella.last_conception_time<<endl;
//     }
// }
//
// void print_herd_info( const Herd& h )
// {
//   cout << "** R cows:"<<endl;
//   print_cow_set( h.R_cows );
//
//   cout << "** S cows:"<<endl;
//   print_cow_set( h.S_cows );
//
//   cout << "** TI cows:"<<endl;
//   print_cow_set( h.TI_cows );
//
//   cout << "** PI cows:"<<endl;
//   print_cow_set( h.PI_cows );
//   cout<<endl;
// }
//
//
// int main()
// {
//   Herd h;
//   Cow* Berta = new Cow(0);
//   h.R_cows.insert( *Berta );
//   h.R_cows.insert(  Cow(  0 ) );
//   h.S_cows.insert( Cow(  0 ) );
//   h.S_cows.insert( Cow(  0 ) );
//   h.TI_cows.insert( Cow( 0 ) );
//   h.TI_cows.insert( Cow( 0 ) );
//   h.PI_cows.insert( Cow( 0 ) );
//   h.PI_cows.insert( Cow( 0 ) );
//
//   h.future_events.schedule_event(  Event( 1   , END_OF_MA , COW , Berta ) );
//   h.future_events.schedule_event(  Event( 10.1 , INFECTION , COW , Berta ) );
//   h.future_events.schedule_event(  Event( 4.2 , BIRTH     , COW , Berta ) );
//
//   while ( !h.future_events.empty() )
//   {
//       print_herd_info( h );
//       h.test_execute_next_event();
//   }
//     print_herd_info( h );
//     cout << Berta->calving_number <<endl;
//     /*
//       h.test_execute_next_event();
//       print_herd_info( h );
//       h.test_execute_next_event();
//       print_herd_info( h );
//       h.test_execute_next_event();
//       print_herd_info( h );*/
// }
