/**
*	router.c - Функции модуля router 
*	децентрализованной сети illum.
*
*	@mrrva - 2018
*/
#include "./illum.h"
/**
*	Структуры модуля.
*/

/**
*	Приватные переменные и указатели.
*/
static struct illumrouter *p_router;
static struct illumnetwork *p_net;
static bool routerinit = false;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static unsigned char *illum_responce(struct illumheaders *, 
	struct illumipport);
static struct illumheaders *illum_hdedoce(unsigned char *);
/**
*	illum_router - Функция инициализации модуля
*	маршрутизации сети.
*
*	@router - Указатель на управляющую структуру.
*	@network - Указатель на структуру network.
*	@fp - Указатель на файловый стрим лог файла.
*/
bool illum_router(struct illumrouter *router,
	struct illumnetwork *network, FILE *fp)
{
	if (routerinit || !router || !network || !fp) {
		fprintf(fp, "Error: Incorrect args int router.\n");
		return false;
	}

	p_router = router;
	p_net = network;
	error = fp;

	router->responce = illum_responce;
	router->h_decode = illum_hdedoce;

	router->template[0] = 0x00;
	

	return (routerinit = true);
}
/**
*	illum_responce - Функция генерации ответа на
*	запрос клиента в сети.
*
*	@headers - Структура заголовков сообщения.
*	@ipport - Структура Ip адреса и порта клиента.
*/
static unsigned char *illum_responce(struct illumheaders *headers,
	struct illumipport ipport)
{
	return NULL;
}
/**
*	illum_hdedoce - Функция декодирования заголовков
*	сообщения.
*
*	@headers - Байтовый массив сообщения.
*/
static struct illumheaders *illum_hdedoce(unsigned char *headers)
{
	size_t size = sizeof(struct illumheaders);
	struct illumheaders *st_hdrs;

	if (!headers || strlen(headers) < HEADERSIZE
		|| strlen(header) > FULLSIZE)
		return st_hdrs;

	st_hdrs = (struct illumheaders *)malloc(size);
	if (!st_hdrs)
		return st_hdrs;

	memcpy(st_hdrs->info, headers + HASHSIZE + 1, INFOSIZE);
	memcpy(st_hdrs->hash, headers + 1, HASHSIZE);
	st_hdrs->type = headers[0] << TYPESHIFT;

	return st_hdrs;
}