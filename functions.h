/**
*	functions.h - Заголовочный файл для functions.c с перечнем
*	всех функций и структур.
*
*	@mrrva - 2018
*/
#ifndef ILL_FUNCTIONS
#define ILL_FUNCTIONS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../sources/database.h"
#include "../sources/encryption.h"
#include "../sources/router.h"
#include "../sources/server.h"
#include "../sources/constants.h"
/**
*	Доступные структуры
*/
struct illfunc {
	illenc encrypt;
	illrouter router;
	illsrv server;
	char *errfile;
	char *dbpath;
	illdb db;
};

typedef struct {
	bool (*firstnode)(), (*message)()
		(*onionmessage)();
	unsigned int (*connect)();
	void (*start)();
} illum;
/**
*	Доступные функции
*/
bool illum_init(illum *, char *);
#endif