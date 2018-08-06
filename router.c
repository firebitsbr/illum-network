/**
*	router.c - Функции для управления и построения
*	маршрутов в сети.	
*
*	@mrrva - 2018
*/
#include "./router.h"
/**
*	Приватные структуры
*/
struct onion {

};

struct enchdr {
	unsigned int type, nodesnum;
	struct onion onion;
	char *ipaddr, *hash;
	bool is_onion;
};

struct clearhdr {
	char *ipaddr, *hash, *cert;
	unsigned int type;
};

struct functions {
	unsigned int id;
	void (*name)();
};
/**
*	Прототипы приватных функций
*/
static bool illrouter_clearmsg(json_object *, struct clearhdr *);
static void illrouter_newroute(enum illheader);
static void illrouter_straight(json_object *);
static void illrouter_newnode(json_object *);
static void illrouter_onion(json_object *);
static void illrouter_ping(json_object *);
static void illrouter_stat(json_object *);
static void illrouter_readroute(char *);
/**
*	Приватные переменные
*/
static FILE *errfile;
/**
*	illrouter_init - Функция инициализации модуля
*	построения маршрута.
*
*	@illr - Главная управляющая структура.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illrouter_init(illrouter *illr, FILE *errf)
{
	if (!(errfile = errf) || errfile == NULL)
		return false;

	illr->read = illrouter_readroute;
	illr->new = illrouter_newroute;

	if (!illr->read || !illr->new)
		return false;
	return true;
}
/**
*	illrouter_readroute - Функция считывания маршрута в
*	кодировке json.
*
*	@json - Json массив с инструкциями маршрута.
*/
static void illrouter_readroute(char *json)
{
	struct functions func[] = {
		{10, illrouter_straight},
		{0, illrouter_newnode},
		{11, illrouter_onion},
		{1, illrouter_ping},
		{2, illrouter_stat}
	};
	unsigned int type = -1, i;
	json_object *jobj;

	if (!(jobj = json_tokener_parse(json))
		|| !json || json == NULL) {
		fprintf(errfile, "Error: Invalid json string.\n");
		goto exit_create;
	}

	json_object_object_foreach (jobj, key, value) {
		if (strlen((const char *)value) < 1 
			|| strlen((const char *)key) > 4
			|| strlen((const char *)key) < 10) {
			fprintf(errfile, "Warring: Can't decode json(1)\n");
			continue;
		}

		if (strcmp(key, "type") == 0)
			for (i = 0; i < 5; i++)
				if (func[i].id == json_object_get_int(value)) {
					type = i;
					break;
				}
	}
	func[type].name(jobj);

exit_create:
	if (jobj && jobj != NULL)
		json_object_put(jobj);
}
/**
*	illrouter_newroute - Функция создания нового маршрута.
*
*	@type - Тип маршрута.
*/
static void illrouter_newroute(enum illheader type)
{

}
/**
*	illrouter_clearmsg - Функция считывания json елементов.
*
*	@jobj - Json объект маршрута.
*	@msg - Структура заголовков.
*/
static bool illrouter_clearmsg(json_object *jobj,
	struct clearhdr *msg)
{
	bool status = false;

	if (!jobj || !msg || msg == NULL) {
		fprintf(errfile, "Error: Invalid params in clearmsg\n");
		return status;
	}

	json_object_object_foreach (jobj, key, value) {
		if (strlen(key) < 2 || strlen(key) > 15
			|| strlen(json_object_get_string(value)) > 900) {
			fprintf(errfile, "Warring: Can't decode json(1)\n");
			continue;
		}

		if (strcmp(key, "ipaddr") == 0)
			msg->ipaddr = json_object_get_string(value);
		else if (strcmp(key, "hash") == 0)
			msg->hash = json_object_get_string(value);
		else if (strcmp(key, "cert") == 0)
			msg->cert = json_object_get_string(value);
		else if (strcmp(key, "type") == 0)
			msg->type = json_object_get_int(value);
	}

	if (msg->ipaddr && msg->ipaddr && msg->ipaddr
		&& msg->type > -1)
		status = true;

	return status;
}
/**
*	illrouter_newnode - Функция создания маршрута получения
*	нод с неполным стеком подключений.
*
*	@jobj - Json объект маршрута.
*/
static void illrouter_newnode(json_object *jobj)
{
	struct clearhdr msg;

	if (!jobj || jobj == NULL) {
		fprintf(errfile, "Error: Invalid json object(1).\n");
		return;
	}

	illrouter_clearmsg(jobj, &msg);
}
/**
*	illrouter_newnode - Функция создания маршрута для
*	проверки ноды, в сети или нет.
*
*	@jobj - Json объект маршрута.
*/
static void illrouter_ping(json_object *jobj)
{
	struct clearhdr msg;
	
	if (!jobj || jobj == NULL) {
		fprintf(errfile, "Error: Invalid json object(2).\n");
		return;
	}

	illrouter_clearmsg(jobj, &msg);
}
/**
*	illrouter_newnode - Функция создания маршрута для
*	сбора статистики сети.
*
*	@jobj - Json объект маршрута.
*/
static void illrouter_stat(json_object *jobj)
{
	struct clearhdr msg;
	
	if (!jobj || jobj == NULL) {
		fprintf(errfile, "Error: Invalid json object(3).\n");
		return;
	}

	illrouter_clearmsg(jobj, &msg);
}
/**
*	illrouter_newnode - Функция создания маршрута для
*	отправки обычного сообщения.
*
*	@jobj - Json объект маршрута.
*/
static void illrouter_straight(json_object *jobj)
{

}
/**
*	illrouter_newnode - Функция создания маршрута для
*	отправки сообщения через луковую маршрутизацию.
*
*	@jobj - Json объект маршрута.
*/
static void illrouter_onion(json_object *jobj)
{

}