/**
*	router.c - Функции для управления и построения
*	маршрутов в сети.	
*
*	@mrrva - 2018
*/
#include "./illum.h"
#define FUNCNULL 99
/**
*	Приватные структуры.
*/
struct onion {

};

struct headers {
	unsigned int type, nodesnum;
	char *ipaddr, *hash;
	struct onion onion;
	bool is_onion;
};

struct functions {
	unsigned int id;
	void (*name)();
	bool call;
};
/**
*	Приватные переменные и указатели.
*/
static struct illumdb *p_db;
static unsigned int threads;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static bool illum_routerdecode(json_object *, struct headers *);
static void illum_routerstraight(json_object *, struct headers,
	char *, char *);
static void illum_routeronion(json_object *, struct headers,
	char *, char *);
static void illum_routerinfo(json_object *, struct headers,
	char *, char *);
static void illum_routerfail(json_object *, struct headers,
	char *, char *);
static void illum_routerok(json_object *, struct headers,
	char *, char *);
static bool illum_routernewtask(enum illumheader, char *,
	char *);
static char *illum_routerheaders(enum illumheader, char *);
static bool illum_routerread(char *, char *);
static void illum_freepointer(char **, int);
static void *illum_routeprocessing(void *);
static char **illum_routerexplode(char *);
/**
*	illum_routerinit - Функция инициализации модуля
*	построения маршрута.
*
*	@router - Главная управляющая структура.
*	@database - Структура базы данных.
*	@errfile - Файловый стрим для записи ошибок.
*/
bool illum_routerinit(struct illumrouter *router,
	struct illumdb *database, FILE *errfile)
{
	if (!(error = errfile) || !(p_db = database))
		return false;

	router->newtask = illum_routernewtask;
	router->headers = illum_routerheaders;
	router->read = illum_routerread;

