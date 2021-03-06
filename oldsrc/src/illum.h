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
#define MAXNODES		500
#define MAXTEXTSIZE		9000
#define SERVER_TIMEOUT	6
#define ILLUMPORT		110
#define THREADLIMIT		7
#define DBDEBUG			(bool)true
#define SRVDEBUG		(bool)true
#define RTEDEBUG		(bool)true
#define ENCDEBUG		(bool)true
/**
*	Доступные структуры.
*/
enum illumheader {
	INFOREQUEST = 0, OKREQUEST = 1, FAILREQUEST = 2,
	FINDNODE = 3, MSTRAIGHT = 4, MONION = 5
};

struct userkeys {
	unsigned char public[crypto_box_PUBLICKEYBYTES];
	unsigned char secret[crypto_box_SECRETKEYBYTES];
};

struct illumtask {
	char *ipaddr, *text, *headers;
	unsigned int id;
	int type;
};

struct illumnode {
	char *ipaddr, *hash;
	long use_t;
	int id;
};

struct illummsg {
	char *ipaddr, *hash, *text;
	enum illumheader type;
	struct illummsg *next;
};

struct illumserver {
	pthread_t resv, send;
	bool (*start)();
};

struct illumdb {
	bool (*issetnode)(), (*newnode)(), (*setvar)(),
		(*gettask)();
	struct illumnode *(*nodelist)();
	int (*newtask)(), (*nodenum)();
	void (*getvar)();
};

struct illumrouter {
	bool (*newtask)(), (*read)(), (*is_ok)();
	char *(*headers)();
};

struct illumencrypt {
	unsigned char *(*hex2byte)(), *(*decrypt)(),
		*(*encrypt)();
	char *(*byte2hex)();
};

struct illummsg *msgbuffer;
/**
*	Прототипы доступных функций.
*/
bool illum_serverinit(struct illumserver *, struct illumdb *,
	struct illumrouter *, FILE *);
bool illum_encryptinit(struct illumencrypt *, struct illumdb *,
	FILE *);
bool illum_routerinit(struct illumrouter *, struct illumdb *,
	FILE *);
bool illum_dbinit(struct illumdb *, char *, FILE *);
#endif