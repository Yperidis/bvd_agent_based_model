#include "BVD_Random_Number_Generator.h"
#include "Model_Constants.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <limits>
#include <iostream>
#include "System.h"
#include "BVDContainmentStrategy.h"
Random_Number_Generator::Random_Number_Generator() //Constructor with random seed
{

 seed = (time(NULL)  + rand());    //set a random seed
 init(seed);

}

Random_Number_Generator::Random_Number_Generator( unsigned int seed ):seed(seed) //Constructor with fixed seed
{

  init(seed);
}
void Random_Number_Generator::init( unsigned int seed){
	gsl_rng_env_setup();
  const gsl_rng_type * T = gsl_rng_default;
  generator = gsl_rng_alloc (T);
  gsl_rng_set (generator,seed);

}
int Random_Number_Generator::ran_unif_int( int hi, int lo)
{
  if ( hi < lo )
    {
      std::cerr << "ran_unif_int called with hi < lo.. Aborting" << std::endl;
      exit(1);
    }
  return lo + gsl_rng_uniform_int(generator, hi-lo);
}
double Random_Number_Generator::ran_unif_double( double hi, double lo)
{
  if ( hi < lo )
    {
      std::cerr << "ran_unif_double called with hi < lo.. Aborting" << std::endl;
      exit(1);
    }
  return lo + (hi-lo) * gsl_rng_uniform(generator);
}

double Random_Number_Generator::ran_triangular_double(double lo,double hi,double mod)
{
  if (!( (lo <= mod) && (mod<= hi) && (lo < hi) ) )
    {
      std::cerr << " ran_triangular called with lo="<<lo<<", hi="<<hi<<", mod="<<mod;
      std::cerr << " -> lo <= mod <= hi && lo < hi evaluates to false.. Aborting"<<std::endl;
      exit(1);
    }
  double y = ran_unif_double(1,0);                                     //random double between 0-1
  double ymod = (mod - lo) / (hi - lo);                                //F⁻1(mod)=ymod
  double inverse_lo = lo + sqrt(y * (hi - lo) * (mod - lo) );            //inverse distribution function for 0<y<F
  double inverse_hi = hi - sqrt((1 - y) * (hi - lo) * (hi - mod) );      //inverse distribution function for F<y<1
  if (y <= ymod)
    return inverse_lo;
  else
    return inverse_hi;
}

int Random_Number_Generator::ran_triangular_int(double lo,double hi,double mod)
{
    int triangular_int = (int) (ran_triangular_double(lo,hi,mod)+0.5);
    return triangular_int;

  /*double y = ran_unif_double(1,0);                                       //random double zwischen 0-1
   double ymod = (mod - lo) / (hi - lo);                                  //F⁻1(mod)=ymod
   double inverse_lo = lo + sqrt(y * (hi - lo) * (mod - lo))+0.5;         //inverse Verteilungsfunktion für 0<y<F
   double inverse_hi = hi - sqrt((1 - y) * (hi - lo) * (hi - mod))+0.5;   //inverse Verteilungsfunktion für F<y<1
   int result_a = (int)inverse_lo;
   int result_b = (int)inverse_hi;
   if (y <= ymod)
      return result_a;
   else
      return result_b;*/
}

/*int Random_number_Generator::ran_poisson_int(int r, double mu)
{
    return gsl_ran_poisson(int r, double mu);
// }*/







bool Random_Number_Generator::will_TI_calf_die()
{
  int index = ran_unif_int(100, 0);
  if (index < bvd_const::lethality_TI_calf) {
      return true;
  } else {
      return false;
  }
}

