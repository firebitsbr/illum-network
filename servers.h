/**
*	servers.h - Заголовочный файл для servers.c с перечнем
*	всех функций и структур.
*
*	@mrrva - 2018
*/
#ifndef ILL_SERVERS
#define ILL_SERVERS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "./database.h"
#include "./router.h"
#define TEXTSIZE_BUFER 2000
#define MAXTEXTSIZE 9000
#define SERVER_TIMEOUT 10
#define ILLUM_PORT 110
#define THREAD_LIMIT 5
/**
*	Доступные структуры
*/
struct threads {
	pthread_t server, client;
};

typedef struct {
	void (*setnode)();
	struct threads (*start)();
} illsrv;
/**
*	Доступные функции
*/
bool illsrv_init(illsrv *, illdb *, illroute *, FILE *);
#endif
