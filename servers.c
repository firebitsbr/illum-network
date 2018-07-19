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
static struct threads illsrv_startservers(illsrv *);
static void illsrv_setnode(char *);
static void create_socket(int *);
static void *illsrv_server();
static void *illsrv_client();
/**
*	Приватные переменные
*/
static FILE *errfile;
static illdb *database;
static illroute *rte;
static struct timeval timeout;
static bool sockflag;
/**
*	Приватные структуры
*/


/**
*	illsrv_init - Функция инициализации потоков серверов.
*
*	@srvstruct - Главная управляющая структура.
*	@db - Структура управления базой данных.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illsrv_init(illsrv *srvstruct, illdb *db, illroute *route, FILE *errf)
{
	unsigned int len = 0;

	if (!srvstruct || srvstruct == NULL || !db || db == NULL ||
		!route || route == NULL || !errf || errf == NULL)
		return false;

	db->nodelist(&len, errf);
	timeout.tv_sec = SERVER_TIMEOUT;
	timeout.tv_usec = 0;
	sockflag = false;
	errfile = errf;
	database = db;
	rte = route;

	if (!(srvstruct->start = illsrv_startservers)) {
		fprintf(errf, "Error: Can't create the pointer to "
				"illsrv_startservers.\n");
		return false;
	}
	if (len == 0 && !(srvstruct->setnode = illsrv_setnode)) {
		fprintf(errf, "Error: Can't create the pointer to "
				"illsrv_setnode.\n");
		return false;
	}
	/*else
		db->updnodes(list, len);*/

	return true;
}
/**
*	illsrv_startservers - Функция инициализации сервера.
*
*	@srvstruct - Главная управляющая структура.
*/
static struct threads illsrv_startservers(illsrv *srvstruct)
{
	struct threads thrds;

	if (pthread_create(&thrds.server, NULL, illsrv_server, NULL) != 0)
		fprintf(errfile, "Error: Can't start server (1).\n");

	if (pthread_create(&thrds.client, NULL, illsrv_client, NULL) != 0)
		fprintf(errfile, "Error: Can't start server (2).\n");

	return thrds;
}
/**
*	illsrv_server - Функция создания сокета.
*
*	@socket_r - Якорь сокета.
*/
static void create_socket(int *socket_r)
{
	*socket_r = socket(AF_INET, SOCK_STREAM, 0);

	setsockopt(*socket_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	setsockopt(*socket_r, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
	setsockopt(*socket_r, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
}
/**
*	illsrv_server - Функция инициализации сервера для прием
*	данных.
*/
static void *illsrv_server()
{
	int read_s, str_size, socket_r, clnt_r;
	struct sockaddr_in server, client;
	char recive_msg[MAXTEXTSIZE];

	str_size = sizeof(struct sockaddr_in);
	server.sin_port = htons(ILLUM_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;

	for (;;) {
		memset(server.sin_zero, '\0', sizeof(server.sin_zero));
		memset(recive_msg, '\0', MAXTEXTSIZE);

		create_socket(&socket_r);
		if (bind(socket_r, (struct sockaddr *)&server, sizeof(server)) < 0) {
			fprintf(errfile, "Waring: Bind(1) returned num less 0.\n");
			goto close_socket;
		}

		listen(socket_r, 10);
		if ((clnt_r = accept(socket_r, (struct sockaddr *)&client,
			(socklen_t *)&str_size)) <= 0) {
			fprintf(errfile, "Waring: Accept(1) returned num less 0.\n");
			goto close_socket;
		}

		read_s = recv(clnt_r, recive_msg, MAXTEXTSIZE, 0);
		printf(recive_msg);

	close_socket:
		close(socket_r);
		close(clnt_r);
	}

	pthread_exit(0);
}

static void *illsrv_client()
{
	//struct sockaddr_in client = ((struct sockaddr_in) &data);
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
	int socket_r;

	if (!ipaddr || ipaddr == NULL || strlen(ipaddr) < 7) {
		fprintf(errfile, "Error: Invalid input data in illsrv_setnode.");
		goto exit_setnode;
	}

	server.sin_addr.s_addr = inet_addr(ipaddr);
	server.sin_port = htons(ILLUM_PORT);
	server.sin_family = AF_INET;
	message = "Here will be pointer to route module.";

	create_socket(&socket_r);
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
		memset(recive_msg, '\0', TEXTSIZE_BUFER);
	}

	close(socket_r);

	/*Decode a data and create new tasks*/

exit_setnode:
	free(message);
	free(data_r);
}
