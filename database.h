/**
*	database.h - Заголовочный файл для database.c с перечнем
*	всех функций и структур.
*
*	Пример инициализации модуля:
*	illdb *db;
*	illdb_init("/path/to/database/illum.db", &db, file_stream);
*
*	Пример вызова функций:
*	db.settask( some params.. ); 
*
*	@mrrva - 2018
*/
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

};

typedef struct {
	sqlite3 *db;
	struct node_list *(*nodelist)();
	bool (*removetask)();
	bool (*newnode)();
	int (*settask)();
} illdb;
/**
*	Доступные функции
*/
bool illdb_init(char *, illdb *, FILE *);