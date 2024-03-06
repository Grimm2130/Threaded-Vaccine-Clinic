#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define NUM_THREADS				2
#define MAX_FUEL				75
#define FUEL_UPDATE_RATE		10
int fuel;

pthread_mutex_t fuelMutex;
pthread_cond_t fuelCond;
pthread_t threads[ NUM_THREADS ];


void * car( void * arg );
void * fillingStation( void * arg );


int main( void )
{
	// init mutex
	pthread_mutex_init( &fuelMutex, NULL );
	pthread_cond_init( &fuelCond, NULL );

	for( int i = 0; i < 2; i++ )
	{
		if( i & 1 )
		{
			if( pthread_create( &threads[i], NULL, fillingStation, NULL ) != 0 )
			{
				perror("Error creating thread");
			}
		}
		else
		{
			if( pthread_create( &threads[i], NULL, car, NULL ) != 0 )
			{
				perror("Error creating thread");
			}
		}
	}
	for( int i = 0; i < NUM_THREADS; i++ )
	{
		pthread_join( threads[i], NULL );
	}
	printf("Joined threads\n");

	pthread_mutex_destroy( &fuelMutex );
	pthread_cond_destroy( &fuelCond );
	return 0;
}


void * car( void * arg )
{	
	int currFuel;
	// read
	pthread_mutex_lock( &fuelMutex );
	currFuel = fuel;
	pthread_mutex_unlock( &fuelMutex );

	// Fill tank
	pthread_mutex_lock( &fuelMutex );
	printf("\tCar : Obtained Lock!\n");

	while(  currFuel < (45) )
	{
		printf("\tWaiting on gas...| Current %d\n", currFuel);
		if(pthread_cond_wait( &fuelCond,  &fuelMutex )) perror("Conditional Wait failed"); 
		printf("Car | Released\n");
		currFuel = fuel;
	}

	printf("Done\n");
	fuel -= FUEL_UPDATE_RATE * 3 ;
	currFuel = fuel;
	
	printf("\tCar : Releasing Lock!\n");
	pthread_mutex_unlock( &fuelMutex );

	printf("Car | Driving off | Current fuel left in car tank : %d\n", currFuel);
	// pthread_exit(NULL);
}

/// @brief Simulate car being filled with gas
/// @param arg 
/// @return 
void * fillingStation( void * arg )
{
	// Wait for fuel to be at useable value
	int i = 0;
	int end = 5;
	while(  i < end )
	{	
		pthread_mutex_lock( &fuelMutex );
		printf("\tFilling Station : Obtained Lock!\n");
		// Consume Fuel
		fuel += FUEL_UPDATE_RATE;
		// currFuel = fuel;
		printf("Filling Station | Current fuel in car tank : %d\n", fuel);
		// Condition Signal
		printf("\tFilling Station : Releasing Lock!\n");
		pthread_mutex_unlock( &fuelMutex );

		printf("\tFilling Station | Signalling car to check\n");
		pthread_cond_signal( &fuelCond );
		i++;
		// sleep(1);
	}
	printf("Filling Station | Done Filling!!\n");

	// pthread exit
	// pthread_exit(NULL);
}