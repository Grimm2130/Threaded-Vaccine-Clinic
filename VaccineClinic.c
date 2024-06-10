#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ds/queue.h"
#include "ds/lookup_buffer.h"
#include "../timers/WheelTimerLib/output_formatters_defs/print_formatters.h"
#define NUM_VIALS 							10
#define SHOTS_PER_VIAL 						6
#define NUM_CLIENTS 						((int)(NUM_VIALS * SHOTS_PER_VIAL))
#define NUM_NURSES 							10
#define NUM_STATIONS 						NUM_NURSES		
#define NUM_REGISTRATIONS_SIMULTANEOUSLY 	4

/* Global Variables */
int client_count = NUM_CLIENTS;				// client count
int nurse_count = NUM_NURSES;				// nurse count
int numVials = NUM_VIALS;		       	//This is the number of vialsMutex
lookup_buffer_t client_at_station;				// marks stations as occupied by client or not
lookup_buffer_t nurse_done_station;				// marks stations as occupied by client or not
// lookup_buffer_t StationLookup[ NUM_CLIENTS + 1 ];		// Table for looking up the current station of client
queue_t stations_available_nurse;
queue_t stations_available_client;

/* Pthread */
pthread_t clientThreads[NUM_CLIENTS];
pthread_t nurseThreads[NUM_NURSES];
/* Semaphores */
sem_t regTableSemaphore;

/* Mutex */
pthread_rwlock_t client_available_lock;
pthread_rwlock_t vail_available_lock;
pthread_spinlock_t vial_lock;
pthread_mutex_t stations_available_nurse_mutex;		// Stations in the mutex
pthread_mutex_t stations_available_client_mutex;		// Stations in the mutex

/* Conditionals*/
pthread_cond_t stations_available_nurse_cv;	// Array of condition variables for each station
pthread_cond_t stations_available_client_cv;	// Array of condition variables for each station



/* Functions */
char *curr_time_s();
void * Nurse( void * arg );
void * Client( void * arg );
bool vails_available();
void update_vail_count();
bool is_client_available();
void update_client_count();


int main( void )
{
	// init buffer
	lookup_buffer_init( &client_at_station, NUM_STATIONS );
	lookup_buffer_init( &nurse_done_station, NUM_STATIONS );
	// init queue
	queue_init( &stations_available_client );
	queue_init( &stations_available_nurse );
	// rw init
	pthread_rwlock_init( &client_available_lock, NULL );
	pthread_rwlock_init( &vail_available_lock, NULL );
	// mutex init
	pthread_mutex_init( &stations_available_nurse_mutex, NULL );
	pthread_mutex_init( &stations_available_client_mutex, NULL );
	// cond vars init
	pthread_cond_init( &stations_available_nurse_cv, NULL );
	pthread_cond_init( &stations_available_client_cv, NULL );
	// spin init
	pthread_spin_init( &vial_lock, 0 );
	// sem init
	sem_init( &regTableSemaphore, 0, NUM_REGISTRATIONS_SIMULTANEOUSLY );
	// Isnstantiate stations
	for( int i = 0; i < NUM_STATIONS; i++ )
		queue_append( &stations_available_nurse, i );

	printf("Starting threads\n");
	for( int i = 0; i < NUM_NURSES; i++ )
	{
		u_int32_t* id = (uint32_t*)calloc(1, sizeof(uint32_t));
		*id = i;
		pthread_create( &nurseThreads[i], NULL, Nurse, id );
	}

	for( int i = 0; i < NUM_CLIENTS; i++ )
	{
		u_int32_t* id = (uint32_t*)calloc(1, sizeof(uint32_t));
		*id = i;
		pthread_create( &clientThreads[i], NULL, Client, id );
	}

	for( int i = 0; i < NUM_NURSES; i++ )
	{
		pthread_join( nurseThreads[i], NULL );
	}

	for( int i = 0; i < NUM_CLIENTS; i++ )
	{
		pthread_join( clientThreads[i], NULL );
	}

	printf("%s | Closing Clinic\n", curr_time_s());
	// exit
    return 0;
}

