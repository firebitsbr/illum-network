/**
*	encryption.h - Заголовочный файл для encryption.c с перечнем
*	всех функций и структур.
*
*	Пример инициализации модуля:
*	illdb db;
*	illenc enc;
*	illenc_init(&enc, &db, fp);
*
*	@mrrva - 2018
*/
#ifndef ILL_ENCRYPTION
#define ILL_ENCRYPTION

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sodium.h>
#include "./constants.h"
#include "./database.h"
/**
*	Доступные структуры
*/
struct userkeys {
	unsigned char public[crypto_box_PUBLICKEYBYTES];
	unsigned char secret[crypto_box_SECRETKEYBYTES];
};

typedef struct {
	unsigned char *(*hex2byte)(), *(*encrypt)(),
		*(*decrypt)();
	char *(*byte2hex)(), *(*publickey)();
	struct userkeys keys;
} illenc;
/**
*	Доступные функции
*/
bool illenc_init(illenc *, illdb *, FILE *);
#endif