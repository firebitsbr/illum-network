/**
*	storage.h - Заголовочный файл децентрализованной
*	сети illum. Здесь опублекованны все константы,
*	прототипы и структуры модуля storage.
*
*	@mrrva - 2018
*/
#ifndef ILLUM_STORAGE
#define ILLUM_STORAGE

#include "illum.h"
/**
*	Доступные переменные и указатели.
*/
struct illumencrypt *p_enc;
struct illumnodes *nodes;
bool storageinit;
sqlite3 *db;
FILE *error;
/**
*	Прототипы публичных функций.
*/
bool illum_nodeinsert_list(
	struct illumnodes *
);

void illum_freenode(
	struct illumnodes *, 
	bool
);

void illum_setvar(
	char *,
	char *
);

char *illum_getvar(
	char *
);

bool illum_nodeexists(
	char *
);

bool illum_newnode(
	struct illumnodes *
);

void illum_nodeselect(
);

bool illum_tables(
);

void illum_printnodes(
);

void illum_setlists(
);

#endif