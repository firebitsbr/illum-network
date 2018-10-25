/**
*	server.h - Заголовочный файл для servers.c с перечнем
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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "./constants.h"
#include "./database.h"
#include "./router.h"
/**
*	Доступные структуры
*/
struct threads {
	pthread_t server, client;
};

typedef struct {
	struct threads (*start)();
} illsrv;
/**
*	Доступные функции
*/
bool illsrv_init(illsrv *, illdb *, illrouter *, FILE *);
#endif