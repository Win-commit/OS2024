#include "lib.h"
#include "types.h"

int uEntry(void) {
	// For lab4.1
	// Test 'scanf'
	// int dec = 0;
	// int hex = 0;
	// char str[6];
	// char cha = 0;
	// int ret = 0;
	// while (1)
	// {
	// 	printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
	// 	ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
	// 	printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
	// 	if (ret == 4)
	// 		break;
	// }
    
	// For lab4.2
	// Test 'Semaphore'
	// int i = 4;
	// sem_t sem;
	// printf("Father Process: Semaphore Initializing.\n");
	// int ret = sem_init(&sem, 2);
	// if (ret == -1)
	// {
	// 	printf("Father Process: Semaphore Initializing Failed.\n");
	// 	exit();
	// }

	// ret = fork();
	// if (ret == 0)
	// {
	// 	while (i != 0)
	// 	{
	// 		i--;
	// 		printf("Child Process: Semaphore Waiting.\n");
	// 		sem_wait(&sem);
	// 		printf("Child Process: In Critical Area.\n");
	// 	}
	// 	printf("Child Process: Semaphore Destroying.\n");
	// 	sem_destroy(&sem);
	// 	exit();
	// }
	// else if (ret != -1)
	// {
	// 	while (i != 0)
	// 	{
	// 		i--;
	// 		printf("Father Process: Sleeping.\n");
	// 		sleep(128);
	// 		printf("Father Process: Semaphore Posting.\n");
	// 		sem_post(&sem);
	// 	}
	// 	printf("Father Process: Semaphore Destroying.\n");
	// 	sem_destroy(&sem);
	// 	exit();
	// }
    
	// For lab4.3
	// TODO: You need to design and test the philosopher problem.
	// Producer-Consumer problem and Reader& Writer Problem are optional.
	// Note that you can create your own functions.
	// Requirements are demonstrated in the guide.
	printf("producer-consumer test started!\n");
	int s=0, ret=0;
	sem_t empty, full, mutex;
	int buffer_size = 3;
	int num_to_produce = 2;
	sem_init(&empty, buffer_size);
	sem_init(&full, 0);
	sem_init(&mutex, 1);
	for(s=0;s<4;s++){
		if(ret==0)ret = fork();
		else if(ret>0) break;
	}
	// 1 2 3 4 producer, 5 consumer
	int id = getpid();
	// producer
	if(id < 5){ 
		for(int i=0;i<num_to_produce;i++){
			sleep(128);
			sem_wait(&empty); 
			sleep(128);
			sem_wait(&mutex); 
			sleep(128);
			printf("Producer %d is producing\n", id);
			if(i==1)printf("Producer %d finished\n", id);
			sleep(128);
			sem_post(&mutex); 
			sleep(128);
			sem_post(&full); 
			sleep(128);
		}
	}
	// consumer
	else if(id == 5){ 
		for(int i=0;i<4*num_to_produce;i++){
			sleep(128);
			sem_wait(&full); 
			sleep(128);
			sem_wait(&mutex);
			sleep(128);
			printf("Consumer is consuming\n");
			sleep(128);
			sleep(128);
			sem_post(&mutex);
			sleep(128);
			sem_post(&empty);
			sleep(128);
		}
	}
	if(id!=5)exit();
	sem_destroy(&empty);
	sem_destroy(&full);
	sem_destroy(&mutex);
	printf("producer-consumer test finished!\n");
	exit();
	return 0;
}
