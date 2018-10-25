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
#include <pthread.h>
#include "../sources/database.h"
#include "../sources/encryption.h"
#include "../sources/router.h"
#include "../sources/server.h"
#include "../sources/constants.h"
/**
*	Доступные структуры
*/
struct illfunctions {
	struct threads thrd;
	illenc encrypt;
	illrouter router;
	illsrv server;
	char *errfile;
	char *dbpath;
	illdb db;
};

typedef struct {
	bool (*connect)(), (*firstnode)(), (*sendstraight)(),
		(*sendonion)();
	void (*testrequest)();
	struct illfunctions fn;
} illum;
/**
*	Доступные функции
*/
void illum_init(illum *, char *);
#endif