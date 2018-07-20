/**
*	servers.h - Заголовочный файл для servers.c с перечнем
*	всех функций и структур.
*
*	Пример инициализации модуля:
*	illdb db;
*	illsrv srv;
*	illroute router;
*	illsrv_init(&srv, &db, &router, file_stream);
*
*	Пример вызова функций:
*	srv.start( some params.. );
*
*	@mrrva - 2018
*/
#ifndef ILL_SERVERS
#define ILL_SERVERS

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "./database.h"
#include "./router.h"
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
