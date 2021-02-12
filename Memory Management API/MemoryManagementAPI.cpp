#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h> 
#include <queue> 
#include <semaphore.h>
using namespace std;

#define NUM_THREADS 10
#define MEMORY_SIZE 150

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

int next_avail =0; //keeps track of the next empty slot
int count =0;

void my_malloc(int thread_id, int size)
{
	//This function will add the struct to the queue

	node allocate = node();
	allocate.id= thread_id;
	allocate.size= size;
	myqueue.push(allocate);	
}

void * server_function(void *)
{
	//This function should grant or decline a thread depending on memory size.
	
    while(true)
    {
		 if(count == NUM_THREADS)
		{
            break;
		}
        if(!myqueue.empty())
        {
			pthread_mutex_lock(&sharedLock);
            
            if(MEMORY_SIZE - next_avail < myqueue.front().size)
			{
                thread_message[myqueue.front().id] = -1;
			}
            else
			{
                thread_message[myqueue.front().id] = next_avail;
				next_avail = myqueue.front().size+ next_avail;
			}
            sem_post(&semlist[myqueue.front().id]);
			count++;
			myqueue.pop();
			pthread_mutex_unlock(&sharedLock);
        }
       
    }

}
void * thread_function(void * id) 
{
	//This function will create a random size, and call my_malloc
	
	
	int rand_size = (rand() % (MEMORY_SIZE / 6)) + 1;
    int * id1 = (int *) id;
	pthread_mutex_lock(&sharedLock);
    my_malloc(*id1,rand_size);
	pthread_mutex_unlock(&sharedLock);
	
	//
    sem_wait(&semlist[*id1]);
    if(thread_message[*id1] == -1)
	{
		printf("thread[%d]: Not enough memory\n" ,*id1);
        //cout << "Thread " << *id1 << ": Not enough memory" << endl;
		
	}
    else
    {
		pthread_mutex_lock(&sharedLock);
        for(int i=0; i < rand_size; i++)
        {   
		   memory[thread_message[*id1]+i]= 48 + *id1 ;
		   
		}
        pthread_mutex_unlock(&sharedLock);
    }
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
    for(int i=0; i < MEMORY_SIZE; i++)
    cout << memory[i];
	  
}

int main (int argc, char *argv[])
 {
	srand(time(NULL));
 	//You need to create a thread ID array here
	
     int thread_id[NUM_THREADS];
     for(int i=0; i < NUM_THREADS; i++)
	 {
		 thread_id[i] = i;
	 }
   
	init();	// call init
	
	//You need to create threads with using thread ID array, using pthread_create()

	pthread_t thread[NUM_THREADS];
    for(int i=0; i < NUM_THREADS; i++){
	pthread_create(&thread[i],NULL,thread_function,(void *) &thread_id[i]);
	}
     
	//You need to join the threads
	 //unlock
	 for(int i=0; i < NUM_THREADS; i++)
	 {
         pthread_join(thread[i],NULL);
	 }
     

	dump_memory(); // this will print out the memory
	
 	printf("\nMemory Indexes:\n" );
 	for (int i = 0; i < NUM_THREADS; i++)
 	{
 		printf("[%d]" ,thread_message[i]); // this will print out the memory indexes
 	}
 	printf("\nTerminating...\n");
 }