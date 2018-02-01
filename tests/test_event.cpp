#include "Events.h"
#include <iostream>
using namespace std;

// void print_info( Event_queue& q )
// {
//   Event* e = q.top();
//   cout << "Next execution time: " << e->execution_time << ", type: " << (int)e->type << ". Is irc? " << e->is_infection_rate_changing_event() << endl;
//   cout << q.size() << " Events remaining." << endl;
// }
//
// int main(int argc, char** argv)
// {
//
//   Event_queue q ;
//
//
//   q.push( new Event(  9  , Event_Type::BIRTH      , 5  ) );
//   print_info( q );
//   q.push( new Event(  10 , Event_Type::END_OF_MA  , 5  ) );
//   print_info( q );
//   q.push( new Event(  1  , Event_Type::BIRTH      , 5  ) );
//   print_info( q );
//   q.push( new Event(  3  , Event_Type::END_OF_MA  , 5  ) );
//   print_info( q );
//   q.push( new Event(  2  , Event_Type::END_OF_MA  , 5  ) );
//   print_info( q );
//   q.push( new Event(  2  , Event_Type::INSEMINATION , 5  ) );
//   print_info( q );
//   q.push( new Event(  4  , Event_Type::CONCEPTION , 5  ) );
//   q.push( new Event(  5  , Event_Type::END_OF_MA  , 5  ) );
//   q.push( new Event(  6  , Event_Type::BIRTH      , 5  ) );
//   q.push( new Event(  7  , Event_Type::END_OF_MA  , 5  ) );
//   q.push( new Event(  8  , Event_Type::BIRTH      , 5  ) );
//   q.push( new System_Event(  3.1  , Event_Type::LOG_OUTPUT       ) );
//
//   cout << endl ;
//
//
//   while( !q.empty() )
//     {
//       print_info(q);
//       delete q.top();
//       q.pop();
//     }
// }