Calf_Status Random_Number_Generator::calf_outcome_from_infection( double time_of_pregnancy )
{
  int index = ran_unif_int(100,0);
  if (time_of_pregnancy <= bvd_const::period_of_pregnancy.first)
  {
    if (index < bvd_const::first_period_of_pregnancy.PI)
      return Calf_Status::PERSISTENTLY_INFECTED;
    else
      return Calf_Status::ABORT;
  }

  else if (time_of_pregnancy <= bvd_const::period_of_pregnancy.second)
  {
    if      (index < bvd_const::second_period_of_pregnancy.PI)
      return Calf_Status::PERSISTENTLY_INFECTED;
    else if (index <  bvd_const::second_period_of_pregnancy.abort)
      return Calf_Status::ABORT;
    else if (index < bvd_const::second_period_of_pregnancy.cripple)
      return Calf_Status::CRIPPLE;
    else
      return Calf_Status::IMMUNE;
  }
  else if (time_of_pregnancy < bvd_const::period_of_pregnancy.third)
  {
    if      (index < bvd_const::third_period_of_pregnancy.abort)
      return Calf_Status::ABORT;
    else if (index <  bvd_const::third_period_of_pregnancy.cripple)
      return Calf_Status::CRIPPLE;
    else
      return Calf_Status::IMMUNE;
  }
  else
  {
    if      (index < bvd_const::fourth_period_of_pregnancy.abort)
      return Calf_Status::ABORT;
    else if (index <  bvd_const::fourth_period_of_pregnancy.cripple)
      return Calf_Status::CRIPPLE;
    else
      return Calf_Status::IMMUNE;
  }
}

double Random_Number_Generator::time_of_abortion_due_to_infection( double time_of_pregnancy )
{
  return ran_unif_double(bvd_const::time_till_abortion_takes_place,0);
  /*  {
  int index = ran_unif_int(100,0);
  if (time_of_pregnancy <=70)
  {
    if (index <=10)
      return time_of_pregnancy+ ran_unif_double(14,0);
  }
  else if (time_of_pregnancy <=120)
  {
    if (index <=15)
      return time_of_pregnancy+ran_unif_double(14,0);
  }
  else if (time_of_pregnancy <=180)
  {
    if (index <=20)
      return time_of_pregnancy+ran_unif_double(14,0);
  }
  else if (time_of_pregnancy >180)
  {
    if (index <=5)
      return time_of_pregnancy+ran_unif_double(14,0);
  }
  }*/
}

double Random_Number_Generator::time_of_rest_after_calving( int calving_number) //unabhängig von der calving number?!
{
  return ran_triangular_double(bvd_const::time_of_rest.min,
			                         bvd_const::time_of_rest.max,
			                         bvd_const::time_of_rest.mod);
}

bool Random_Number_Generator::is_this_a_deadbirth( bool first_pregnancy )
{
  int index = ran_unif_int(100,0);           // 17% of calves of heifers are born dead

  if ( first_pregnancy ){
    if (index < bvd_const::deadbirth.heifer)
    {
      return true;
    }
    return false;
  }
  else{                     // 8% of calves of dairy cows are born dead
    if (index < bvd_const::deadbirth.cow)
    {
      return true;
    }
    return false;
  }

}


double Random_Number_Generator::duration_of_infection()
{
  return ran_triangular_double(bvd_const::duration_of_infection.min,
                               bvd_const::duration_of_infection.max,
                               bvd_const::duration_of_infection.mod);
}

bool Random_Number_Generator::is_calf_female()
{
  int index = ran_unif_int(100,0);
  if (index < 50)
    return true;  //is female
  return false;  //is male
}

double Random_Number_Generator::duration_of_MA()
{
  return ran_triangular_double(bvd_const::duration_of_MA.min,
                                bvd_const::duration_of_MA.max,
                                bvd_const::duration_of_MA.min); //mod=min
}

/*double lifetime_male_calves() // ADD DISTRIBION FROM JOERN, HIER BEISPIELHAFT UNIFORM ZWISCHEN 7-180 TAGE
{
  return ran_unif_double(180,7);
}
*/

double Random_Number_Generator::life_expectancy_male_cow()
{
  double index;
  index = ran_unif_double(100,0);
  if (index < 30.){
    return ran_triangular_double(bvd_const::life_expectancy_males_first_third.min,
                            bvd_const::life_expectancy_males_first_third.max,
                            bvd_const::life_expectancy_males_first_third.mod);
  }

  if (index < 65.){
    return ran_triangular_double(bvd_const::life_expectancy_males_second_third.min,
                            bvd_const::life_expectancy_males_second_third.max,
                            bvd_const::life_expectancy_males_second_third.mod);
  }
  else{
    return ran_triangular_double(bvd_const::life_expectancy_males_third_third.min,
                            bvd_const::life_expectancy_males_third_third.max,
                            bvd_const::life_expectancy_males_third_third.mod);
  }
}


