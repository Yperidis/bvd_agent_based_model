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
#include "FarmManager.h"  //Needed for selling directly to the slaughterhouse
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
	this->smallFarmMax = reader->GetInteger("modelparam", "smallFarmSizeMax", 0);
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
	int farmNum = reader->GetInteger("modelparam", "numberOfFarms", 1);
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

    std::cout << "Farm size distribution path: " << inifilename << std::endl;
	

        
    set_number_of_slaughterhouses(slaughterHouseNum);
    set_number_of_wells(wellNum);

    if(inifilename.compare(Initializer::noinifilestring) != 0){
		CSVTable<int> table = CSVReader<int>::readCSVFile(inifilename, true, ';');
		simType = inputFarmFile;
		int total_number_of_farms = 0;
		int total_number_of_animals = 0;
		int farmNumber, cowNum;
		if(table.getNumCols() < 2){
			std::cerr << "csv file does not contain enough columns, the path, or the delimiter is wrong (has to be ;)" << std::endl;
			std::cout << inifilename << std::endl;
			exit(15);
		}
		///Storing the information from the given csv file in an array (see the table implementation in the CSVTable template)
		///and initialising the farms in terms of STRP distributions
		for(int i=0; i < table.getNumRows() ; i++){
			cowNum = table[0][i];     //Number of animals column
			farmNumber = table[1][i];    //Number of farms column
			if(cowNum <= minFarmSize || cowNum > maxFarmSize) continue;    //Conditional to filter out farms corresponding
            //to animals less or equal or greater than certain values. These values are input from the ini file.
			total_number_of_farms += farmNumber;
            for(int j=0; j < farmNumber; j++){
                No_farm_animals.push_back(cowNum);    //This vector keeps track of the number of cows corresponding to the number of farms.
                //Useful for allocating the farm types (clean or PI-infected) upon the setup of the farm number. Consistent
                //with the next loop. Has to be set before the set_number_of_farms function is called to be useful.
            }

		}
		//Here we set the total number of farms filtered from the ini file
		this->set_number_of_farms(total_number_of_farms);

		total_number_of_farms = 0;

		///Distributing the number of animals in each farm, which already contains instructions for the STRP distribution
		for(int i=0; i < table.getNumRows() ; i++){
			cowNum = table[0][i];    //Number of animals column
			farmNumber = table[1][i];    //Number of farms column
			if(cowNum <= minFarmSize || cowNum > maxFarmSize) continue;    //Conditional to filter out farms corresponding
            //to animals less or equal or greater than certain values. These values are input from the ini file.
			for(int j=0; j < farmNumber; j++){
				total_number_of_farms++;
				//Here we set the number of animals in every single farm as they have been already set up
				this->set_number_of_animals_in_farm(total_number_of_farms-1, cowNum);    //total_number_of_farms-1 counts
				//the farm ID
				
			}
/*			for(int k=0; k < cowNum; k++)
				total_number_of_animals++;*/
		}
		farmNum = total_number_of_farms;

        //This alternative leads to the default value of farms or those defined from the ini file.
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
    previouslyInfected = {previnfP, previnfR, previnfT, previnfS}; // This order is given due to the way the compartments are pushed in the vector container
    clean = {cleanP, cleanR, cleanT, cleanS};
    InitialFarmConditionToFarmData =
    {
            {FarmInitialConditionsType::clean, Initializer::clean},
            {FarmInitialConditionsType::previouslyInfected, Initializer::previouslyInfected}
    };
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
	System *s = System::getInstance(nullptr);
  if ( N < 1 )     //Ensure that the system is initialised with at least one farm
    N=1;
  number_of_farms = N;
	std::cout << "pct of previously PI infected farms is " << this->percentageOfPreviouslyInfected << std::endl;

  for (int i=0 ; i < N ; i++ )
    {
      age_distr.push_back( def_age_distr );    // Initialise a vector with the default age distribution for every farm created
      no_animal.push_back( -1 );    // Initialise a vector for the animal number with an initial entry for every farm created
      //if(No_farm_animals.at(i) > 100){    // For assigning PI infected farms according to an animal count criterion
      if(s->rng.ran_unif_double( 1.0, 0.0 ) <= this->percentageOfPreviouslyInfected){  // Assigning randomly (from a uniform
          // rnd distribution) PI-infected and clean farms, according to a threshold defined in the ini file
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

//TODO Make the age distribution function have an effect on the initialisation
void Initializer::set_age_distribution_in_farm( int farm_idx , double min , double max , double mod )
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
		if(this->smallFarmMax < no_animal.at(i)){    // Initialize simple or small (no annual replacement requirement) herd farm
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

	// For debugging purposes
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
            case SMALL_FARM:
                outputfile << f->id << ";" << "SMALL FARM" << "\n";
                break;
			default:
				outputfile << f->id << ";" << "SIMPLE FARM" << "\n";
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

	double time = 0.;    // Start of time for a random farm
	int number = no_animal.at(farm_idx);
	double hi,lo;
	lo = log10((double)def_farm_size_distr.min);
	hi = log10((double)def_farm_size_distr.max);
	//(1) calculate number of animals for each farm
	if ( number <= 0  )
	{
		number = (int)pow(10,s->rng.ran_unif_double( hi , lo ));
	}


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
	//TODO Connect the ini reader with the desired age distribution
	lo = age_distr.at(farm_idx).min;
	hi = age_distr.at(farm_idx).max;
	mod= age_distr.at(farm_idx).mod;
	System* s = System::getInstance(nullptr);
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
    //Future events in the system other than slaughter are scheduled only for female cows. See scheduleFutureEventsForCow()
    this->scheduleFutureEventsForCow(c, f, farm_idx, i, number, time);
	f->system->addCow(c);

/*	if(c->id() == 203458)
	    std::cout << (int) c->infection_status << std::endl;*/

	return c;
}
// TODO For the initialised cows an ear tag test is never scheduled as it is only triggered by birth
inline void Initializer::scheduleFutureEventsForCow(Cow* c, Farm* farm, const int& farm_idx, int& i,const int& number, double& time){
    Event_Type et;
    double t;
    System* s = System::getInstance(nullptr);

	if (c->female){  //Block only for female cows

		double insem_age = s->rng.first_insemination_age();     // insem_age belongs in [0, max(first_insemination_age() ) )
		double age = c->age();
		double timeForCalving = age - insem_age;
		int calvings_so_far=0;
		if ( timeForCalving >= 0 )    // if the cow is either of age to be inseminated, pregnant or about to give birth
		{
		    // If the age of the cow is greater than its first insemination age we count the units of pregnancies
            // and rest times until the next insemination which fit in the difference of the animal's age and
            // first insemination age as its number of calvings upon initialisation, i.e. its an age dependent definition
            // of the calvings.
			calvings_so_far = (int) round( (timeForCalving) / ( s->rng.duration_of_pregnancy() +
															   s->rng.time_of_rest_after_calving(c->calving_number) ) );
			c->calving_number -= calvings_so_far;    // alter the calving counter according to the age over the pregnancy
			// and the rest duration rounded
			if (c->calving_number > 0) {
				c->has_been_pregnant_at_all_so_far = true;
			}
			double timeOfLastInsemination = s->rng.staggering_first_inseminations();    // Select in a uniformly random
            // fashion the time of insemination between 0 and the minimum pregnancy duration

			if(timeOfLastInsemination == time){    // if it is already carrying at t=0, schedule its labour...
				et = Event_Type::BIRTH;
				switch( c->infection_status )
				{
					case Infection_Status::TRANSIENTLY_INFECTED:
						c->calf_status = s->rng.calf_outcome_from_infection ( 0 );  // At t=0 since we are initialising
						break;
					case Infection_Status::PERSISTENTLY_INFECTED:
						c->calf_status = Calf_Status::PERSISTENTLY_INFECTED;  // p=1 for the birth of a PI calf by a PI mother.
						break;
					default:
						c->calf_status = Calf_Status::SUSCEPTIBLE;  // Yes, SUSCEPTIBLE is right. An eventual immunity through MA is handled in the BIRTH routine.
				}
				t = timeOfLastInsemination + s->rng.duration_of_pregnancy();    // ...right after the duration of its pregnancy
			}
			else{
				et = Event_Type::INSEMINATION;    // otherwise schedule its insemination somewhere in [0, min(duration of pregnancy) )
				t = timeOfLastInsemination;
/*                if(c->id() == 203458)
                    std::cout << "Insemination for t=" << t << std::endl;*/
				if(timeOfLastInsemination < time)
					std::cerr << "INSEMINATION TAKING PLACE BEFORE t=0. DEBUG ME!" << std::endl;
			}
		}
		else
		{    // if the cow is below its insemination age, schedule its insemination somewhere in time between its age
			// of insemination and the min(duration of pregnancy)
			et = Event_Type::INSEMINATION;  // Vaccination is achieved through insemination
			t  = insem_age - age + s->rng.staggering_first_inseminations();
/*            if(c->id() == 203458)
                std::cout << "Insemination for t=" << t << std::endl;*/
		}
		//(6) schedule the event.
		s->schedule_event( new Event( t , et , c->id() ) );
	}
	/// Send all the initialised male animals to the slaughterhouse, each one within a uniformly distributed random
	/// time drawn from their life expectancy.
	else{
	    et = Event_Type::SLAUGHTER;
	    t = time + s->rng.life_expectancy_male_cow();
        s->schedule_event( new Event( t , et , c->id() ) );
	}
}