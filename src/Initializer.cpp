#include "Initializer.h"
#include "Farm.h"
#include "Slaughterhouse.h"
#include "Simple_One_Herd_Farm.h"
#include "Events.h"
#include "Cow.h"
#include "BVD_Random_Number_Generator.h"
#include "Model_Constants.h"
#include "System.h"
#include "projectImports/inih/cpp/INIReader.h"
#include <iostream>
#include "CowWellFarm.h"
#include "CSV_Reader.h"
#include "Small_One_Herd_Farm.h"
#include <iostream>
#include <fstream>

std::string Initializer::noinifilestring = "NONE";

/*const InitialFarmData Initializer::previouslyInfected = {0.02, 0.46, 0.06, 0.46 }; //(P, R, T, S)
const InitialFarmData Initializer::clean = {0.00, 0.205, 0.005, 0.79 }; //(P, R, T, S)
const std::map< FarmInitialConditionsType ,const InitialFarmData > Initializer::InitialFarmConditionToFarmData =
{
	{FarmInitialConditionsType::clean, Initializer::clean},
	{FarmInitialConditionsType::previouslyInfected, Initializer::previouslyInfected}
};*/
Initializer::Initializer(INIReader* inireader)
{
	this->reader = inireader;
  set_default_age_distribution( );
  set_default_farm_size_distribution( ); 
  
  std::string  type = reader->Get("system", "type", "rndFarms");
	this->smallFarmMax = reader->GetInteger("input", "smallFarmSizeMax", 0);
  if(type.compare("rndFarms") == 0){
	  this->simType = rndFarms;
  }else if(type.compare("inputFarmFile") == 0){
	  this->simType = inputFarmFile;
  }else{
	  std::cerr << "Unknown simulation type. Aborting" << std::endl;
	  exit(18);
  }
    double previnfS = reader->GetReal("modelparam", "previnfS", 0.46);
    double previnfT = reader->GetReal("modelparam", "previnfT", 0.06);
    double previnfR = reader->GetReal("modelparam", "previnfR", 0.46);
    double previnfP = reader->GetReal("modelparam", "previnfP", 0.02);

    double cleanS = reader->GetReal("modelparam", "cleanS", 0.79);
    double cleanT = reader->GetReal("modelparam", "cleanT", 0.005);
    double cleanR = reader->GetReal("modelparam", "cleanR", 0.205);
    double cleanP = reader->GetReal("modelparam", "cleanP", 0.00);

    set_STRP(previnfS, previnfT, previnfR, previnfP,
             cleanS, cleanT, cleanR, cleanP);

	double percPI = reader->GetReal("modelparam", "populationPercentagePI", 0.02);
	double percTI = reader->GetReal("modelparam", "populationPercentageTI", 0.02);
	int farmNum = reader->GetInteger("modelparam", "numberOfFarms", 0);
	int wellNum = reader->GetInteger("modelparam", "numberOfWells", 0);

	int minAge = reader->GetInteger("modelparam", "age_dist_min", 0);
	int maxAge = reader->GetInteger("modelparam", "age_dist_max", 3000);
	int modAge = reader->GetInteger("modelparam", "age_dist_mod", 200);
	set_default_age_distribution(minAge, maxAge, modAge);
	
	int minFarmSize = reader->GetInteger("modelparam", "farmsize_min", 10);
	int maxFarmSize = reader->GetInteger("modelparam", "farmsize_max", 500);
	set_default_farm_size_distribution(minFarmSize, maxFarmSize);
	
	percentageOfPreviouslyInfected = reader->GetReal("modelparam", "previouslyInfectedPercentageOfFarms", 0.02);
	
	int slaughterHouseNum = reader->GetInteger("modelparam", "numberOfSlaughterHouses", 0);
	if(percPI + percTI > 1.0){
		std::cerr << "More than 100% of the population are supposed to suffer from BVD...aborting." << std::endl;
		exit(21);
		
	}
	std::string inifilename = reader->Get("modelparam", "inifileName", Initializer::noinifilestring);
	

        
    set_number_of_slaughterhouses(slaughterHouseNum);
    set_number_of_wells(wellNum);

    if(inifilename.compare(Initializer::noinifilestring) != 0){
		CSVTable<int> table = CSVReader<int>::readCSVFile(inifilename, true, ';');
		simType = inputFarmFile;
		int total_number_of_farms = 0;
		int farmNumber, cowNum;
		if(table.getNumCols() < 2){
			std::cerr << "csv file does not contain enoguh columns" << std::endl;
			std::cout << inifilename << std::endl;
			exit(15);
		}
		for(int i=0; i < table.getNumRows() ; i++){
			cowNum = table[0][i];
			farmNumber = table[1][i];
			if(cowNum <= minFarmSize || cowNum > maxFarmSize) continue; 
			total_number_of_farms += farmNumber;

				
		}
		this->set_number_of_farms(total_number_of_farms);

		total_number_of_farms = 0;
		
		for(int i=0; i < table.getNumRows() ; i++){
			cowNum = table[0][i];
			farmNumber = table[1][i];
			if(cowNum <= minFarmSize || cowNum > maxFarmSize) continue; 
			for(int j=0; j < farmNumber; j++){
				total_number_of_farms++;
				this->set_number_of_animals_in_farm(total_number_of_farms-1, cowNum);
				
			}
		}
		farmNum = total_number_of_farms;
		
	}else{
		set_number_of_farms(farmNum);

	}
	#ifdef _INITIALIZER_DEBUG_
		std::cout <<"setting number of farms to " << farmNum << ", number of slaughterhouses to " <<  slaughterHouseNum << " and number of Wells to " << wellNum << std::endl;
	#endif
	std::cout << "created initializer for " << farmNum << " farms." << std::endl;
}

