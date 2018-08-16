/**
*	router.c - Функции для управления и построения
*	маршрутов в сети.	
*
*	@mrrva - 2018
*/
#include "./router.h"
#define FUNCNULL 99
/**
*	Приватные структуры
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
};
/**
*	Прототипы приватных функций
*/
/*static bool illrouter_mkclearhdr(struct node_list *, unsigned int,
	char *, char *, enum illheader);*/
static bool illrouter_decode(json_object *, struct headers *);
static void illrouter_newroute(enum illheader, char *);
static void illrouter_befriends(json_object *, char *);
static void illrouter_straight(json_object *, char *);
static void illrouter_newnode(json_object *, char *);
static void illrouter_onion(json_object *, char *);
static void illrouter_ping(json_object *, char *);
static void illrouter_stat(json_object *, char *);
static void illrouter_readroute(char *, char *);
/**
*	Приватные переменные
*/
static FILE *errfile;
static illdb *db;
/**
*	illrouter_init - Функция инициализации модуля
*	построения маршрута.
*
*	@illr - Главная управляющая структура.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illrouter_init(illrouter *illr, illdb *database, FILE *errf)
{
	if (!(errfile = errf) || !(db = database) || errfile == NULL)
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
static void illrouter_readroute(char *json, char *ipaddr)
{
	struct functions func[] = {
		{ILL_BEFRIENDS, illrouter_befriends},
		{ILL_STRAIGHT, illrouter_straight},
		{ILL_NEWNODE, illrouter_newnode},
		{ILL_ONION, illrouter_onion},
		{ILL_PING, illrouter_ping},
		{ILL_STAT, illrouter_stat},
	};
	unsigned int type = FUNCNULL, i;
	json_object *jobj;

	if (!(jobj = json_tokener_parse(json)) || !ipaddr
		|| ipaddr == NULL || strlen(ipaddr) < 7) {
		fprintf(errfile, "Error: Invalid json string or ip.\n");
		goto exit_create;
	}

	json_object_object_foreach (jobj, key, value) {
		if (strlen((const char *)value) < 1 
			|| strlen((const char *)key) > 10
			|| strlen((const char *)key) < 4) {
			fprintf(errfile, "Error: Can't decode json(1).\n");
			goto exit_create;
		}

		if (strcmp(key, "type") == 0)
			for (i = 0; i < 5; i++)
				if (func[i].id == json_object_get_int(value)) {
					type = i;
					break;
				}
	}

	if (type != FUNCNULL && func[type].name
		&& func[type].name != NULL)
		func[type].name(jobj, ipaddr);

exit_create:
	if (jobj && jobj != NULL)
		json_object_put(jobj);
}
/**
*	illrouter_newroute - Функция создания нового маршрута.
*
*	@type - Тип маршрута.
*	@ipaddr - Адрес для отправки.
*/
static void illrouter_newroute(enum illheader type, char *ipaddr)
{
	unsigned int nodenum = 0;
	json_object *jobj;
	char *json, *hash;

	if (!ipaddr || ipaddr == NULL || strlen(ipaddr) < 7) {
		fprintf(errfile, "Error: Incorrect ipaddr in newroute.\n");
		return;
	}

	jobj = json_object_new_object();
	nodenum = db->nodenum(errfile);
	hash = "hghghghghghghghg";

	json_object_object_add(jobj, "nodenum", json_object_new_int(nodenum));
	json_object_object_add(jobj, "ipaddr", json_object_new_string(""));
	json_object_object_add(jobj, "hash", json_object_new_string(hash));
	json_object_object_add(jobj, "type", json_object_new_int(type));

	if (type == ILL_ONION) {

	}

	json = (char *)json_object_to_json_string(jobj);
	db->newtask(ipaddr, NULL, json, errfile);
	json_object_put(jobj);
}
/**
*	illrouter_decode - Функция считывания json елементов.
*
*	@jobj - Json объект маршрута.
*	@msg - Структура заголовков.
*/
static bool illrouter_decode(json_object *jobj, struct headers *msg)
{
	bool status = false;

	if (!jobj || !msg || msg == NULL) {
		fprintf(errfile, "Error: Invalid params in clearmsg.\n");
		return status;
	}

	msg->is_onion = false;

	json_object_object_foreach (jobj, key, value) {
		if (strlen(key) < 2 || strlen(key) > 15) {
			fprintf(errfile, "Warring: Can't decode json(2).\n");
			return status;
		}

		if (strcmp(key, "ipaddr") == 0) {
			msg->ipaddr = (char *)json_object_get_string(value);
			if (!msg->ipaddr || msg->ipaddr == NULL)
				msg->ipaddr = "";
		}
		else if (strcmp(key, "nodesnum") == 0)
			msg->nodesnum = (int)json_object_get_int(value);
		else if (strcmp(key, "hash") == 0)
			msg->hash = (char *)json_object_get_string(value);
		else if (strcmp(key, "type") == 0)
			msg->type = (int)json_object_get_int(value);
		else if (strcmp(key, "onion") == 0) {
			msg->is_onion = true;
			// Decoding onion struct
		}
	}

	if (msg->ipaddr && msg->hash && msg->type >= 0
		&& msg->nodesnum >= 0)
		status = true;

	return status;
}
/**
*	illrouter_newnode - Функция создания маршрута получения
*	нод с неполным стеком подключений.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_newnode(json_object *jobj, char *ipaddr)
{
	struct node_list *list;
	char *headers, *ipc;
	struct headers msg;
	unsigned int len;

	if (!jobj || jobj == NULL || !ipaddr || ipaddr == NULL) {
		fprintf(errfile, "Warring: Invalid json object(1).\n");
		return;
	}
	if (!illrouter_decode(jobj, &msg)) {
		fprintf(errfile, "Warring: Can't decode json(3).\n");
		return;
	}

	list = db->nodelist(&len, errfile);
	if (len < MAXNODES) {
		ipc = (msg.ipaddr && strlen(msg.ipaddr) >= 7)
			? msg.ipaddr : ipaddr;

		if (!db->isset_node(ipc, msg.hash, 2, errfile)) {
			db->newnode(ipc, msg.hash, 10, errfile);
			illrouter_newroute(ILL_BEFRIENDS, ipc);
		}
	}

	if (!msg.ipaddr || msg.ipaddr == NULL || strlen(msg.ipaddr) < 7)
		json_object_object_add(jobj, "ipaddr", 
								json_object_new_string(ipaddr));

	if (!(headers = (char *)json_object_to_json_string(jobj))) {
		fprintf(errfile, "Error: Invalid json string in "
				"resend.\n");
		goto exit_newnode;
	}

	if (!list || list == NULL)
		goto exit_newnode;

	for (int i = 0; i < len; i++)
		if (list[i].ipaddr && strlen(list[i].ipaddr) >= 7
			&& list[i].ipaddr != ipaddr) {
			db->newtask(list[i].ipaddr, NULL, headers, errfile);
			free(list[i].ipaddr);
			free(list[i].hash);
		}

exit_newnode:
	if (headers && headers != NULL)
		free(headers);
	if (list && list != NULL)
		free(list);
}
/**
*	illrouter_newnode - Функция создания маршрута для
*	проверки ноды, в сети или нет.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_ping(json_object *jobj, char *ipaddr)
{
	struct headers msg;
	
	if (!jobj || jobj == NULL) {
		fprintf(errfile, "Error: Invalid json object(2).\n");
		return;
	}

	illrouter_decode(jobj, &msg);
}
/**
*	illrouter_newnode - Функция создания маршрута для
*	сбора статистики сети.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_stat(json_object *jobj, char *ipaddr)
{
	struct headers msg;
	
	if (!jobj || jobj == NULL) {
		fprintf(errfile, "Error: Invalid json object(3).\n");
		return;
	}

	illrouter_decode(jobj, &msg);
}
/**
*	illrouter_befriends - Функция создания маршрута для
*	подтверждения статической связи между нодами.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_befriends(json_object *jobj, char *ipaddr)
{
	struct headers msg;

	if (!jobj || jobj == NULL || !ipaddr || ipaddr == NULL) {
		fprintf(errfile, "Warring: Invalid json object(1).\n");
		return;
	}
	if (!illrouter_decode(jobj, &msg)
		|| db->isset_node(ipaddr, msg.hash, 1, errfile)) {
		fprintf(errfile, "Warring: Except in befriends.\n");
		return;
	}
	if (db->nodenum(errfile) < MAXNODES)
		return;
	
	if (db->isset_node(ipaddr, msg.hash, 0, errfile)) {
		db->staticnode(msg.hash, errfile);
		illrouter_newroute(ILL_BEFRIENDS, ipaddr);
		return;
	}

	db->newnode(ipaddr, msg.hash, 10, errfile);
	illrouter_newroute(ILL_BEFRIENDS, ipaddr);
}
/**
*	illrouter_newnode - Функция создания маршрута для
*	отправки обычного сообщения.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_straight(json_object *jobj, char *ipaddr)
{

}
/**
*	illrouter_newnode - Функция создания маршрута для
*	отправки сообщения через луковую маршрутизацию.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_onion(json_object *jobj, char *ipaddr)
{

}
