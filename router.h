/**
*	router.h - Заголовочный файл для router.c с перечнем
*	всех функций и структур.
*
*	Пример инициализации модуля:
*	illdb db;
*	illroute router;
*	illrouter_init(&router, &db, file_stream);
*
*	@mrrva - 2018
*/
#ifndef ILL_ROUTER
#define ILL_ROUTER

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <json-c/json.h>
#include <time.h>
#include "./database.h"
#define MAXNODES 180
#define UPDTIME 10000
/**
*	Доступные структуры
*/
enum illheader {
	ILL_NEWNODE = 0, ILL_PING = 1, ILL_STAT = 2,
	ILL_BEFRIENDS = 3, ILL_STRAIGHT = 10, ILL_ONION = 11
/*
	ILL_NEWNODE и ILL_STRAIGHT единственные доступные
	типы сообщений без статического соединения

	ILLH_FREENODES = 0, ILLH_UPDNODES = 1,
	ILLH_MESSAGE = 2, ILLH_PING = 3,
	ILLH_ONION = 10, ILLH_STRAIGHT = 11,
	ILL_STAT = 12
*/
};

typedef struct {
	void (*read)(), (*new)(), (*updnodes)(bool);
} illrouter;
/**
*	Доступные функции
*/
bool illrouter_init(illrouter *, illdb *, FILE *);
#endif
