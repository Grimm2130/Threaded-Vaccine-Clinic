#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ds/QueueBuffer.h"

#define NUM_VIALS 							30
#define SHOTS_PER_VIAL 						6
#define NUM_CLIENTS 						((int)(NUM_VIALS * SHOTS_PER_VIAL))
#define NUM_NURSES 							10
#define NUM_STATIONS 						NUM_NURSES		
#define NUM_REGISTRATIONS_SIMULTANEOUSLY 	4

/* Global Variables */
int numVials = NUM_VIALS;		        	//This is the number of vialsMutex
int Stations[NUM_STATIONS];					// representation of stations
int StationLookup[ NUM_CLIENTS + 1 ];		// Table for looking up the current station of client

QueueBuffer_t ClientBuffer;
QueueBuffer_t StationBuffer;

/* Pthread */
pthread_t clientThreads[NUM_CLIENTS];
pthread_t nurseThreads[NUM_NURSES];
/* Semaphores */
sem_t regTableSemaphore;

/* Mutex */
pthread_mutex_t logMutex;
pthread_mutex_t vialMutex;
pthread_mutex_t ClientBufferMutex;					// Mutex for client Queue buffer
pthread_mutex_t StationBufferMutex;					// Station buffer mutex
pthread_mutex_t StationLookupMutex;					// Mutex for station buffer
pthread_mutex_t StationsMutex[NUM_STATIONS];		// Stations in the mutex

/* Conditionals*/
pthread_cond_t stationsConditionalVar[ NUM_STATIONS ];	// Array of condition variables for each station
pthread_cond_t stationLookupConditionalVar[ NUM_STATIONS ];	// Array of condition variables for each station


/* Enum */
/// @brief Person type
typedef enum PersonType
{
	Client_t,
	Nurse_t
} PersonType_t;

/* Functions */
char *curr_time_s();
void * Nurse( void * arg );
void * Client( void * arg );
char* GetPersonType( PersonType_t pt );
static void InitStationBuffer( void );
static void InitStationsMutex( void );
static void DestroyStationsMutex( void );
static void InitStationLookup( void );


