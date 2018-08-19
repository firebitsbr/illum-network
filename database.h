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
#include <time.h>
#include <sqlite3.h>
#define MAX_TEXTSIZE 9000
/**
*	Доступные структуры
*/
struct node_list {
	char *ipaddr, *hash, *cert;
	unsigned long ping;
	unsigned int id;
};

struct stask {
	char *ipaddr, *cert, *text, *headers;
	unsigned int id;
};

typedef struct {
	bool (*removetask)(), (*newnode)(), (*isset_node)();
	void (*currenttask)(), (*staticnode)(), (*ping)();
	struct node_list *(*nodelist)(), (*nodeinfo)();
	int (*newtask)(), (*nodenum)();
	char *(*setting)();
} illdb;
/**
*	Доступные функции
*/
bool illdb_init(char *, illdb *, FILE *);
void illdb_free(illdb *);
#endif
