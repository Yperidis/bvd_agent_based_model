#ifndef __model_constants_h_
#define __model_constants_h_


namespace bvd_const
{
  const double lambda_TI              = 0.03; // Default value (Viet 2004) 0.03
  const double lambda_PI              = 0.5; // Default value (Viet 2004) 0.5
  const double age_threshold_calf     = 180.; // (180 days = 6 months )
  //  const int    maximum_calving_number = 4;
  const double minimum_pregnancy_duration = 280.; // Minimum pregnancy duration
  const double threshold_abortion_counts_as_calving = 240.; // 8 months. Is that right?
  const int     lethality_TI_calf     = 2;  // 2% of TI calves die from the infection
  const int     time_till_abortion_takes_place = 2; // 2 percent abortion
  const int     time_till_death_takes_place = 14; // 0-14 days
  const double 	standard_trade_execution_time = 0.5; // time that a common trade needs to take place
  const double firstVaccAge = 186.;
  //FIXME With the input from the ini file the timeOfVaccinationPersistence is redundant
  const double timeOfVaccinationPersistance = 365.;
  const double probabilityOfSecondTestForThatCow = 0.08; // upper limit for the probability of a second test for
    // the cow. See cowGetsASecondChance() at BVD_Radnom.Number_Generator.cpp
  struct {
	  double DAILY = 1.0;
	  double WEEKLY = 7.0;
	  double BIWEEKLY = 14.0;
	  double MONTHLY = 30.41;
	  double QUARTERYEARLY = 91.25;
	  double HALFYEARLY = 182.5;
	  double YEARLY = 365.;
  }tradingTimeIntervall;
  struct {
    double min = 280.;
    double max = 292.;
  } pregnancy_duration;
  struct {
    double min = 42.;
    double max = 115.;
    double mod = 90.;
  } time_of_rest;
  struct{
    double first=70.;                            //0-70 days
    double second=120.;                          //70-120 days
    double third=180.;                           //120-180 days
  } period_of_pregnancy;
  struct{
    double PI=90.;
    double abort=90. + 10.;    // Definitely an abortion if not PI in the first period of pregnancy
    double cripple=0.;
    double immune=0.;
  } first_period_of_pregnancy; // There are biological grounds for the chosen values of 1st to 4th pregnancies.
  struct{
    double PI=45.;
    double abort=15.+45.;
    double cripple=15.+15.+45.;
    double immune=25.+15.+15.+45.;    // Definitely immune if nothing else in the second period of pregnancy
  } second_period_of_pregnancy;
  struct{
    double PI=0.;
    double abort=20.;
    double cripple=25.+20.;
    double immune=55.+25.+20.;    // Definitely immune if nothing else in the third period of pregnancy
  } third_period_of_pregnancy;
  struct{
    double PI=0.;
    double abort=5.;
    double cripple=15.+5.;
    double immune=80.+15.+5.;    // Definitely immune if nothing else in the fourth period of pregnancy
  } fourth_period_of_pregnancy;
  struct{
    double heifer=2.; //17
    double cow=2.; //8
  } deadbirth;
  struct{
    double min=4.;
    double max=8.;
    double mod=7.;
  } duration_of_infection;
  struct{
    double min=180.;
    double max=270.;
  } duration_of_MA;
  struct{
    double min=480.;
    double max=600.;
    double mod=540.;
  } first_insemination_age;
  struct{
    double first_two_days=2.5;  // 2.5%
    double first_six_month=12.5;  // 10%
    double first_twelve_month=14.;  // 1.5%
  } propability_of_death_as_calf;
  struct{
    double first_two_days=2.5;
    double first_six_month=182.5;
    double first_twelve_month=365.;
  } time_of_death_as_calf;
  struct{
    double zero=90.48;
    double one=9.05 + 90.48;
    double two=0.45 + 9.05 + 90.48;
    double three=0.02 + 0.45 + 9.05 + 90.48;
  } number_inseminations_heifer;  //Sums to 100, no possibility of not remaining pregnant for a heifer
  struct{
    double zero=67.03;
    double one=26.81+67.03;
    double two=5.36+26.81+67.03;
    double three=0.72+5.36+26.81+67.03;
  } number_inseminations_cow;  //Sums to 99.92, so 0.08 pct probability of infertility
  struct{
    double second_month=7.;
    double third_month=2.+7.;
    double fourth_month=1.+2.+7.;
    double rest_months=2.+1.+2.+7.;
  } conception_result;    // probability percentages that an abortion will happen depending on certain stages of the pregnancy
  struct{
    double second_month=60.;
    double third_month=90.;
    double fourth_month=120.;
    double rest_months=210.;
  } conception_result_time;    // The corresponding interval limits in days, where the abortion is scheduled to take place
  struct{
    double first_year=50.;  // see the implementation of lieftime_PI to make sense of the numbers
    double second_year=67.; // 17%
    double third_year=72.;  // 5%
    double fourth_year=73.5;// 1.5%
  } probability_lifetime_PI;
  struct{
    double min=0.;  // 5000? Previous value
    double max=7.;  // 5000? Previous value
  } time_of_death_infected_calf;     // infected calf dies between 0-7 days
  struct{
    double min=3.;
    double max=5.;
    double mod=4.;
  } number_of_calvings; // Have to be positive doubles (effectively integers)
  struct{
    double min=18.;
    double max=24.;
    double mod=21.;
  } time_between_inseminations;
  struct{
    double min=0.;
    double max=30.;
    double mod=10.;
  } life_expectancy_males_first_third;
  struct{
    double min=170.;
    double max=250.;
    double mod=200.;
  } life_expectancy_males_second_third;
  struct{
    double min=450.;
    double max=750.;
    double mod=640.;
  } life_expectancy_males_third_third;
struct {
    double min = 4.;
    double max = 30.;
    double mod = 11.;
  } time_of_first_test ;

}

#endif
