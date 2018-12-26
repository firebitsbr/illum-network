/**
*	network.h - Заголовочный файл децентрализованной
*	сети illum. Здесь опублекованны все константы,
*	прототипы и структуры модуля network.
*
*	@mrrva - 2018
*/
#ifndef ILLUM_NETWORK
#define ILLUM_NETWORK

#include "illum.h"
/**
*	Доступные переменные и указатели.
*/
struct illumusers *users;
bool networkinit;
FILE *error;
/**
*	Доступные структуры.
*/
struct thrarr {
	unsigned char buffer[FULLSIZE + 1];
	struct sockaddr_in client;
};
/**
*	Прототипы публичных функций.
*/
void illum_register(
	struct illumusers *,
	unsigned char *,
	struct illumipport
);

void illum_adduser(
	struct illumusers *,
	unsigned char *,
	struct illumipport
);

void illum_removeusers(
	struct illumusers *
);

unsigned char *illum_useract(
	struct illumipport,
	unsigned char *
);

struct thrarr *illum_thrarray(
	void *,
	void *
);

void *illum_replayto(
	void *
);

void *illum_receiver(
);

void *illum_sender(
);

#endif