#ifndef __initializer_h_
#define __initializer_h_
#include <vector>
#include <cmath>
#include <string>
#include <map>
class System;
class Farm;
class Slaughterhouse;
class Cow;
class INIReader;

enum simulationType{
	rndFarms,
	inputFarmFile
};

enum class FarmInitialConditionsType{
	clean = 1,
	previouslyInfected =2
};

struct InitialFarmData{
	double PIs;
	double Rs;
	double TIs;
	double S;	
};


class Initializer
{
 public:
  Initializer(INIReader* inireader);
  ~Initializer();
  void set_default_age_distribution(     double min=1 , double max=1800 , double mod=720 );
  void set_default_farm_size_distribution(  int min=100,    int max=1000 );
  void set_default_percentage_of_PI_animals_in_farm( double no_of_PI=0.0 );
  void set_default_percentage_of_TI_animals_in_farm( double no_of_PI=0.0 );

    void set_STRP(double previnfS, double previnfT, double previnfR, double previnfP,
                  double cleanS, double cleanT, double cleanR, double cleanP);

  	void set_number_of_farms( int N );
  	void set_number_of_slaughterhouses( int N );
  	void set_number_of_wells( int N );
  void set_number_of_animals_in_farm(    int farm_idx , int no_of_animals );

  void set_age_distribution_in_farm(     int farm_idx , double min , double max , double mod );

  void initialize_system( System* s );

 private:
  void initialize_random_farm( Farm* farm , int farm_idx );
  void initialize_farm_of_size(Farm* farm, int size);
  void initialize_slaughterhouse( Slaughterhouse* slh );
  Cow *createCow(const int& farm_idx,  int& i, const int& number, Farm* f, double& time , double age = -1., Cow* mother = NULL);
  void scheduleFutureEventsForCow(Cow* c, Farm* farm, const int& farm_idx,  int& i, const int& number, double& time);
  struct d_tri_distr { double min; double max; double mod; } ;
  struct i_tri_distr { int min; int max; int  mod; } ;
  d_tri_distr def_age_distr;
  i_tri_distr def_farm_size_distr;


  int number_of_farms;
  int number_of_slaughterhouses;
  int number_of_wells;
  int smallFarmMax;
  INIReader* reader;
  std::vector< d_tri_distr > age_distr;
  std::vector< int >         no_animal;
  simulationType simType;
	std::vector <FarmInitialConditionsType> initialTypes;
	std::map< FarmInitialConditionsType , InitialFarmData > InitialFarmConditionToFarmData;
	
	double percentageOfPreviouslyInfected;
  static std::string noinifilestring;
  InitialFarmData previouslyInfected;
  InitialFarmData clean;
};
#endif
/*
The Initializer is to be used as follows:
1) Create an instance of it:
Initializer init;

2) Set all the default values as desired, using the functions set_default_xxx
If none of these functions is used, default values will be the ones appearing in the signature.

init.set_default_number_of_PI_animals_in_farm( 1 );
void set_default_age_distribution(     10 , 2000 , 720 );

3) Set the number of farms to be initialized

init.set_number_of_farms( 10 );

4) If you want to have a non-default age distribution or number of animals or number of PI animals in some of the farms, use the set_xxx_in_farm functions for this.

init.set_number_of_animals_in_farm( 2 , 40 )
( sets the number of animals in thefarm with index 2 to 40.)

5) use initialize_system with a pointer to System to initialize everything.

The desired number of farms will be generated, filled with a certain number of animals according to the farm-size-distribution. (uniform distribution in log(N)). 
The age of the cows will be according to the age distribution for this farm (if not set explicitly -> default age distribution)
In each farm there will be a certain number of PI animals (either default value or custom value for this farm.)
For every cow an event will be an scheduled pertaining to the breeding dynamics.
 */
