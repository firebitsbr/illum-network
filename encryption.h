/**
*	encryption.h - Заголовочный файл для encryption.c с перечнем
*	всех функций и структур.
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
#include "./database.h"

struct cryptobox_d {
	unsigned int publickey, secretkey;
};

struct userkeys {
	uint8_t public[crypto_box_PUBLICKEYBYTES];
	uint8_t secret[crypto_box_SECRETKEYBYTES];
};

typedef struct {
	struct userkeys keys;
	uint8_t *(*hex2byte)();
	char *(*byte2hex)();
} illenc;

#endif