double Random_Number_Generator::first_insemination_age()
{
  return ran_triangular_double(bvd_const::first_insemination_age.min,
                               bvd_const::first_insemination_age.max,
                               bvd_const::first_insemination_age.mod);
}

double Random_Number_Generator::time_of_death_as_calf()
{
  double index;
  index = ran_unif_double(100,0);
  if (index < bvd_const::propability_of_death_as_calf.first_two_days){                      //propability 0f 2,5% that calve dies in first 2 days
    return ran_unif_double(bvd_const::time_of_death_as_calf.first_two_days,0);
  }

  if (index < bvd_const::propability_of_death_as_calf.first_six_month){ //propability 0f 10.2% that calve dies between 3 and 180 days
    return ran_unif_double(bvd_const::time_of_death_as_calf.first_six_month,
                           bvd_const::time_of_death_as_calf.first_two_days);
  }
  if (index < bvd_const::propability_of_death_as_calf.first_twelve_month){ //propability 0f 2.3% that calve dies between 181 and 360 days
    return ran_unif_double(bvd_const::time_of_death_as_calf.first_twelve_month,
                           bvd_const::time_of_death_as_calf.first_six_month );
  }
  else{
    return -1;    //survival as calf
  }
}

double Random_Number_Generator::duration_of_pregnancy()
{
  return ran_unif_double(bvd_const::pregnancy_duration.max,
                          bvd_const::pregnancy_duration.min);
}

double Random_Number_Generator::staggering_first_inseminations() {
    return ran_unif_double(bvd_const::pregnancy_duration.min, 0.0);
}

double Random_Number_Generator::insemination_result( bool first_pregnancy , bool* conception)
{
    double index = ran_unif_double(100,0);  // Andersrum aufbauen und mehrfach aus der Dreiecksverteilung ziehen.
    auto time_between_two_inseminations = [this](){return ran_triangular_double(bvd_const::time_between_inseminations.min,
                                                                                bvd_const::time_between_inseminations.max,
                                                                                bvd_const::time_between_inseminations.mod);};
    if ( first_pregnancy ){
        if (index < bvd_const::number_inseminations_heifer.zero){
            *conception = true;
            return 0.0;
        } else if (index < bvd_const::number_inseminations_heifer.one){
            *conception = true;
            return time_between_two_inseminations();
        } else if (index< bvd_const::number_inseminations_heifer.two){
            *conception = true;
            return (time_between_two_inseminations()
                    + time_between_two_inseminations()); //Dreiechsverteilung 2 mal ausgeführt
        } else if (index < bvd_const::number_inseminations_heifer.three){
            *conception = true;
            return (time_between_two_inseminations()
                    + time_between_two_inseminations()
                    + time_between_two_inseminations());
        } else {
            //time till death takes place 0-14 days
            // Not right, I think. The waiting times between the inseminations have been neglected.
            // Cannot occur with the present model constants
            *conception = false;
            return ran_unif_double(bvd_const::time_till_death_takes_place,0);
        }
    } else {
        if (index < bvd_const::number_inseminations_cow.zero) {
            *conception = true;
            return 0.0;
        } else if (index < bvd_const::number_inseminations_cow.one){
            *conception = true;
            return time_between_two_inseminations();
        } else if (index < bvd_const::number_inseminations_cow.two){
            *conception = true;
            return (time_between_two_inseminations() + time_between_two_inseminations());
        } else if (index < bvd_const::number_inseminations_cow.three){
            *conception = true;
            return (time_between_two_inseminations() + time_between_two_inseminations() + time_between_two_inseminations());
        } else {
            //time till death takes place 0-14 days
            // Not right, I think. The waiting times between the inseminations have been neglected.
            *conception = false;
            return ran_unif_double(bvd_const::time_till_death_takes_place,0);
        }
    }
}

double Random_Number_Generator::conception_result( double time_of_pregnancy, Infection_Status is_of_mother , bool* birth )
{
  int index = ran_unif_double(100.,0.);
  if (index < bvd_const::conception_result.second_month)
  {
    *birth = false;
    return ran_unif_double(bvd_const::conception_result_time.second_month,0.);
  }
  else if (index < bvd_const::conception_result.third_month)
  {
    *birth = false;
    return ran_unif_double(bvd_const::conception_result_time.third_month,
                           bvd_const::conception_result_time.second_month);
  }
  else if (index < bvd_const::conception_result.fourth_month)
  {
    *birth = false;
    return ran_unif_double(bvd_const::conception_result_time.fourth_month,
                           bvd_const::conception_result_time.third_month);
  }
  else if (index < bvd_const::conception_result.rest_months)
  {
    *birth = false;
    return ran_unif_double(bvd_const::conception_result_time.rest_months,
                           bvd_const::conception_result_time.fourth_month);
  }
  else{
    *birth = true;
    return duration_of_pregnancy();
  }
}

