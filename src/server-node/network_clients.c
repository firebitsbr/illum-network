/**
*	network_clients.c - Функции модуля network 
*	децентрализованной сети illum отвечающие
*	за работу приложения с клиентами сети.
*
*	@mrrva - 2018
*/
#include "include/network.h"
/**
*	illum_register - Функция регистрации нового/существующего
*	подключившегося пользователя.
*
*	@list - Список пользователей сети.
*	@hash - Хэш клиента.
*	@ipport - Ip и порт нового клиента.
*	@fp - Файловый стрим ошибок.
*/
void illum_register(struct illumusers *list, 
	unsigned char *hash, struct illumipport ipport)
{
	struct illumusers *tmp = list, *removenode;
	struct illumipport *tmp_p;
	int f_compare, s_compare;
	unsigned char *chash;
	bool exist = false;

	if (!list || list == NULL || !hash
		|| hash == NULL) {
		fprintf(error, "Error: Args of new user.\n");
		return;
	}

	while (tmp->next != NULL) {
		tmp_p = &tmp->next->data;
		chash = tmp->next->hash;

		f_compare = sodium_memcmp(chash, hash, 32);
		s_compare = strcmp(tmp_p->ip, ipport.ip);

		if ((f_compare == 0 && s_compare != 0)
			|| (f_compare != 0 && s_compare == 0)) {
			removenode = tmp->next;
			tmp->next = tmp->next->next;
			free(removenode);
			tmp = tmp->next;
			continue;
		}
		if (f_compare + s_compare == 0 && !exist) {
			tmp->next->ping = time(NULL);
			exist = true;
		}

		tmp = tmp->next;
	}

	if (exist)
		return;

	if (sodium_memcmp(tmp->hash, hash, 32) == 0) {
		if (strcmp(tmp->data.ip, ipport.ip) == 0) {
			tmp->ping = time(NULL);
			return;
		}
		removenode = tmp->next;
		tmp = tmp->next;
		free(removenode);
	}

	illum_adduser(list, hash, ipport);
}
/**
*	illum_adduser - Функция добавления пользователя сети
*	в общий список.
*
*	@list - Список пользователей сети.
*	@hash - Хэш клиента.
*	@ipport - Ip и порт нового клиента.
*/
void illum_adduser(struct illumusers *list,
	unsigned char *hash, struct illumipport ipport)
{
	struct illumusers *tmp;
	size_t f_size, s_size;

	if (!list || !hash)
		return;

	s_size = sizeof(struct illumipport);
	f_size = sizeof(struct illumusers);
	tmp = (struct illumusers *)malloc(f_size);
	tmp->next = list;
	list = tmp;

	memcpy(&tmp->data, &ipport, s_size);
	memcpy(tmp->hash, hash, HASHSIZE);
}
/**
*	illum_removeusers - Функция определения пользователей,
*	которые подключены к ноде.
*
*	@list - Список пользователей сети.
*/
void illum_removeusers(struct illumusers *list)
{
	struct illumusers *tmp = list, *removenode;
	time_t c_time = time(NULL);

	if (!list || list == NULL)
		return;

	while (tmp->next != NULL) {
		if (c_time - tmp->next->ping >= TIMEOUT) {
			removenode = tmp->next;
			tmp->next = tmp->next->next;
			free(removenode);
		}
		tmp = tmp->next;
	}

	if (c_time - list->ping >= TIMEOUT) {
		removenode = list;
		list = list->next;
		free(removenode);
	}
}