/*
	- Goes to the back to get the vial
	- Decrements vials left
	- ** Waits for a Station to become available **
	Station value from the Queue
	
	while there's shots left from vial

		- ** Requests a new client from the client queue **
			Pop Sets index to zero and listening client is signaled

		- Stores the location of the station on the client lookup

		- Waits for the arrival of the client: 
			Station Value value must be 2, indicating 2 occupants ( nurse and client )
		
		- simulate injection sleep

		- Wait for client to leave

		- clear Station value @ lookup

	- Push Station value back to queue
*/
/// @brief Nurse thread function
/// @param arg 
/// @return Nothing
void * Nurse( void * arg )
{
	uint32_t id = *(uint32_t*)arg;

	#if LOG_LEVEL_1
	printf("(%s) | Nurse [%d] starts shift\n", __func__, id );
	#endif


    while( vails_available() && is_client_available() )
	{
		update_vail_count();	
		update_client_count();
		pthread_spin_lock( &vial_lock );
		int vial_count = numVials;
		pthread_spin_unlock( &vial_lock );		

		#if LOG_LEVEL_2
		printf("\t(%s) | Nurse [%d] takes vail\n", __func__, id);
		#endif

		#if DEBUG
		printf("\t\t(%s) | [ __DEBUG__ ] %d Vail(s) Available\n", __func__, vial_count );
		#endif

		pthread_mutex_lock( &stations_available_nurse_mutex );
		while( queue_empty(&stations_available_nurse ) )
		{
			#if LOG_LEVEL_3
			printf("\t\t(%s) | [ __DEBUG__ ] Nurse [%d] waiting for stattion to become available\n", __func__, id);
			#endif
			pthread_cond_wait( &stations_available_nurse_cv, &stations_available_nurse_mutex );
		}
		pthread_mutex_unlock( &stations_available_nurse_mutex );

		// nurse goes to station
		uint32_t station = queue_pop( &stations_available_nurse );

		#if LOG_LEVEL_2
		printf("\t(%s) | Nurse [%d] goes to station %d\n", __func__, id, station );
		#endif

		int vaccines_administered = 0;
		// each vail holds 10 shots, after that replenish
		while( vaccines_administered < SHOTS_PER_VIAL && is_client_available() )		
		{
			lookup_buffer_set( &nurse_done_station, station, false );
			// make station available for client
			pthread_mutex_lock( &stations_available_client_mutex );
			queue_append( &stations_available_client, station );
			pthread_mutex_unlock( &stations_available_client_mutex );
			pthread_cond_signal( &stations_available_client_cv );

			// Wait for the client to arrive
			while( !lookup_buffer_get( &client_at_station, station ) )
			{
				#if LOG_LEVEL_3
				printf("\t\t(%s) | [ __DEBUG__ ] Nurse [%d] Waiting for client to come to station %d\n", __func__, id, station );
				#endif
				sleep(2);
			}

			#if LOG_LEVEL_2
			printf("\t(%s) | Client has arrived at station\n", __func__ );
			#endif

			#if LOG_LEVEL_2
			printf("\t(%s) | Nurse [%d] administering vaccine\n", __func__, id );
			#endif

			int iter = 5;
			printf(".....\n");

			// mark that nurse is done
			lookup_buffer_set( &nurse_done_station, station, true );

			// Wait for the client to arrive
			while( lookup_buffer_get( &client_at_station, station ) )
			{
				#if LOG_LEVEL_3
				printf("\t\t(%s) | [ __DEBUG__ ] Nurse [%d] Waiting for client to leave station %d\n", __func__, id, station );
				#endif
				sleep(2);
			}

			#if LOG_LEVEL_2
			printf("\t(%s) | Nurse [%d] done with client, signalling next client\n", __func__ , id);
			#endif

			vaccines_administered++;

			#if LOG_LEVEL_2
			printf("\t(%s) | Nurse [%d] has administed %d vaccines\n", __func__, id, vaccines_administered );
			#endif
		}

		#if LOG_LEVEL_2
		printf("\t(%s) | Nurse [%d] done with shift...leaving stattion %d\n", __func__, id, station );
		#endif
		pthread_cond_signal( &stations_available_nurse_cv );	// Signal station is available for nurse
	}
	free( arg );
    pthread_exit( NULL );
}

