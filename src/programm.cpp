#include "Initializer.h"
#include "System.h"
//#include "Output.h"
#include <iostream>
#include "BVD_Random_Number_Generator.h"
#include <chrono>
#include "AdvancedOutput.h"
#include "BVDOptions.h"
#include "projectImports/inih/cpp/INIReader.h"







#pragma mark - main
/* actual program code */
int main(int argnum, char *arguments[], char *environment[])
{
	BVDOptions opt = BVDOptions();
	opt.handleCommandLineArguments(argnum, arguments);
	
  	if(BVDOptions::iniFilePath.compare("NONE") == 0){
	  	std::cerr << "Supply path to ini-file in order to start the simulation or use --help to get information on the proper use of this program." << std::endl;
	  	exit(1);
  	}
  	INIReader reader(BVDOptions::iniFilePath);
  	if (reader.ParseError() < 0) {
        std::cout << "Can't load ini file supposedly located at " << BVDOptions::iniFilePath << std::endl;
        exit(2);
    }
	double t_end   = reader.GetReal("simulation", "t_end", 1000.0);
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
	System * s = System::getInstance(&reader);


	Initializer init = *new Initializer(&reader);
	#ifdef _DEBUG_
	std::cout << "set init stuff" << std::endl;
	#endif
	std::cout << "Initializing (blurb)..." << std::endl;
	
	//init.set_number_of_farms( 3 );
	//init.set_number_of_slaughterhouses( 2 );
	//init.set_number_of_PI_animals_in_farm( 20, 0); // No PI in the system should result in a very different trajectory..
	//init.set_age_distribution_in_farm(     0, 4 , 2000, 800 );
	#ifdef _DEBUG_
		std::cout << "initialize system" << std::endl;
	#endif
	init.initialize_system ( s );

	// Step 3: Log initial state
	s->log_state();
	
	//s->output->write_to_file();
	if(reader.GetBoolean("modelparam", "inputCows", false)){
			s->scheduleFutureCowIntros();
	}
	
	std::cout << "Beginning simulation" << std::endl;
	s->run_until(t_end);
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::minutes>( t2 - t1 ).count();
	std::cout << "The simulation took " << duration << " minutes" << std::endl;

	//Step 5: The system s now contains all cows, farms, events etc. as they result from the run.
	// Output has been written to file as desired and can be used.
	// Tests on the system state, further runs continuing from this state are possible.
	
	// However, we quit here, by explicitly deleting s, which prints one line of total stats (how many events and cows have been processed.)
	//delete s;
}
