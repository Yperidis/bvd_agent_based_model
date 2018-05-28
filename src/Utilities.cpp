#include "Utilities.h"
#include "Events.h"
#include <ostream>
#include <map>
#include <execinfo.h>

const std::map< Infection_Status , std::string > Utilities::IS_tostr =
  {{Infection_Status::SUSCEPTIBLE          ,"S  "},
   {Infection_Status::PERSISTENTLY_INFECTED,"PI "},
   {Infection_Status::TRANSIENTLY_INFECTED ,"TI "},
   {Infection_Status::IMMUNE               ,"R  "}};


const std::map<Calf_Status , std::string > Utilities::CS_tostr=
  {{Calf_Status::NO_CALF               , "NO_CALF"},
   {Calf_Status::SUSCEPTIBLE           , "S      "},
   {Calf_Status::PERSISTENTLY_INFECTED , "PI     "},
   {Calf_Status::IMMUNE                , "R      "},
   {Calf_Status::CRIPPLE               , "CRIPPLE"},
   {Calf_Status::ABORT                 , "ABORT  "}};

const std::map< Event_Type  , std::string > Utilities::Event_tostr =
  {{Event_Type::ABORTION    ,"ABORTION"},
   {Event_Type::INSEMINATION,"INSEMINATION"},
   {Event_Type::CONCEPTION  ,"CONCEPTION"},
   {Event_Type::BIRTH       ,"BIRTH"},
   {Event_Type::DEATH       ,"DEATH"},
   {Event_Type::END_OF_MA   ,"END_OF_MA"},
   {Event_Type::INFECTION   ,"INFECTION"},
   {Event_Type::RECOVERY    ,"RECOVERY"},
   {Event_Type::TRADE       ,"TRADE"},
   {Event_Type::REMOVECOW	  ,"REMOVECOW"}	,
   {Event_Type::SLAUGHTER	,"SLAUGHTER"},
   {Event_Type::CULLING		,"CULLING"},
   {Event_Type::VACCINATE	,"VACCINATE"},
   {Event_Type::END_OF_VACCINATION	,"END_OF_VACCINATION"},
   {Event_Type::LOG_OUTPUT  ,"LOG_OUTPUT"},
   {Event_Type::WRITE_OUTPUT,"WRITE_OUTPUT"},
   {Event_Type::STOP        ,"STOP"},
   {Event_Type::MANAGE		,"MANAGE"},
   {Event_Type::EARTAG		,"EARTAG"},
   {Event_Type::TEST		,"TEST"},
   {Event_Type::ANTIBODYTEST,"ANTIBODYTEST"},
   {Event_Type::VIRUSTEST	,"VIRUSTEST"},
   {Event_Type::VACCINATION	,"VACCINATION"},
   {Event_Type::QUARANTINEEND, "QUARANTINEEND"},
   {Event_Type::JUNGTIER      , "JUNGTIER"},
   {Event_Type::JUNGTIER_EXEC	,"JUNGTIER_EXEC"},
   {Event_Type::JUNGTIER_SMALL_GROUP, "JUNGTIER_SMALL_GROUP"},
   {Event_Type::ChangeContainmentStrategy, "ChangeContainmentStrategy"}
   };



void Utilities::pretty_print( Cow* c , std::ostream& out )
{
  out << "id: " << c->id() << ", Birth time: " << c->birth_time  << " female: " << c->female << std::endl;
  out << "    Infection_status:" << IS_tostr.at(c->infection_status) << ", last_conception_time: " << c->last_conception_time ;
  out << " calving_number: " << c-> calving_number << ", Calf_status: " << CS_tostr.at(c->calf_status)<<std::endl;
}

void Utilities::pretty_print( const Event* e , std::ostream& out)
{
  out << "Execution time: "<< e->execution_time << ", type: " << Event_tostr.at(e->type);
  out << ", id: " << e->id<<". Is irc? " << e->is_infection_rate_changing_event() << std::endl;  //False is 0, True is 1
}

void Utilities::printStackTrace(int number){

	char **strings;
	void *array[number];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, number);
	printf("backtrace() returned %d addresses\n", size);
	// print out all the frames to stderr
	strings = backtrace_symbols(array, size);
	if (strings == NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}

	for (int j = 0; j < size; j++)
	printf("%s\n", strings[j]);
  	//backtrace_symbols_fd(array, size, STDERR_FILENO);
  	//exit(1);

}
