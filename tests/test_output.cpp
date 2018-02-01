#include <ostream>
#include <array>
#include <vector>
#include <iostream>
#include "Output.h"
using namespace std;
//
// int main(int argc, char** argv)
// {
//   int N = 7;
//   int T = 8;
//   Output o = Output("test.h5",true );
//   int i,j;
//   std::array< int , 4 >* temp;
//   o.set_number_of_farms(N);
//   for (i=0 ; i< T ; i++)
//     {
//       Output::Datapoint* state = new Output::Datapoint();
//       for ( j=0 ; j<N ; j++)
// 	{
// 	  temp = new std::array< int , 4 >();
// 	  *temp = { i*1000 + j*10 + 1,
// 		    i*1000 + j*10 + 2,
// 		    i*1000 + j*10 + 3,
// 		    i*1000 + j*10 + 4};
// 	  state->push_back( temp );
// 	}
//       o.log_datapoint( i, state );
//       for ( auto t : *state )
// 	delete t;
//       delete state;
//     }
//   o.write_to_file();
//   return 1;
// }
