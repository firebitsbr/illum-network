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
	bool call;
};
/**
*	Прототипы приватных функций
*/
static void illrouter_befriends(json_object *, struct headers, char *, char *);
static void illrouter_straight(json_object *, struct headers, char *, char *);
static void illrouter_newnode(json_object *, struct headers, char *, char *);
static void illrouter_onion(json_object *, struct headers, char *, char *);
static void illrouter_ping(json_object *, struct headers, char *, char *);
static void illrouter_stat(json_object *, struct headers, char *, char *);
static void illrouter_newroute(enum illheader, char *, char *);
static bool illrouter_decode(json_object *, struct headers *);
static void illrouter_readroute(char *, char *);
static char **illrouter_explode(char *);
static void illrouter_updnodes(bool);
/**
*	Приватные переменные
*/
static FILE *errfile;
static illenc *encrpt;
static illdb *db;
/**
*	illrouter_init - Функция инициализации модуля
*	построения маршрута.
*
*	@illr - Главная управляющая структура.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illrouter_init(illrouter *illr, illdb *database, illenc *enc,
	FILE *errf)
{
	if (!(errfile = errf) || !(db = database) || errfile == NULL
		|| !(encrpt = enc))
		return false;

	illr->updnodes = illrouter_updnodes;
	illr->read = illrouter_readroute;
	illr->new = illrouter_newroute;

	if (!illr->read || !illr->new || !illr->updnodes)
		return false;
	return true;
}
/**
*	illrouter_readroute - Функция считывания маршрута в
*	кодировке json.
*
*	@json - Json массив с инструкциями маршрута.
*/
static void illrouter_readroute(char *data, char *ipaddr)
{
	struct functions func[] = {
		{ILL_BEFRIENDS, illrouter_befriends, true},
		{ILL_STRAIGHT, illrouter_straight, true},
		{ILL_NEWNODE, illrouter_newnode, true},
		{ILL_ONION, illrouter_onion, false},
		{ILL_PING, illrouter_ping, false},
		{ILL_STAT, illrouter_stat, false}
	};
	unsigned int type = FUNCNULL;
	char **content, *json;
	struct headers msg;
	json_object *jobj;

	if ((content = illrouter_explode(data)) == NULL) {
		fprintf(errfile, "Error: Incorrect data in explode.\n");
		return;
	}
	//json = encrpt->decrypt(content[0], ipaddr);
	json = content[0];

	if (!(jobj = json_tokener_parse(json)) || !ipaddr
		|| ipaddr == NULL || strlen(ipaddr) < 7) {
		fprintf(errfile, "Error: Invalid json string or ip.\n");
		goto exit_create;
	}
	if (!illrouter_decode(jobj, &msg)) {
		fprintf(errfile, "Error: Can't decode json.\n");
		goto exit_create;
	}

	for (int i = 0; i < 6; i++) 
		if (func[i].id == msg.type) {
			type = i;
			break;
		}
	if (type != FUNCNULL && func[type].name && (func[type].call
		|| db->isset_node(ipaddr, NULL, 1)))
		func[type].name(jobj, msg, ipaddr, content[1]);

exit_create:
	if (content[0])
		free(content[0]);
	if (content[1])
		free(content[1]);
	if (content)
		free(content);
	if (jobj && jobj != NULL)
		json_object_put(jobj);
}
/**
*	illrouter_explode - Функция разрыва содержимого сообщения.
*
*	@content - Содержимое сообщения
*/
static char **illrouter_explode(char *content)
{
	char **data, *tok, cont[strlen(content) + 1];

	if (strlen(content) > MAXTEXTSIZE
		|| !strstr(content, "\r\n\r\n")) {
		fprintf(errfile, "Error: Can't explode content.\n");
		return NULL;
	}

	data = (char **)malloc(sizeof(char *) * 2);
	memcpy(cont, content, strlen(content) + 1);

	if ((tok = strtok(cont, "\r\n\r\n")) == NULL) {
		free(data);
		return NULL;
	}
	data[0] = (char *)malloc(strlen(tok) + 1);
	memcpy(data[0], tok, strlen(tok) + 1);

	if ((tok = strtok(NULL, "\r\n\r\n")) == NULL) {
		data[1] = NULL;
		return data;
	}
	data[1] = (char *)malloc(strlen(tok) + 1);
	memcpy(data[1], tok, strlen(tok) + 1);

	tok = strtok(NULL, "\r\n\r\n");
	return data;
}
/**
*	illrouter_newroute - Функция создания нового маршрута.
*
*	@type - Тип маршрута.
*	@ipaddr - Адрес для отправки.
*/
static void illrouter_newroute(enum illheader type, char *ipaddr, char *text)
{
	char *json, *hash, *buffer = NULL;
	unsigned int nodenum = 0;
	json_object *jobj;

	if (!ipaddr || ipaddr == NULL || strlen(ipaddr) < 7) {
		fprintf(errfile, "Error: Incorrect ipaddr in newroute.\n");
		return;
	}
	if (type > 9 && type < 20 && (!text || text == NULL
		|| strlen(text) <= 1)) {
		fprintf(errfile, "Error: Incorrect args for type %d.\n", type);
		return;
	}

	//if (text && text != NULL)
		//buffer = encrpt->encrypt(text, ipaddr);
	//buffer = (char *)malloc(strlen(text) + 10);
	//strcpy(buffer, text);

	jobj = json_object_new_object();
	hash = encrpt->publickey();
	nodenum = db->nodenum();

	json_object_object_add(jobj, "nodenum", json_object_new_int(nodenum));
	json_object_object_add(jobj, "ipaddr", json_object_new_string(""));
	json_object_object_add(jobj, "hash", json_object_new_string(hash));
	json_object_object_add(jobj, "type", json_object_new_int(type));

	if (type == ILL_ONION) {

	}

	json = (char *)json_object_to_json_string(jobj);
	db->newtask(ipaddr, buffer, json);

	if (jobj && jobj != NULL)
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
static void illrouter_newnode(json_object *jobj, struct headers msg,
	char *ipaddr, char *content)
{
	struct node_list *list;
	char *headers, *ipc;
	unsigned int len;

	if (!jobj || jobj == NULL || !ipaddr || ipaddr == NULL) {
		fprintf(errfile, "Warring: Invalid json object(1).\n");
		return;
	}

	list = db->nodelist(&len);
	if (len < MAXNODES) {
		ipc = (msg.ipaddr && strlen(msg.ipaddr) >= 7)
			? msg.ipaddr : ipaddr;

		if (!db->isset_node(ipc, NULL, 2)) {
			db->newnode(ipc, msg.hash);
			illrouter_newroute(ILL_BEFRIENDS, ipc, NULL);
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
			db->newtask(list[i].ipaddr, NULL, headers);
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
*	illrouter_ping - Функция создания маршрута для
*	проверки ноды, в сети или нет.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_ping(json_object *jobj, struct headers msg,
	char *ipaddr, char *content)
{
	struct node_list node;
	time_t ntime;

	if (!jobj || jobj == NULL) {
		fprintf(errfile, "Error: Invalid json object(2).\n");
		return;
	}
	if (!db->isset_node(ipaddr, NULL, 1)) {
		fprintf(errfile, "Warring: Except in ping.\n");
		return;
	}

	node = db->nodeinfo(ipaddr);
	ntime = time(NULL);

	if (node.hash == NULL
		|| (unsigned long)ntime - node.ping < UPDTIME)
		return;

	db->ping(ipaddr);
	illrouter_newroute(ILL_PING, ipaddr, NULL);
}
/**
*	illrouter_stat - Функция создания маршрута для
*	сбора статистики сети.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_stat(json_object *jobj, struct headers msg,
	char *ipaddr, char *content)
{

}
/**
*	illrouter_befriends - Функция создания маршрута для
*	подтверждения статической связи между нодами.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_befriends(json_object *jobj, struct headers msg,
	char *ipaddr, char *content)
{
	if (!jobj || jobj == NULL || !ipaddr || ipaddr == NULL) {
		fprintf(errfile, "Warring: Invalid json object(1).\n");
		return;
	}
	if (db->isset_node(ipaddr, NULL, 1)) {
		fprintf(errfile, "Warring: Except in befriends.\n");
		return;
	}
	if (db->nodenum(errfile) >= MAXNODES)
		return;

	if (db->isset_node(ipaddr, NULL, 0)) {
		db->staticnode(msg.hash);
		illrouter_newroute(ILL_BEFRIENDS, ipaddr, NULL);
		return;
	}

	db->newnode(ipaddr, msg.hash);
	illrouter_newroute(ILL_BEFRIENDS, ipaddr, NULL);
}
/**
*	illrouter_straight - Функция создания маршрута для
*	отправки обычного сообщения.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_straight(json_object *jobj, struct headers msg,
	char *ipaddr, char *content)
{

}
/**
*	illrouter_onion - Функция создания маршрута для
*	отправки сообщения через луковую маршрутизацию.
*
*	@jobj - Json объект маршрута.
*	@ipaddr - Ip с которого пришел запрос.
*/
static void illrouter_onion(json_object *jobj, struct headers msg,
	char *ipaddr, char *content)
{

}
/**
*	illrouter_updnodes - Функция проверки подключений.
*
*	@use_time - Обращать ли внемание на время ping.
*/
static void illrouter_updnodes(bool use_time)
{
	struct node_list *nodes;
	unsigned int len;
	time_t ntime;

	nodes = db->nodelist(&len);
	ntime = time(NULL);

	if (len == 0 || !nodes || nodes == NULL)
		return;

	for (int i = 0; i < len; i++) {
		if ((((unsigned long)ntime - nodes[i].ping > UPDTIME)
			&& use_time) || !use_time) {
			db->unstatic(nodes[i].ipaddr);
			illrouter_newroute(ILL_PING, nodes[i].ipaddr, NULL);
		}

		free(nodes[i].ipaddr);
		free(nodes[i].hash);
	}

	free(nodes);
}
