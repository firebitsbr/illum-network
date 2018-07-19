/**
*	database.h - Заголовочный файл для database.c с перечнем
*	всех функций и структур.
*
*	Пример инициализации модуля:
*	illdb db;
*	illdb_init("/path/to/database/illum.db", &db, file_stream);
*
*	Пример вызова функций:
*	db.settask( some params.. ); 
*
*	@mrrva - 2018
*/
#ifndef ILL_DATABASE
#define ILL_DATABASE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#define MAX_TEXTSIZE 1000
/**
*	Доступные структуры
*/
struct node_list {
	unsigned int id, mseconds;
	char *ipaddr, *hash, *about, *cert;
};

struct stask {
	unsigned int id;
	char *ipaddr, *cert, *text;
};

typedef struct {
	bool (*removetask)(), (*newnode)();
	struct node_list *(*nodelist)();
	void (*currenttask)();
	char *(*setting)();
	int (*newtask)();
} illdb;
/**
*	Доступные функции
*/
bool illdb_init(char *, illdb *, FILE *);
void illdb_free(illdb *);
#endif