int main( void )
{
    /*Init global variables*/
	// Queue init
	InitStationBuffer();
	QueueInit( &ClientBuffer, NUM_CLIENTS );

	// Semaphores
	sem_init( &regTableSemaphore, 0, NUM_REGISTRATIONS_SIMULTANEOUSLY );

	// Mutexes
	pthread_mutex_init( &vialMutex, NULL );
	pthread_mutex_init( &logMutex, NULL );
	pthread_mutex_init( &ClientBufferMutex, NULL );
	pthread_mutex_init( &StationBufferMutex, NULL );
	pthread_mutex_init( &StationLookupMutex, NULL );
	InitStationsMutex();
	InitStationLookup();

	// Conditionals
	for( int i = 0; i < NUM_STATIONS; i++)
	{
		pthread_cond_init( &stationsConditionalVar[i], NULL );
		pthread_cond_init( &stationLookupConditionalVar[i], NULL );
	}

	// Start threads
	for( int i = 0; i < NUM_NURSES; i++ )
	{
		int id = i;
		pthread_create( &nurseThreads[i], NULL, Nurse, (void*)&id ); 
	}
	for( int i = 0; i < NUM_CLIENTS; i++ )
	{
		int id = i;
		pthread_create( &clientThreads[i], NULL, Client, (void*)&id ); 
	}

	// Join threads
	for( int i = 0; i < NUM_NURSES; i++ )
	{
		pthread_join( nurseThreads[i], NULL ); 
	}
	for( int i = 0; i < NUM_CLIENTS; i++ )
	{
		pthread_join( clientThreads[i], NULL ); 
	}

	// destroy all global variables and synchonization mechanisms
	// Conditionals
	for( int i = 0; i < NUM_STATIONS; i++)
	{
		pthread_cond_destroy( &stationsConditionalVar[i] );
		pthread_cond_destroy( &stationLookupConditionalVar[i] );
	}

	// Mutexes
	pthread_mutex_destroy( &vialMutex );
	pthread_mutex_destroy( &logMutex );
	pthread_mutex_destroy( &ClientBufferMutex );
	pthread_mutex_destroy( &StationBufferMutex );
	pthread_mutex_destroy( &StationLookupMutex );
	DestroyStationsMutex();

	// Semaphores
	sem_destroy( &regTableSemaphore );

	QueueDestroy( &ClientBuffer );
	QueueDestroy( &StationBuffer );

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
    // PersonType_t person = Nurse_t;
	int Id = *((int*)arg);
	pthread_mutex_lock( &logMutex );
	printf("%s | Nurse %d clocking in\n", curr_time_s(), Id);
	pthread_mutex_unlock( &logMutex );

	pthread_mutex_lock( &vialMutex );
	int nVails = numVials;
	pthread_mutex_unlock( &vialMutex );
	
	pthread_mutex_lock( &logMutex );
	printf("Debug : Num of vials available %d\n", nVails);
	pthread_mutex_unlock( &logMutex );
	
	while( nVails > 0 )
	{

		pthread_mutex_lock( &vialMutex );
		numVials--;
		pthread_mutex_unlock( &vialMutex );

		int shots = SHOTS_PER_VIAL;
		
		pthread_mutex_lock( &logMutex );
		printf("%s Nurse %d getting new vial\n", curr_time_s(), Id );
		pthread_mutex_unlock( &logMutex );

		// Get available station
		pthread_mutex_lock( &ClientBufferMutex );
		int stationNum = QueuePop( &StationBuffer );
		pthread_mutex_unlock( &ClientBufferMutex );

		while( shots > 0 )
		{

			// increment the station person count
			pthread_mutex_lock( &StationsMutex[ stationNum ] );
			Stations[ stationNum ]++;
			pthread_mutex_unlock( &StationsMutex[ stationNum ] );

			pthread_mutex_lock( &logMutex );
			printf("%s | Nurse %d @ Station {%d} checks queue for an available client\n", curr_time_s(), Id, stationNum);
			pthread_mutex_unlock( &logMutex );
			

			int clientId;
			do
			{
				// pthread_mutex_lock( &ClientBufferMutex );
				// queueEmpty = QueueIsEmpty( &ClientBuffer );
				// pthread_mutex_unlock( &ClientBufferMutex );

				// Get client Id
				pthread_mutex_lock( &ClientBufferMutex );
				clientId = QueuePop( &ClientBuffer );
				pthread_mutex_unlock( &ClientBufferMutex );
			}
			while( clientId == -1 );


			pthread_mutex_lock( &StationLookupMutex );
			StationLookup[ clientId ] = stationNum;
			pthread_mutex_unlock( &StationLookupMutex );

			// Signal client of available buffer
			pthread_mutex_lock( &logMutex );
			printf("%s | Nurse %d signals for Client-[%d] to come over to station [%d]\n", curr_time_s(), Id, clientId, stationNum );
			pthread_mutex_unlock( &logMutex );

			pthread_cond_signal( &stationLookupConditionalVar[clientId] );

			pthread_mutex_lock( &logMutex );
			printf("%s | Nurse %d administers vaccine to Client-[%d]\n", curr_time_s(), Id, clientId );
			pthread_mutex_unlock( &logMutex );
			sleep(1);

			pthread_mutex_lock( &StationLookupMutex );
			StationLookup[ clientId ] = -1;
			pthread_mutex_unlock( &StationLookupMutex );

			pthread_mutex_lock( &logMutex );
			printf("%s | Nurse %d finished and asks Client-[%d] to leave\n", curr_time_s(), Id, clientId );
			pthread_mutex_unlock( &logMutex );

			pthread_cond_signal( &stationLookupConditionalVar[clientId] );
			shots--;

			pthread_mutex_lock( &logMutex );
			printf("%s | Nurse %d has %d shots of the vaccine left\n", curr_time_s(), Id, shots );
			pthread_mutex_unlock( &logMutex );
		}

		pthread_mutex_lock( &logMutex );
		printf("%s | Nurse %d leaves to go refill the vial\n", curr_time_s(), Id );
		pthread_mutex_unlock( &logMutex );

		pthread_mutex_lock( &ClientBufferMutex );
		QueueInsert( &StationBuffer, stationNum );
		pthread_mutex_unlock( &ClientBufferMutex );

		pthread_mutex_lock( &StationsMutex[ stationNum ] );
		Stations[ stationNum ]--;
		pthread_mutex_unlock( &StationsMutex[ stationNum ] );

		pthread_mutex_lock( &vialMutex );
		nVails = numVials;
		pthread_mutex_unlock( &vialMutex );
	
	}

	pthread_mutex_lock( &logMutex );
	printf("%s | Out of Vails ! Nurse %d leaves clinic\n", curr_time_s(), Id );
	pthread_mutex_unlock( &logMutex );

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
    // PersonType_t person = Client_t;
	int Id = *((int*)arg);

	pthread_mutex_lock( &logMutex );
	printf("%s | Client %d Walks in...\n", curr_time_s(), Id);
	printf("%s | Client %d gets in line for the cash register\n", curr_time_s(), Id);
	pthread_mutex_unlock( &logMutex );

	// Access via semaphore
	sem_wait( &regTableSemaphore );

	pthread_mutex_lock( &logMutex );
	printf("%s | Client %d has registered and now moves towards queue for the station\n", curr_time_s(), Id);
	pthread_mutex_unlock( &logMutex );

	sem_post( &regTableSemaphore );

	// Append client to the queue
	pthread_mutex_lock( &ClientBufferMutex );
	QueueInsert( &ClientBuffer, Id );
	pthread_mutex_unlock( &ClientBufferMutex );

	// Wait for client to get released
	pthread_mutex_lock( &StationLookupMutex );

	int booth = StationLookup[ Id ];

	pthread_mutex_lock( &logMutex );
	printf("Debug : Waiting for Client %d to get called\n", Id);
	pthread_mutex_unlock( &logMutex );

	while( booth == -1 )
	{
		pthread_cond_wait( &stationLookupConditionalVar[Id], &StationLookupMutex );
		booth = StationLookup[ Id ];
	}
	pthread_mutex_unlock( &StationLookupMutex );
	
	pthread_mutex_lock( &logMutex );
	printf("%s | Client %d Called to booth %d\n", curr_time_s(), Id, booth);
	pthread_mutex_unlock( &logMutex );

	pthread_mutex_lock( &StationsMutex[ booth ] );

	Stations[ booth ]++;
	// Check if this booth has 2 occupants
	if( Stations[ booth ] < 2 )
	{	
		pthread_mutex_lock( &logMutex );
		printf("%s | Client %d waiting for the nurse to arrive\n", curr_time_s(), Id);
		pthread_mutex_unlock( &logMutex );

		pthread_cond_wait( &stationsConditionalVar[Id], &StationsMutex[ booth ]);
	}

	pthread_mutex_unlock( &StationsMutex[ booth ] );

	// pthread_mutex_lock( &StationLookupMutex );
	// int newBooth = StationLookup[ Id ];
	// pthread_mutex_unlock( &StationLookupMutex );

	pthread_mutex_lock( &StationLookupMutex );

	// printf("\tDebug Booth count %d\n", Stations[ booth ]);
	// Check if this booth has 2 occupants
	while( StationLookup[ Id ] != -1 )
	{
		pthread_mutex_lock( &logMutex );
		printf("%s | Client %d Waiting for nurse to administer vaccine\n", curr_time_s(), Id);
		pthread_mutex_unlock( &logMutex );
		
		pthread_cond_wait( &stationLookupConditionalVar[Id], &StationLookupMutex );
		// printf("\tDebug : Waiting... Booth count %d\n", Stations[ booth ]);
	}
	// printf("\tDebug Booth count %d\n", Stations[ booth ]);

	pthread_mutex_unlock( &StationLookupMutex );

	pthread_mutex_lock( &logMutex );
	printf("%s | Client %d leaving the clinic\n", curr_time_s(), Id);
	pthread_mutex_unlock( &logMutex );

	Stations[ booth ]--;

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

/* General functions*/
char* GetPersonType( PersonType_t pt )
{
	switch (pt)
	{
        case Client_t: return "Client";
        case Nurse_t: return "Nurse";
        default: return "Unknown";
	}
}

/// @brief Initialize the buffer with all the available stations
/// @param  
static void InitStationBuffer( void )
{
	QueueInit( &StationBuffer, NUM_STATIONS );
	int currentStattion = 0;
	while( !QueueIsFull( &StationBuffer ) )
	{
		QueueInsert( &StationBuffer, currentStattion++ );
	}
}


static void InitStationLookup( void )
{
	for( int i = 0; i < NUM_CLIENTS+1; i++ )
	{
		StationLookup[i] = -1;
	}
}

/// @brief Function to initialize the stations mutexes
/// @param  
static void InitStationsMutex( void )
{
	for( int i = 0; i < NUM_STATIONS; i++ )
	{
		pthread_mutex_init( &StationsMutex[i], NULL );
	}
}

/// @brief Function to initialize the stations mutexes
/// @param  
static void DestroyStationsMutex( void )
{
	for( int i = 0; i < NUM_STATIONS; i++ )
	{
		pthread_mutex_destroy( &StationsMutex[i] );
	}
} 