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
static struct illumencrypt *p_enc;
static struct illumrouter *p_router;
static struct illumnetwork *p_net;
static bool routerinit = false;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static unsigned char *illum_responce(struct illumheaders *, struct illumipport);
static unsigned char *illum_rserver(struct illumheaders *, struct illumipport *);
static unsigned char *illum_rclient(struct illumheaders *, struct illumipport *);
static struct illumheaders *illum_hdedoce(unsigned char *, int);
static void illum_printtemp();
/**
*	illum_router - Функция инициализации модуля
*	маршрутизации сети.
*
*	@router - Указатель на управляющую структуру.
*	@network - Указатель на структуру network.
*	@fp - Указатель на файловый стрим лог файла.
*/
bool illum_router(struct illumrouter *router,
	struct illumnetwork *network,
	struct illumencrypt *encrypt,
	FILE *fp)
{
	if (routerinit || !router || !network || !fp
		|| !encrypt) {
		fprintf(fp, "Error: Incorrect args int router.\n");
		return false;
	}

	p_router = router;
	p_net = network;
	p_enc = encrypt;
	error = fp;

	router->responce = illum_responce;
	router->h_decode = illum_hdedoce;

	memset(router->template, '\0', HEADERSIZE);
	memcpy(router->template + 1, p_enc->keys->public,
		HASHSIZE);
	
#ifdef DEBUG
	illum_printtemp();
#endif
	return (routerinit = true);
}
/**
*	illum_responce - Функция генерации ответа на
*	запрос устройства в сети.
*
*	@headers - Структура заголовков сообщения.
*	@ipport - Структура Ip адреса и порта клиента.
*/
static unsigned char *illum_responce(struct illumheaders *headers,
	struct illumipport ipport)
{
	unsigned char *responce;

	if (!headers || headers == NULL) {
		fprintf(error, "Error: Invalid args in responce.\n");
		return NULL;
	}

	responce = (headers->is_node) ? illum_rserver(headers, &ipport)
		: illum_rclient(headers, &ipport);
	return responce;
}
/**
*	illum_rserver - Функция генерации ответа на
*	запрос сервера в сети.
*
*	@headers - Структура заголовков сообщения.
*	@ipport - Структура Ip адреса и порта клиента.
*/
static unsigned char *illum_rserver(struct illumheaders *headers,
	struct illumipport *ipport)
{
	unsigned char *responce;

	switch (headers->type) {
		case S_RESPONSE_DOS:
		/* Неверные заголовочники - удалить из списка */
			printf("1H.\n");
			break;
		case S_RESPONSE_NODES:
			printf("2H.\n");
			break;
		case S_RESPONSE_CLIENTS:
			printf("3H.\n");
			break;
		case S_RESPONSE_FIND:
			printf("4H.\n");
			break;
		case S_RESPONSE_ONION1:
			printf("5H.\n");
			break;
		case S_RESPONSE_ONION2:
			printf("6H.\n");
			break;
		case S_RESPONSE_ONION3:
			printf("7H.\n");
			break;
		default:
			p_router->template[0] = S_RESPONSE_DOS;
			responce = (unsigned char *)malloc(HEADERSIZE);
			memcpy(responce, p_router->template, HEADERSIZE);
	}
	return responce;
}
/**
*	illum_rclient - Функция генерации ответа на
*	запрос клиента в сети.
*
*	@headers - Структура заголовков сообщения.
*	@ipport - Структура Ip адреса и порта клиента.
*/
static unsigned char *illum_rclient(struct illumheaders *headers,
	struct illumipport *ipport)
{
	unsigned char *responce;

	switch (headers->type) {
		case U_RESPONSE_NODES:
			printf("2H.\n");
			break;
		case U_RESPONSE_PING:
			printf("3H.\n");
			break;
		case U_RESPONSE_ONION:
			printf("4H.\n");
			break;
		default:
			p_router->template[0] = U_RESPONSE_DOS;
			responce = (unsigned char *)malloc(HEADERSIZE);
			memcpy(responce, p_router->template, HEADERSIZE);
	}
	return responce;
}
/**
*	illum_hdedoce - Функция декодирования заголовков
*	сообщения.
*
*	@headers - Байтовый массив сообщения.
*/
static struct illumheaders *illum_hdedoce(unsigned char *headers,
	int port)
{
	size_t size = sizeof(struct illumheaders);
	struct illumheaders *st_hdrs;

	if (!headers)
		return NULL;

	st_hdrs = (struct illumheaders *)malloc(size);
	if (!st_hdrs)
		return NULL;

	memcpy(st_hdrs->info, headers + HASHSIZE + 1, INFOSIZE);
	memcpy(st_hdrs->hash, headers + 1, HASHSIZE);
	st_hdrs->type = headers[0] << TYPESHIFT;
	st_hdrs->is_node = (port == ILLUMPORT) ? true : false;

	return st_hdrs;
}
/**
*	illum_printtemp - Функция отображения шаблона
*	для сообщений.
*
*/
static void illum_printtemp()
{
	printf("Message template: ");

	for (int i = 0; i < HEADERSIZE; i++)
		printf("0x%x ", p_router->template[i]);
	printf("\n");
}