Initializer::~Initializer()
{

}


void Initializer::set_STRP(double previnfS, double previnfT, double previnfR, double previnfP,
                           double cleanS, double cleanT, double cleanR, double cleanP)
{
    previouslyInfected = {previnfP, previnfR, previnfT, previnfS}; //This order is given due to the way the compartments are pushed in the vector container
    clean = {cleanP, cleanR, cleanT, cleanS};
    InitialFarmConditionToFarmData =
    {
            {FarmInitialConditionsType::clean, Initializer::clean},
            {FarmInitialConditionsType::previouslyInfected, Initializer::previouslyInfected}
    };
/*    std::cout << "previnfS: " << previnfS << ", previnfT: " << previnfT << ", previnfR: " << previnfR << ", previnfP: " << previnfP
              << ", cleanS: " << cleanS << ", cleanT: " << cleanT << ", cleanR: " << cleanR << ", cleanP: " << cleanP << std::endl;*/
}

void Initializer::set_default_age_distribution( double min , double max , double mod )
{
  def_age_distr.min = min;
  def_age_distr.max = max;
  def_age_distr.mod = mod;  
}


void Initializer::set_default_farm_size_distribution(  int min, int max  )
{
  def_farm_size_distr.min = min;
  def_farm_size_distr.max = max;
  
}

void Initializer::set_number_of_farms( int N )
{
	System *s = System::getInstance(NULL);
  if ( N < 1 )
    N=1;
  number_of_farms = N;
	std::cout << "pct previously infected is " << this->percentageOfPreviouslyInfected << std::endl;
  
  for (int i=0 ; i < N ; i++ )
    {
      age_distr.push_back( def_age_distr );
      no_animal.push_back( -1 );
      if(s->rng.ran_unif_double( 1.0, 0.0) <= this->percentageOfPreviouslyInfected){
	      initialTypes.push_back(FarmInitialConditionsType::previouslyInfected);
      }else{
	      initialTypes.push_back(FarmInitialConditionsType::clean);
      }

    }
}
void Initializer::set_number_of_slaughterhouses( int N ){
	if ( N < 0 )
    	N=0;
  	number_of_slaughterhouses = N;
}
void Initializer::set_number_of_wells( int N ){
	if ( N < 0 )
    	N=0;
  	number_of_wells = N;
}

void Initializer::set_number_of_animals_in_farm(    int farm_idx , int no_of_animals )
{
  if ( farm_idx >= number_of_farms )
    return;
  if( no_of_animals > 0 )
    no_animal.at( farm_idx ) = no_of_animals;
}





void Initializer::set_age_distribution_in_farm(     int farm_idx , double min , double max , double mod )
{
  if ( farm_idx >= number_of_farms )
    return;
  if( min > 0 && mod >= min && max >=mod )
    age_distr.at( farm_idx ) = {min,max,mod};
}



void Initializer::initialize_system( System* s )
{
  
  Farm* farm;
  Cow::set_system( s );
	

	for (int i=0 ; i<number_of_farms ; i++ ){
		if(this->smallFarmMax < no_animal.at(i)){
			farm = new Simple_One_Herd_Farm( s );
		}else{
			farm = new Small_One_Herd_Farm(s);
		}
		initialize_random_farm( farm , i );
		s->register_farm(farm);
	}
	
  for(int i=0; i < this->number_of_wells;i++){
	    farm = new CowWellFarm(s);
		s->register_farm(farm);
    }
  
    for(int i=0; i < this->number_of_slaughterhouses;i++){
	    farm = new Slaughterhouse(s);
	    this->initialize_slaughterhouse((Slaughterhouse*)farm);
	    s->register_farm(farm);
    }

	//For debugging purposes
#ifdef _FARM_INITIALIZER_DEBUG_
    std::ofstream outputfile;
    outputfile.open ("farms.csv");
    outputfile << "farmid;farmtype\n";
    for (Farm *f : s->getFarms() ) {
        switch (f->myType) {
            case SLAUGHTERHOUSE:
                outputfile << f->id << ";" << "SLAUGHTERHOUSE" << "\n";
                break;
            case WELL:
                outputfile << f->id << ";" << "WELL" << "\n";
                break;
            default:
                outputfile << f->id << ";" << "FARM" << "\n";
                break;
        }
    }
    outputfile.close();

	std::cout << "number of cows in the system after initialization: " <<  Cow::number_of_living_cows() << std::endl;
#endif
}



