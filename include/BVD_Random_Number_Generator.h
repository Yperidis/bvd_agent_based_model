#ifndef __bvd_rng_h_
#define __bvd_rng_h_

#include "Cow.h"
#include <cmath>
#include <gsl/gsl_rng.h>
#include <time.h>
#include "Model_Constants.h"
#include <cmath>
#include <gsl/gsl_rng.h>
#include <time.h>

class Random_Number_Generator
{
 public:
	Random_Number_Generator();
	Random_Number_Generator( unsigned int seed );
	void init( unsigned int seed);
	/// draw a random integer/double between lo (inclusive) and hi (exclusive)
	/// from a uniform distribution.
	int          ran_unif_int(    int hi   , int lo=0);
	double       ran_unif_double( double hi, double lo=0 );

	int ran_triangular_int(double lo,double hi,double mod);
	double ran_triangular_double(double lo,double hi,double mod);

	int ran_poisson_int(int r, double mu);

	// true or false, depending on whether a calf ( age < bvd_const::age_threshold_calf ) just infected with BVD will die or not
	bool         will_TI_calf_die();
	double       time_of_death_infected_calf();
	double       time_of_next_infection(double rate);
	Calf_Status calf_outcome_from_infection      ( double  time_of_pregnancy );
	double       time_of_abortion_due_to_infection( double  time_of_pregnancy );
	double       time_of_rest_after_calving(int calving_number);

	// true or false if a birth is a deadbirth, assuming that the infection status of neither mother nor calf are known.
	// This depends on the age of the mother, as indicated in table 4.1 of the handout ("Anteil nicht lebend ausgetragener Kälber, Färsen/Kühle")
	// The wording of the handout is not clear whether in the figures of the handout also abortions are included.
	// In this function, abortions are *not* included.
	bool         is_this_a_deadbirth( bool first_pregnancy );

	//How long is a calf protected by the maternal antibodies in the colostrum.
	double       duration_of_MA();

	//Duration of an infection with BVD ( time that the infection state 'transiently infected' lasts )
	double       duration_of_infection();
	double         duration_of_pregnancy();
	double staggering_first_inseminations();
	int          number_of_calvings();
	virtual bool         is_calf_female();
	//  double         lifetime_male_calves();
	double       first_insemination_age();
	double       lifetime_PI();
	double       time_of_death_as_calf();
	double       insemination_result( bool first_pregnancy , bool* conception );
	double       life_expectancy_male_cow();



	//It has to be decided whether a beginning pregnancy will lead to an abortion or a birth.
	// Also here, pointer *birth is meant for output. if false, abortion is the case. If true birth is the case.
	// The return value is the time until the event (in case of birth -> duration of pregnancy, in case of abortion -> time until abortion ).
	// afaik, in the statistics we have, only the total abortion rates are given.
	// We don't know yet (as of 10.12.2015) whether there is a correlation between abortion happening and the infection status of the mother or the age of the mother.
	// So for now, treat it as uncorrelated. And use the total abortion rates given in the handout.
	double       conception_result( double age_of_mother, Infection_Status is_of_mother, bool* birth );

	void getNRandomNumbersInRange(int n, int rangeLeft, int rangeRight, int* resultArray);
	//double cowWellTimeOfBirth(double time);
	bool bloodTestRightResult();

	double removeTimeAfterSecondTest();
	double retestTime();
	unsigned int getSeed();
	bool 		 cowGetsASecondChance();
	double 		timeOfFirstTest();
	virtual bool 		vaccinationWorks();
 private:
  gsl_rng * generator;
unsigned int seed ;
};

#endif
