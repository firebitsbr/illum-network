/**
*	servers.c - Функции для управления потоками серверов
*	проекта.	
*
*	@mrrva - 2018
*/
#include "./servers.h"
/**
*	Прототипы приватных функций
*/
static void *illsrv_startserver(void *);
static void illsrv_setnode(char *);
/**
*	Приватные переменные
*/
static FILE *errfile;
static illdb *database;
static illroute *rte;
static struct timeval timeout;
static int socket_r;
/**
*	illsrv_init - Функция инициализации потоков серверов.
*
*	@srvstruct - Главная управляющая структура.
*	@db - Структура управления базой данных.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illsrv_init(illsrv *srvstruct, illdb *db, illroute *route, FILE *errf)
{
	struct node_list *list;
	unsigned int len = 0;

	if (!srvstruct || srvstruct == NULL || !db || db == NULL ||
		!route || route == NULL || !errf || errf == NULL)
		return false;

	list = db->nodelist(&len, errf);
	timeout.tv_sec = SERVER_TIMEOUT;
	timeout.tv_usec = 0;
	errfile = errf;
	database = db;
	rte = route;

	if (!(srvstruct->start = illsrv_startserver)) {
		fprintf(errf, "Error: Can't create the pointer to "
				"illsrv_startserver.\n");
		return false;
	}
	if (len == 0 && !(srvstruct->setnode = illsrv_setnode)) {
		fprintf(errf, "Error: Can't create the pointer to "
				"illsrv_setnode.\n");
		return false;
	}
	else
		db->updnodes(list, len);

	return true;
/*
	struct srvdata *data = (struct srvdata *)malloc(sizeof(struct srvdata));

	data->send = data->get = false;
	data->errf = errf;
	data->db = db;

	if (!srvstruct || srvstruct == NULL || !db || db == NULL
		|| !errf || errf == NULL) {
		fprintf(errf, "Error: Incorrect input data in illsrv_init\n");
		free(data);
		return false;
	}

	if (pthread_create(&srvstruct->getserver, NULL, illsrv_getserver,
		data) != 0) {
		fprintf(errf, "Error: Can't start getserver.");
		free(data);
		return false;
	}
	if (pthread_create(&srvstruct->sendserver, NULL, illsrv_sendserver,
		data) != 0) {
		fprintf(errf, "Error: Can't start sendserver.");
		free(data);
		return false;
	}
*/
}
/**
*	illsrv_startserver - Функция инициализации сервера.
*
*	@data - Структура входящих параметров сервера.
*/
static void *illsrv_startserver(void *data)
{

	pthread_exit(0);
}
/**
*	illsrv_setnode - Функция инициализации входящей ноды.
*
*	@ipaddr - Ip адрес входящей ноды.
*/
static void illsrv_setnode(char *ipaddr)
{
	char *message, *data_r = (char *)malloc(1), recive_msg[TEXTSIZE_BUFER];
	struct sockaddr_in server;
	int read_s = 0, read_full = 0;

	if (!ipaddr || ipaddr == NULL || strlen(ipaddr) < 7) {
		fprintf(errfile, "Error: Invalid input data in illsrv_setnode.");
		goto exit_setnode;
	}

	server.sin_addr.s_addr = inet_addr(ipaddr);
	server.sin_port = htons(ILLUM_PORT);
	server.sin_family = AF_INET;
	message = "Here will be pointer to route module.";

	socket_r = socket(AF_INET, SOCK_STREAM , 0);
	if (setsockopt(socket_r, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
		sizeof(timeout)) < 0) {
		fprintf(errfile, "Error: Can't set a timeout for send in "
				"illsrv_setnode.\n");
		goto exit_setnode;
	}
	if (setsockopt(socket_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
		sizeof(timeout)) < 0) {
		fprintf(errfile, "Error: Can't set a timeout for recive in "
				"illsrv_setnode.\n");
		goto exit_setnode;
	}

	memset(server.sin_zero, '\0', sizeof(server.sin_zero)); 
	if (connect(socket_r, (struct sockaddr *)&server, sizeof(server)) < 0
		|| send(socket_r, message, strlen(message), 0) < 0) {
		fprintf(errfile, "Error: Can't send message to %s.\n", ipaddr);
		goto exit_setnode;
	}

	while ((read_s = recv(socket_r, recive_msg, TEXTSIZE_BUFER, 0)) > 0) {
		read_full += read_s;
		if (read_full > MAXTEXTSIZE) {
			fprintf(errfile, "Waring: Got very long message in "
					"illsrv_setnode.\n");
			goto exit_setnode;
		}

		data_r = (char *)realloc(data_r, read_full + 1);
		strncat(data_r, recive_msg, read_s);
	}

	close(socket_r);

	/*Decode a data and create new tasks*/

exit_setnode:
	free(message);
	free(data_r);
}