	return true;
}
/**
*	illum_routerexplode - Функция разрыва содержимого сообщения.
*
*	@content - Содержимое сообщения
*/
static char **illum_routerexplode(char *content)
{
	char **data, *tok, cont[strlen(content) + 1];

	if (strlen(content) > MAXTEXTSIZE
		|| !strstr(content, "\r\n\r\n")) {
		fprintf(error, "Error: Can't explode content.\n");
		return NULL;
	}

	data = (char **)malloc(sizeof(char *) * 2);
	memcpy(cont, content, strlen(content) + 1);

	if ((tok = strtok(cont, "\r\n\r\n")) == NULL) {
		free(data);
		return NULL;
	}
	data[0] = (char *)malloc(strlen(tok) + 2);
	memcpy(data[0], tok, strlen(tok) + 1);

	if ((tok = strtok(NULL, "\r\n\r\n")) == NULL) {
		data[1] = NULL;
		return data;
	}
	data[1] = (char *)malloc(strlen(tok) + 2);
	memcpy(data[1], tok, strlen(tok) + 1);

	tok = strtok(NULL, "\r\n\r\n");
	return data;
}
/**
*	illum_routernewtask - Функция создания нового маршрута.
*
*	@type - Тип сообщения.
*	@ipaddr - Ip клиента.
*	@text - Текст сообщения.
*/
static bool illum_routernewtask(enum illumheader type,
	char *ipaddr, char *text)
{
	char *jsonheaders;
	int status;

	jsonheaders = illum_routerheaders(type, ipaddr);
	status = p_db->newtask(ipaddr, text, jsonheaders, type);

	return (status > -1) ? true : false;
}
/**
*	illum_routerread - Функция считывания маршрута и создания
*	потока обработчика.
*
*	@data - Полученная информация от другого клиента.
*	@ipaddr - Ip адрес клиента.
*/
static bool illum_routerread(char *data, char *ipaddr)
{
	int length = 0;
	bool status = false;
	pthread_t thread;
	char **args, *tmp;

	if (!data || !ipaddr || strlen(ipaddr) > 100
		|| strlen(data) > MAXTEXTSIZE) {
		fprintf(error, "Error: Incorrect args in routerread.\n");
		return false;
	}
	if (threads >= THREADLIMIT) {
		if (RTEDEBUG)
			fprintf(error, "Warring: Limit of threads.\n");
		return false;
	}

	args = (char **)malloc(sizeof(char *) * 2);
	for (int i = 0; i < 2; i++) {
		tmp = (i == 0) ? ipaddr : data;
		args[i] = (char *)malloc(sizeof(char) * 
				(length = strlen(tmp) + 1));
		memcpy(args[i], tmp, length);
	}
	threads++;

	if (pthread_create(&thread, NULL, illum_routeprocessing
		, (void *)args) != 0)
		goto exit_read;
	status = true;

exit_read:
	if (!status)
		threads--;
	return status;
}
/**
*	illum_routeprocessing - Функция обработки поступивших маршрутов.
*
*	@args - Входящие аргументы (ip и data).
*/
static void *illum_routeprocessing(void *args)
{
	struct functions func[] = {
		{MSTRAIGHT, illum_routerstraight},
		{INFOREQUEST, illum_routerinfo},
		{FAILREQUEST, illum_routerfail},
		{OKREQUEST, illum_routerok},
		{MONION, illum_routeronion}
	};
	char **buffer = (char **)args, **content;
	unsigned int type = FUNCNULL;
	struct headers msg;
	json_object *jobj;

	if (!buffer[0] || !buffer[1]
		|| !strstr(buffer[0], "\r\n\r\n"))
		goto exit_processing;

	content = illum_routerexplode(buffer[0]);
	jobj = json_tokener_parse(content[0]);

	if (!illum_routerdecode(jobj, &msg)) {
		fprintf(error, "Error: Can't decode json.\n");
		goto exit_processing;
	}

	for (int i = 0; i < 6; i++)
		if (func[i].id == msg.type) {
			type = i;
			break;
		}
	if (type != FUNCNULL)
		func[type].name(jobj, msg, buffer[1],
						content[1]);

exit_processing:
	illum_freepointer(content, 2);
	illum_freepointer(buffer, 2);

	if (jobj && jobj != NULL)
		json_object_put(jobj);

	threads--;
	pthread_exit(0);
}
/**
*	illum_routerdecode - Функция считывания json елементов.
*
*	@jobj - Json объект маршрута.
*	@msg - Структура заголовков.
*/
static bool illum_routerdecode(json_object *jobj,
	struct headers *msg)
{
	const char *(*json_string)();
	bool status;

	if (!jobj || !msg) {
		fprintf(error, "Error: Invalid args in decode.\n");
		return false;
	}

	json_string = json_object_get_string;
	msg->is_onion = false;
	status = false;

	json_object_object_foreach (jobj, key, value) {
		if (strlen(key) < 2 || strlen(key) > 15)
			return status;

		if (strcmp(key, "ipaddr") == 0) {
			msg->ipaddr = (char *)json_string(value);
			if (!msg->ipaddr || msg->ipaddr == NULL)
				msg->ipaddr = "";
		}
		else if (strcmp(key, "hash") == 0)
			msg->hash = (char *)json_string(value);
		else if (strcmp(key, "type") == 0)
			msg->type = (int)json_object_get_int(value);
		else if (strcmp(key, "onion") == 0) {
			msg->is_onion = true;
			// Decoding onion struct
		}
	}

