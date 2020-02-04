#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h> 
#include <queue> 
#include <semaphore.h>
#include <cstdlib>
using namespace std;

#define NUM_THREADS 5
#define MEMORY_SIZE 1000

struct node
{
	int id;
	int size;
};


queue<node> myqueue; // shared que
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_t server; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores

int thread_message[NUM_THREADS]; // thread memory information
char  memory[MEMORY_SIZE]; // memory size



void release_function()
{
	//This function will be called
	//whenever the memory is no longer needed.
	//It will kill all the threads and deallocate all the data structures.
}

void my_malloc(int thread_id, int size)
{
	//This function will add the struct to the queue
	pthread_mutex_lock(&sharedLock);
	node threadNode; threadNode.id = thread_id; threadNode.size = size;
	myqueue.push(threadNode);
	pthread_mutex_unlock(&sharedLock);
}

void * server_function(void *)
{
	int index = 0; int memoryLeft = MEMORY_SIZE ;
	int count = 0;
	while (count < 5)
	{
		if (myqueue.empty() == false)
		{
			//This function should grant or decline a thread depending on memory size.
			pthread_mutex_lock(&sharedLock);
			node request = myqueue.front();
			myqueue.pop();
			if (request.size <= memoryLeft)
			{
				thread_message[request.id] = index;
				index = index + request.size;
				memoryLeft = memoryLeft - request.size;
				count++;
			}
			else if	(request.size > memoryLeft)
			{
				thread_message[request.id] = -1;
				count++;
			}
			sem_post(&semlist[request.id]);
			pthread_mutex_unlock(&sharedLock);
		}
		

	}
	
	
}

void * thread_function(void * id) 
{
	//This function will create a random size, and call my_malloc
	int randSize = rand()%(300);
	int* intId = (int*) id;
	my_malloc(*intId, randSize);

	//Block
	sem_wait(&semlist[*intId]);
	pthread_mutex_lock(&sharedLock);
	//Then fill the memory with 1's or give an error prompt

		if (thread_message[*intId] == -1)
			{
				cout << "Thread " << *intId << ": Not enough memory\n";
			}
		else
			{
				int start = thread_message[*intId];
				for (int i = 0; i < randSize; i++ ){
					memory[start] = '1';
					start++;
				}
			}
	pthread_mutex_unlock(&sharedLock);
}

void init()	 
{
	pthread_mutex_lock(&sharedLock);	//lock
	for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores
	{sem_init(&semlist[i],0,0);}
	for (int i = 0; i < MEMORY_SIZE; i++)	//initialize memory 
  	{char zero = '0'; memory[i] = zero;}
   	pthread_create(&server,NULL,server_function,NULL); //start server 
	pthread_mutex_unlock(&sharedLock); //unlock
}



void dump_memory() 
{
	cout << "Mermory dump:\n";
	for (int i = 0; i < MEMORY_SIZE; i++)
		cout << memory[i];
}

int main (int argc, char *argv[])
 {

 	//You need to create a thread ID array here
	 int ids[NUM_THREADS];
	 for (int i = 0; i < NUM_THREADS; i++) ids[i] = i;

 	init();	// call init

 	//You need to create threads with using thread ID array, using pthread_create()
	pthread_t threads[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_create(&threads[i], NULL, thread_function, (void*)&ids[i]);
	}

 	//You need to join the threads
	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i], NULL);
	}

 	dump_memory(); // this will print out the memory
 	printf("\nMemory Indexes:\n" );
 	for (int i = 0; i < NUM_THREADS; i++)
 	{
 		printf("[%d]" ,thread_message[i]); // this will print out the memory indexes
 	}
 	printf("\nTerminating...\n");
 }