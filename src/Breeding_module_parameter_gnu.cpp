#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <time.h>

int und (int ranLRmax)			//uniform number distribution
{

	const gsl_rng_type * T;
  	gsl_rng * r;

  	int i, n = 10;

	T = gsl_rng_default;

 	r = gsl_rng_alloc (T);

 	gsl_rng_env_setup();
 	//set a random seed
 	unsigned int seed = (time(NULL)  + rand());
    gsl_rng_set (r,seed);


    int ranLR;
  ranLR = gsl_rng_uniform_int(r, ranLRmax);
  //printf("A random long from U[ranLRmin,%d-1): %d\n", ranLRmax, ranLR);

  gsl_rng_free (r);

  return ranLR;
}



int days_till_abortion()
{
	int index = und(101);				//propability index 1-100
	printf("zufall: %d\n", index);

	if (index <= 7){						//propability 0f 7% that calve dies within 60 days
		return und(61);						//returns random number of uniform distribtion between 0-60 days
	}
	else if ( index <=9){		//propability 0f 2% that calve dies between 61 and 90 days
		return (und(30)+61);
	}
	else if (index == 10){					//propability 0f 1% that calve dies between 91 and 120 days
		return (und(30)+91);
	}
	else if (index >10 && index <=12){		//propability 0f 2% that calve dies between 121 and 240 days
		return (und(120)+121);
	}
	else{ 									//propability 0f 88% that no abortion will take place
		return -1;
	}
}

int days_till_calve_dies()					//gives the days untill a calve dies within its first 1-360 days
{
	int index = und(101);
	printf("zufall: %d\n", index);
	if (index > 0 && index <= 2.5){			//propability 0f 2,5% that calve dies in first 2 days
		return und(3);
	}
	index = und(101);
	printf("zufall: %d\n", index);			//propability 0f 10.2% that calve dies between 3 and 180 days
	if (index >0 && index <=10.2){
		return (und(178)+3);
	}
	index = und(101);
	printf("zufall: %d\n", index);			//propability 0f 2.3% that calve dies between 181 and 360 days
	if (index > 0 && index <=2.3){
		return (und(180)+181);
	}
	else{
		return -1;
	}
}

int number_of_inseminations(int calvingnumber)		//number of total inseminations, dependent on the number of calvings the cow went through
{
	int index = und(1001);
	printf("zufall: %d\n", index);

	if (calvingnumber ==0){							// if it's the first pregnancy
		if (index >0 && index <= 0.2){
			return 4;
		}
		if (index > 0.2 && index<= 5){
			return 3;
		}
		if (index > 5 && index<= 95){
			return 2;
		}
		if (index > 95 && index<= 100){
			return 1;
		}

	}
	else{											// if numer of calving is greater than 0
		if (index >0 && index <= 0.7){
			return 5;
		}
		if (index >0.7 && index <= 8){
			return 4;
		}
		if (index >8 && index <= 62){
			return 3;
		}
		if (index >62 && index <= 329){
			return 2;
		}
		if (index >329 && index <= 1000){
			return 1;
		}

		}
}

int calve_born_dead(int calvingnumber)				// if calve is born dead return 1, if not return -1
{
	int index = und(101);						// 17% of calves of heifers are born dead

	if (calvingnumber==0){
		if (index >0 && index <= 17){
			return 1;
		}
	}
	else{											// 8% of calves of dairy cows are born dead
		if (index >0 && index <= 8){
		return -1;
		}
	}
}


int time_of_gestation()								//gives back the number of days (280-292) till birth-event
{
	return (und(12)+280);
}