	if (msg->ipaddr && msg->hash && msg->type >= 0)
		status = true;
	return status;
}
/**
*	illum_routerheaders - Функция создания новых заголовков 
*	маршрута.
*
*	@type - Тип сообщения.
*	@ipaddr - Ip клиента.
*/
static char *illum_routerheaders(enum illumheader type,
	char *ipaddr)
{
	char *json, *hash, *ip, *tmp;
	json_object *jobj = NULL;
	void (*json_add)();

	ip = (ipaddr && ipaddr != NULL) ? ipaddr : "";
	hash = (char *)malloc(121);
	json = (char *)malloc(600);

	json_add = json_object_object_add;
	jobj = json_object_new_object();
	p_db->getvar("PUBLICKEY", hash);

	json_add(jobj, "ipaddr", json_object_new_string(ip));
	json_add(jobj, "hash", json_object_new_string(hash));
	json_add(jobj, "type", json_object_new_int(type));
	if (type == MONION) {
		// do something for onion message.
	}

	tmp = (char *)json_object_to_json_string(jobj);
	memcpy(json, tmp, strlen(tmp) + 1);

	if (jobj && jobj != NULL)
		json_object_put(jobj);
	free(hash);

	return json;
}
/**
*	illum_routerstraight - Функция создания нового маршрута
*	для отправки прямых сообщений.
*
*	@jobj - Json объект маршрута.
*	@msg - Структура заголовков.
*	@ipaddr - Ip отправившего клиента.
*	@data - Контент сообщения.
*/
static void illum_routerstraight(json_object *jobj,
	struct headers msg, char *ipaddr, char *data)
{
	if (!jobj || !ipaddr) {
		fprintf(error, "Error: Incorrect args in straight.\n");
		return;
	}
}
/**
*	illum_routerinfo - Функция создания нового маршрута
*	для получения информации о ноде.
*
*	@jobj - Json объект маршрута.
*	@msg - Структура заголовков.
*	@ipaddr - Ip отправившего клиента.
*	@data - Контент сообщения.
*/
static void illum_routerinfo(json_object *jobj,
	struct headers msg, char *ipaddr, char *data)
{
	char *ip = ipaddr, *headers;
	struct illumnode *nodes;
	void (*json_add)();
	int length;

	if (!jobj || !ipaddr) {
		fprintf(error, "Error: Incorrect args in info.\n");
		return;
	}

	json_add = json_object_object_add;
	if (!msg.ipaddr || strlen(msg.ipaddr) < 7)
		json_add(jobj, "ipaddr", json_object_new_string(ip));
	else
		ip = msg.ipaddr;

	headers = (char *)json_object_to_json_string(jobj);
	nodes = p_db->nodelist(&length);
	p_db->newnode(ip, msg.hash);

	for (int i = 0; i < length; i++)
		if (strcmp(nodes[i].ipaddr, ip) != 0) {
			p_db->newtask(nodes[i].ipaddr, NULL, headers, msg.type);
			free(nodes[i].ipaddr);
			free(nodes[i].hash);
		}

	free(nodes);
}
/**
*	illum_routerfail - Функция создания нового маршрута
*	для обработки ошибок.
*
*	@jobj - Json объект маршрута.
*	@msg - Структура заголовков.
*	@ipaddr - Ip отправившего клиента.
*	@data - Контент сообщения.
*/
static void illum_routerfail(json_object *jobj,
	struct headers msg, char *ipaddr, char *data)
{
	if (!jobj || !ipaddr) {
		fprintf(error, "Error: Incorrect args in fail.\n");
		return;
	}
}
/**
*	illum_routerok - Функция создания нового маршрута
*	для обработки положительного ответа.
*
*	@jobj - Json объект маршрута.
*	@msg - Структура заголовков.
*	@ipaddr - Ip отправившего клиента.
*	@data - Контент сообщения.
*/
static void illum_routerok(json_object *jobj,
	struct headers msg, char *ipaddr, char *data)
{
	if (!jobj || !ipaddr) {
		fprintf(error, "Error: Incorrect args in ok.\n");
		return;
	}
}
/**
*	illum_routeronion - Функция создания нового маршрута
*	для отправки сообщений через луковую маршрутизацию.
*
*	@jobj - Json объект маршрута.
*	@msg - Структура заголовков.
*	@ipaddr - Ip отправившего клиента.
*	@data - Контент сообщения.
*/
static void illum_routeronion(json_object *jobj,
	struct headers msg, char *ipaddr, char *data)
{
	if (!jobj || !ipaddr) {
		fprintf(error, "Error: Incorrect args in onion.\n");
		return;
	}
}
/**
*	illum_freepointer - Функция освобождение двухуровневого
*	массива.
*
*	@args - Указатель.
*	@length - Длина массива.
*/
static void illum_freepointer(char **pointer, int length)
{
	if (!pointer || length < 1)
		return;

	for (int i = 0; i < length; i++)
		if (pointer[i] && pointer[i] != NULL)
			free(pointer[i]);
	if (pointer)
		free(pointer);
}