/*
	- Client Walks in
	- Client attempts to register
	
	- if space left @ registration table
		reduce registration semaphore
	else
		wait
	
	** Client Gets in line and waits turn for name to be called **
	- Insert Client ID and get index of insertion
	- { Wait } for signal that index of insertion to be set to zero, i.e. that the value has been popped
	
	- Look Up Station that's free from table

	- Increment Station Buffer ( should be 2 ) indicating nurse and client are @ table
	** Wait for nurse to finish injection before leaving **
	- While station buffer is == 2:
		Wait for signal from nurse ( in delay )

	- Signal nurse of departure

	leave clinic
*/
/// @brief Client thread function
/// @param arg 
/// @return 
void * Client( void * arg )
{
	uint32_t id = *(uint32_t*)arg;
	#if LOG_LEVEL_1
	printf("(%s) |  Client [%d] arrived at clinic\n", __func__, id );
	#endif

	sem_wait( &regTableSemaphore );

	#if LOG_LEVEL_1
	printf("(%s) |  Client [%d] heads to registration table to register\n", __func__, id);
	#endif
	sleep(3);
	sem_post( &regTableSemaphore );

	// wait for nurse to signal availability
	pthread_mutex_lock( &stations_available_client_mutex );
	while( queue_empty(&stations_available_client) )
	{
		#if LOG_LEVEL_3
		printf("\t\t(%s) | [ __DEBUG__ ] Client [%d] Waiting for nurse to sinal station is available\n", __func__, id);
		#endif
		pthread_cond_wait( &stations_available_client_cv, &stations_available_client_mutex );
		sleep(1);
	}
	pthread_mutex_unlock( &stations_available_client_mutex );

	pthread_mutex_lock( &stations_available_client_mutex );
	int station = queue_pop( &stations_available_client );
	pthread_mutex_unlock( &stations_available_client_mutex );

	#if LOG_LEVEL_2
	printf("\t(%s) | Client [%d] goes to stattion\n", __func__, id );
	#endif
	// mark the station lookup
	lookup_buffer_set( &client_at_station, station, true );

	while( !lookup_buffer_get( &nurse_done_station, station ) )
	{
		#if LOG_LEVEL_3
		printf("\t\t(%s) | [ __DEBUG__ ] Client [%d] Waiting for vaccine to finish being administered\n", __func__, id );
		#endif
		sleep(1);
	}

	lookup_buffer_set( &client_at_station, station, false );

	#if LOG_LEVEL_2
	printf("\t(%s) | Client [%d] leaves station\n", __func__, id );
	#endif

	update_client_count();

	free( arg );
	pthread_exit( NULL );
}

/// @brief Gets the current time as a string
/// @return return time as string
char *curr_time_s() 
{
	char *time_str;
	time_t t;
	time(&t);
	// Get string representation
	time_str = ctime(&t);
	// replace ending newline with end-of-string.
	time_str[strlen(time_str) - 1] = '\0';
	return time_str;
}

bool vails_available()
{
	pthread_rwlock_rdlock( &vail_available_lock );
	bool res = numVials > 0;
	pthread_rwlock_unlock( &vail_available_lock ); 
	return res;
}

void update_vail_count()
{
	pthread_rwlock_wrlock( &vail_available_lock );
	numVials--;
	pthread_rwlock_unlock( &vail_available_lock ); 
}

bool is_client_available()
{
	pthread_rwlock_rdlock( &client_available_lock );
	bool res = client_count > 0;
	pthread_rwlock_unlock( &client_available_lock ); 
	return res;
}

void update_client_count()
{
	pthread_rwlock_wrlock( &client_available_lock );
	client_count--;
	pthread_rwlock_unlock( &client_available_lock ); 
}