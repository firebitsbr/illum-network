#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void *thread1()
{
	for (int i = 0; i < 20; i++) {
		printf("Line is %d\n", i);
		sleep(8);
	}

	pthread_exit(0);
}

void processing()
{
	pthread_t thread;

	pthread_create(&thread, NULL, thread1, NULL);
}

int main() 
{
	processing();
	processing();
	processing();
	processing();
	processing();

	getchar();
	getchar();
	getchar();
	getchar();
}