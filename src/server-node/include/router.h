/**
*	router.h - Заголовочный файл децентрализованной
*	сети illum. Здесь опублекованны все константы,
*	прототипы и структуры модуля router.
*
*	@mrrva - 2018
*/
#ifndef ILLUM_ROUTER
#define ILLUM_ROUTER

#include "illum.h"
#define NODESLIM 12
/**
*	Доступные переменные и указатели.
*/
struct illumrouter *p_router;
struct illumencrypt *p_enc;
struct illumnetwork *p_net;
struct illumdb *p_db;
bool routerinit;
FILE *error;
/**
*	Прототипы публичных функций.
*/
unsigned char *illum_response(
	struct illumheaders *,
	struct illumipport
);

unsigned char *illum_rserver(
	struct illumheaders *,
	struct illumipport *
);

unsigned char *illum_rclient(
	struct illumheaders *,
	struct illumipport *
);

unsigned char *illum_ip2bytes(
	char *
);

struct illumheaders *illum_hdecode(
	unsigned char *
);

void illum_nodelist(
	unsigned char *,
	char *
);

void illum_printtemp(
);

#endif