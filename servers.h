/**
*	servers.h - Заголовочный файл для servers.c с перечнем
*	всех функций и структур.
*
*	@mrrva - 2018
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "./database.h"
#define TEXTSIZE_SEND 2000
#define ILLUM_PORT 110
/**
*	Доступные структуры
*/
typedef struct {
	pthread_t getserver, sendserver;
	bool stop_get, stop_send;
} illsrv;
/**
*	Доступные функции
*/
bool illsrv_init(illsrv *, illdb *, FILE *);