// private:
void Initializer::initialize_random_farm( Farm* farm , int farm_idx )
{
#ifdef _DEBUG_
	std::cout << "initializing farm" << std::endl;
#endif

	System* s = farm->system;
	int i;

	double time = 0.;
	int number = no_animal.at(farm_idx);
	double hi,lo;
	lo = log10((double)def_farm_size_distr.min);
	hi = log10((double)def_farm_size_distr.max);
	//(1) calculate number of animals
	if ( number <= 0  )
	{
		number = (int)pow(10,s->rng.ran_unif_double( hi , lo ));
	}
	//For each animal

	for( i=0 ; i<number ; i++ )
	{
		this->createCow(farm_idx, i, number, farm, time);
	}
	farm->holdSize();
#ifdef _DEBUG_
	std::cout << "finished setting up the farm" << std::endl;
#endif
}
void Initializer::initialize_farm_of_size(Farm* farm, int size){
	
}
void Initializer::initialize_slaughterhouse( Slaughterhouse* slh ){
	
}

Cow* Initializer::createCow(const int& farm_idx, int& i, const int& number, Farm* f, double& time, double age, Cow* mother){
	//(2) calculate age
	double mod,hi,lo;
	lo = age_distr.at(farm_idx).min;
	hi = age_distr.at(farm_idx).max;
	mod= age_distr.at(farm_idx).mod;
	System* s = System::getInstance(NULL);
	if(age <=0 )
		age = s->rng.ran_triangular_double( lo, hi, mod );

	//(3) create cow
	Cow* c = new Cow( (time-age) ,mother );

	//(4) calculate infection status
	FarmInitialConditionsType type = initialTypes[farm_idx];
	InitialFarmData dat = Initializer::InitialFarmConditionToFarmData.at(type);

	if(i < (int) (dat.PIs * ceil(number))){
		c->infection_status = Infection_Status::PERSISTENTLY_INFECTED;
	}else if(i < (int) (dat.TIs * ceil(number))){
		c->infection_status = Infection_Status::TRANSIENTLY_INFECTED;
	}else if(i < (int) (dat.Rs * ceil(number))){
		c->infection_status = Infection_Status::IMMUNE;
	}else{
		c->infection_status =  Infection_Status::SUSCEPTIBLE;
	}
	//(5) put animal into farm
	f->push_cow( c );
    //Future events in the system are scheduled only for female cows
    if (c->female) {
        this->scheduleFutureEventsForCow(c, f, farm_idx, i, number, time);
    }
	f->system->addCow(c);
	return c;
}

inline void Initializer::scheduleFutureEventsForCow(Cow* c, Farm* farm, const int& farm_idx, int& i,const int& number, double& time){
    Event_Type et;
    double t;
    System* s = System::getInstance(NULL);
    double insem_age = s->rng.first_insemination_age();
    double age = c->age();
    double timeForCalving = age - insem_age ;
    int calvings_so_far=0;
    if ( timeForCalving >= 0 )
    {
        calvings_so_far = (int) round((timeForCalving) / ( s->rng.duration_of_pregnancy() + s->rng.time_of_rest_after_calving(c->calving_number) )) ;
        c->calving_number -= calvings_so_far;
		if (c->calving_number > 0) {
			c->has_been_pregnant_at_all_so_far = true;
		}
        double timeOfLastInsemination = s->rng.staggering_first_inseminations();

        if(timeOfLastInsemination <= time){
            et = Event_Type::BIRTH;
            switch( c->infection_status )
            {
                case Infection_Status::TRANSIENTLY_INFECTED:
                    c->calf_status = s->rng.calf_outcome_from_infection ( 0 );
                    break;
                case Infection_Status::PERSISTENTLY_INFECTED:
                    c->calf_status = Calf_Status::PERSISTENTLY_INFECTED;          // p=1 for the birth of a PI calf by a PI mother.
                    break;
                default:
                    c->calf_status = Calf_Status::SUSCEPTIBLE; // Yes, SUSCEPTIBLE is right. An eventual immunity through MA is handled in the BIRTH routine.
//                    break;
            }
            t = timeOfLastInsemination + s->rng.duration_of_pregnancy();
        } else {
            et = Event_Type::INSEMINATION;
            t = timeOfLastInsemination;
        }
    }
    else
    {
        et = Event_Type::INSEMINATION;
        t  = insem_age - (age + time);
    }
    //(6) schedule the event.
    s->schedule_event( new Event( t , et , c->id() ) );
}