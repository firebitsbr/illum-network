/**
*	router.h - Заголовочный файл для router.c с перечнем
*	всех функций и структур.
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
/**
*	Доступные структуры
*/
enum illheader {
	ILL_NEWNODE = 0, ILL_PING = 1, ILL_STAT = 2,
	ILL_STRAIGHT = 10, ILL_ONION = 11
/*
	ILLH_FREENODES = 0, ILLH_UPDNODES = 1,
	ILLH_MESSAGE = 2, ILLH_PING = 3,
	ILLH_ONION = 10, ILLH_STRAIGHT = 11,
	ILL_STAT = 12
*/
};

typedef struct {
	void (*read)(), (*new)();
} illrouter;
/**
*	Доступные функции
*/
bool illrouter_init(illrouter *, FILE *);
#endif