void Random_Number_Generator::getNRandomNumbersInRange(int n, int rangeLeft, int rangeRight, int* resultArray){

	//resultArray = (int *) malloc(n*sizeof(int));
	bool reroll = true;
	for(int i = 0; i < n; i++){
		reroll = true;
		while(reroll){
			resultArray[i] = this->ran_unif_int(rangeRight, rangeLeft);
			if(i > 0){
				for(int j = 0; j < i; j++){
					if(resultArray[j] == resultArray[i]){
						#ifdef _RNG_DEBUG_
							std::cout << "doppelter Index" << std::endl;
						#endif
						reroll = true;
						break;
					}else{

						reroll = false;
					}
				}
			}else{
				reroll = false;
			}
		#ifdef _RNG_DEBUG_
			std::cout << "RNJESUS: random number " << resultArray[i] << " at index " << i << std::endl;
		#endif
		}
	}
}
double Random_Number_Generator::lifetime_PI()
{
  int index = ran_unif_double(100,0);

  if (index < bvd_const::probability_lifetime_PI.first_year)
    return ran_unif_double(365,0);
  else if (index <= bvd_const::probability_lifetime_PI.second_year)
    return ran_unif_double(730,365);
  else if (index <= bvd_const::probability_lifetime_PI.third_year)
    return ran_unif_double(1095,731);
  else if (index <= bvd_const::probability_lifetime_PI.fourth_year)
    return ran_unif_double(1460,1096);
  else
    return ran_unif_double(3650 , 1461);
}

double Random_Number_Generator::time_of_death_infected_calf()
{
  return ran_unif_double(bvd_const::time_of_death_infected_calf.max,
                         bvd_const::time_of_death_infected_calf.min);
}

double Random_Number_Generator::time_of_next_infection( double rate )
{
  // The GSL uses the anglo-american convention of parametrizing the exponential distribution by the average waiting time between two events.
  // Those good old europeans that we are use the inverse, the number of events per time. Thus the 1.0/rate business here.
  if ( rate > 0.0)
    return  gsl_ran_exponential ( generator , 1.0/rate );
  return std::numeric_limits<double>::max();
}

int Random_Number_Generator::number_of_calvings()
{
  return ran_triangular_int(bvd_const::number_of_calvings.min,
                            bvd_const::number_of_calvings.max,
                            bvd_const::number_of_calvings.mod);
}

//double Random_Number_Generator::cowWellTimeOfBirth(double time){
//
//}

bool Random_Number_Generator::bloodTestRightResult(){
	double rndNum = this->ran_unif_double( 1.0, 0.0);
	return (rndNum < System::getInstance(nullptr)->activeStrategy->bloodTestAccuracy);
}

unsigned int Random_Number_Generator::getSeed(){
	return this->seed;
}
bool Random_Number_Generator::cowGetsASecondChance(){
	double rndNum = this->ran_unif_double( 1.0, 0.0);
	return (rndNum < bvd_const::probabilityOfOnlyOneTestForThatCow);
}

double Random_Number_Generator::timeOfFirstTest(){
	  return ran_triangular_double(bvd_const::time_of_first_test.min,
			                         bvd_const::time_of_first_test.max,
			                         bvd_const::time_of_first_test.mod);

}
double Random_Number_Generator::retestTime(){
	return ran_unif_double( System::getInstance(nullptr)->activeStrategy->retestingTimeBlood , 20.0);
}
double Random_Number_Generator::removeTimeAfterSecondTest(){  //between one to three weeks approximately
	return ran_unif_double( 22., 6.0);
}
bool Random_Number_Generator::vaccinationWorks(){
	double rndNum = this->ran_unif_double( 1.0, 0.0);
	return (rndNum <= System::getInstance(nullptr)->activeStrategy->vaccinationWorkingProbability);
}
