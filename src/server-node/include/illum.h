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
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/**
*	Константы проекта.
*/
#define ILLUMPORT		110
#define TIMEOUT			6
#define TEXTSIZE		9000
#define HEADERSIZE		500
#define THREADS			0
#define TYPESHIFT		1
#define HASHSIZE		32
#define FULLSIZE		TEXTSIZE + HEADERSIZE
#define INFOSIZE		HEADERSIZE - HASHSIZE - 1
#define DEBUG			1
/**
*	Доступные структуры.
*/
enum illumresponse {
	/**
	*	Клиентские типы сообщений.
	*/
	U_RESPONSE_DOS		= 0x00,
	U_RESPONSE_NODES	= 0x02,
	U_RESPONSE_PING		= 0x06,
	U_RESPONSE_ONION	= 0x08,
	/**
	*	Серверные типы сообщений.
	*/
	S_RESPONSE_DOS		= 0x10,
	S_RESPONSE_NODES	= 0x12,
	S_RESPONSE_CLIENTS	= 0x16,
	S_RESPONSE_FIND		= 0x18,
	S_RESPONSE_ONION1	= 0x1a,
	S_RESPONSE_ONION2	= 0x1c,
	S_RESPONSE_ONION3	= 0x1e
};

struct illumkeys {
	unsigned char public[HASHSIZE];
	unsigned char secret[HASHSIZE];
};

struct illumheaders {
	unsigned char hash[HASHSIZE], info[INFOSIZE];
	enum illumresponse type;
	bool is_node;
};

struct illumipport {
	char ip[20];
	int port;
};

struct illumusers {
	unsigned char hash[HASHSIZE];
	struct illumipport data;
	struct illumusers *next;
	time_t ping;
};

struct illumnodes {
	unsigned char hash[HASHSIZE];
	struct illumnodes *next;
	unsigned long use_t;
	char ip[20];
};

struct illumtasks {
	unsigned char text[FULLSIZE];
	struct illumtasks *next;
	char ip[20];
};

struct illumnetwork {
	bool (*action)(), exit_server;
	pthread_t receiver, sender;
};

struct illumrouter {
	unsigned char template[HEADERSIZE];
	struct illumheaders *(*h_decode)();
	unsigned char *(*response)();
};

struct illumencrypt {
	unsigned char *(*hex2byte)(), *(*decrypt)(),
		*(*encrypt)();
	char *(*byte2hex)();
	struct illumkeys *keys;
};

struct illumdb {
	void (*set)(), (*deletenode)(), (*nodelist)(),
		(*freenode)(), (*printtasks)(),
		(*setlists)(), (*removetask)();
	char *(*get)(), *(*findnode)();
	struct illumtasks *(*p_tasks)();
	struct illumnodes *(*p_nodes)();
	bool (*newnode)(), (*newtask)();
};
/**
*	Прототипы публичных функций.
*/
bool illum_network(
	struct illumnetwork *,
	struct illumrouter *,
	FILE *
);

bool illum_router(
	struct illumrouter *,
	struct illumnetwork *,
	struct illumencrypt *,
	struct illumdb *,
	FILE *
);

bool illum_database(
	struct illumdb *,
	struct illumencrypt *,
	char *,
	FILE *
);

bool illum_encryptinit(
	struct illumencrypt *,
	struct illumdb *,
	FILE *
);

#endif