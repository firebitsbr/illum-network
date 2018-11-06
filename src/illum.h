/**
*	illum.h - Заголовочный файл децентрализованной
*	сети illum. Здесь опублекованны все константы,
*	прототипы и структуры проекта.
*
*	@mrrva - 2018
*/
#ifndef ILLUM
#define ILLUM

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include <pthread.h>
#include <unistd.h>
#include <sodium.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <json-c/json.h>
/**
*	Константы проекта.
*/
#define MAXNODES		5000
#define MAXTEXTSIZE		9000
#define SERVER_TIMEOUT	6
#define ILLUMPORT		110
#define THREADLIMIT		7
#define ILLUMDEBUG		(bool)true
/**
*	Доступные структуры.
*/
enum illumheader {
	NEWNODE = 0, OKREQUEST = 1, FAILREQUEST = 2,
	MSTRAIGHT = 3, MONION = 4
};

struct illumserver {
	unsigned int thr_number;
	pthread_t resv, send;
	bool (*start)();
};

struct illumdb {
	bool (*issetnode)(), (*newnode)(), (*setvar)(),
		(*gettask)();
	void (*getvar)();
};

struct illumrouter {

};

struct illumencrypt {

};
/**
*	Прототипы доступных функций.
*/
bool illum_serverinit(struct illumserver *, struct illumdb *,
	struct illumrouter *, FILE *);
bool illum_dbinit(struct illumdb *, char *, FILE *);
#endif