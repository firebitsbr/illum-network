/**
*	router.c - Функции модуля router 
*	децентрализованной сети illum.
*
*	@mrrva - 2018
*/
#include "./include/router.h"
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
	struct illumdb *database,
	FILE *fp)
{
	if (routerinit || !router || !network || !fp
		|| !encrypt || !database) {
		fprintf(fp, "Error: Incorrect args int router.\n");
		return false;
	}

	p_router = router;
	p_net = network;
	p_enc = encrypt;
	p_db = database;
	error = fp;

	router->response = illum_response;
	router->h_decode = illum_hdecode;

	memset(router->template, '\0', HEADERSIZE);
	memcpy(router->template + 1, p_enc->keys->public,
		HASHSIZE);
	
#ifdef DEBUG
	illum_printtemp();
#endif
	return (routerinit = true);
}
/**
*	illum_response - Функция генерации ответа на
*	запрос устройства в сети.
*
*	@headers - Структура заголовков сообщения.
*	@ipport - Структура Ip адреса и порта клиента.
*/
unsigned char *illum_response(struct illumheaders *headers,
	struct illumipport ipport)
{
	unsigned char *response;

	if (!headers || headers == NULL) {
		fprintf(error, "Error: Invalid args in response.\n");
		return NULL;
	}
	response = (headers->is_node) ? illum_rserver(headers, &ipport)
		: illum_rclient(headers, &ipport);
	return response;
}
/**
*	illum_rserver - Функция генерации ответа на
*	запрос сервера в сети.
*
*	@headers - Структура заголовков сообщения.
*	@ipport - Структура Ip адреса и порта клиента.
*/
unsigned char *illum_rserver(struct illumheaders *headers,
	struct illumipport *ipport)
{
	unsigned char *response;

	if (!headers || !ipport)
		return NULL;

	response = (unsigned char *)malloc(HEADERSIZE);

	switch (headers->type) {
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
			memcpy(response, p_router->template, HEADERSIZE);
			response[0] = S_RESPONSE_DOS;
	}
	return response;
}
/**
*	illum_rclient - Функция генерации ответа на
*	запрос клиента в сети.
*
*	@headers - Структура заголовков сообщения.
*	@ipport - Структура Ip адреса и порта клиента.
*/
unsigned char *illum_rclient(struct illumheaders *headers,
	struct illumipport *ipport)
{
	unsigned char *response;

	if (!headers || !ipport)
		return NULL;

	response = (unsigned char *)malloc(HEADERSIZE);

	switch (headers->type) {
		case U_RESPONSE_NODES:
			illum_nodelist(response, ipport->ip);
			break;
		case U_RESPONSE_PING:
			memcpy(response, p_router->template, HEADERSIZE);
			response[0] = U_RESPONSE_PING;
			break;
		case U_RESPONSE_ONION:
			printf("4H.\n");
			break;
		default:
			memcpy(response, p_router->template, HEADERSIZE);
			response[0] = U_RESPONSE_DOS;
	}
	return response;
}
/**
*	illum_nodelist - Функция создания ответа на
*	сообщение о запросе новых нод.
*
*	@response - Указатель на ответ клиенту.
*	@ipaddr - Ip адрес устройства.
*/
void illum_nodelist(unsigned char *response, char *ipaddr)
{
	size_t size = 1 + HASHSIZE;
	struct illumnodes *nodes;
	unsigned char *b_ipaddr;
	int len = 0;

	if (!response || !ipaddr) {
		fprintf(error, "Error: Invalid args in nodel"
			"ist.\n");
		if (response && response != NULL)
			free(response);
		response = NULL;
		return;
	}

	memcpy(response, p_router->template, HEADERSIZE);
	response[0] = U_RESPONSE_NODES;
	nodes = p_db->p_nodes();

	if (nodes == NULL) {
		response[0] = U_RESPONSE_DOS;
		return;
	}
	while (len++, nodes != NULL && len <= NODESLIM) {
		b_ipaddr = illum_ip2bytes(nodes->ip);

		if (b_ipaddr == NULL) {
			nodes = nodes->next;
			continue;
		}
		memcpy(response + size + 4, nodes->hash, 32);
		memcpy(response + size, b_ipaddr, 4);

		nodes = nodes->next;
		size += 4 + HASHSIZE;
		free(b_ipaddr);
	}
}
/**
*	illum_ip2bytes - Функция перевода строки ip
*	адреса в байтовый массив.
*
*	@ipaddr - Ip адрес.
*/
unsigned char *illum_ip2bytes(char *ipaddr)
{
	unsigned char *bytes;
	int dt[4];

	if (!ipaddr || ipaddr == NULL || !strstr(ipaddr, "."))
		return NULL;
	bytes = (unsigned char *)malloc(4);

	sscanf(ipaddr, "%d.%d.%d.%d", &dt[0], &dt[1],
		&dt[2], &dt[3]);
	for (int i = 0; i < 4; i++)
		bytes[i] = dt[i];

	return bytes;
}
/**
*	illum_hdedoce - Функция декодирования заголовков
*	сообщения.
*
*	@headers - Байтовый массив сообщения.
*/
struct illumheaders *illum_hdecode(unsigned char *headers)
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
	st_hdrs->type = (headers[0] == 0x00 || headers[0] == 0x10)
		? headers[0] : headers[0] << TYPESHIFT;
	st_hdrs->is_node = (headers[0] >= 0x10) ? true : false;

	return st_hdrs;
}
/**
*	illum_printtemp - Функция отображения шаблона
*	для сообщений.
*
*/
void illum_printtemp(void)
{
	printf("Message template: ");

	for (int i = 0; i < HEADERSIZE; i++)
		printf("0x%02x  ", p_router->template[i]);
	printf("\n");
}
