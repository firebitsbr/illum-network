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
static void illsrv_closesocket(int *, ...);
static void illsrv_createsocket(int *);
static void illsrv_setnode(char *);
static void *illsrv_server();
static void *illsrv_client();
/**
*	Приватные переменные
*/
static FILE *errfile;
static illdb *database;
static illroute *rte;
static struct timeval tout;
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

	if (!srvstruct || srvstruct == NULL || !db || db == NULL
		|| !route || route == NULL || !errf || errf == NULL)
		return false;

	db->nodelist(&len, errf);
	tout.tv_sec = SERVER_TIMEOUT;
	tout.tv_usec = 0;
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
*	illsrv_createsocket - Функция создания сокета.
*
*	@soc - Якорь сокета.
*/
static void illsrv_createsocket(int *soc)
{
	int sc = SOL_SOCKET, times = sizeof(tout);
	char *timeo = (char *)&tout;
	*soc = socket(AF_INET, SOCK_STREAM, 0);

	setsockopt(*soc, sc, SO_RCVTIMEO, timeo, times);
	setsockopt(*soc, sc, SO_SNDTIMEO, timeo, times);
	setsockopt(*soc, sc, SO_REUSEADDR, &(int){1}, sizeof(int));
}
/**
*	illsrv_closesocket - Функция закрытия сокета.
*
*	@length - Количество переменных.
*	@... - Входящие аргументы.
*/
static void illsrv_closesocket(int *length, ...)
{
	int *var;
	va_list ap;
	va_start(ap, length);

	for (int i = 0; i < *length; i++) {
		var = va_arg(ap, int *);
		shutdown(*var, SHUT_RDWR);
		close(*var);
	}
	va_end(ap);
}
/**
*	illsrv_server - Функция инициализации сервера для прием
*	данных.
*/
static void *illsrv_server()
{
	int mlength, socsize, socket_r, clnt_r;
	struct sockaddr_in server, client;
	char message[MAXTEXTSIZE];

	socsize = sizeof(struct sockaddr_in);
	server.sin_port = htons(ILLUM_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;

	for (;;) {
		memset(server.sin_zero, '\0', sizeof(server.sin_zero));
		memset(message, '\0', MAXTEXTSIZE);
		illsrv_createsocket(&socket_r);

		if (bind(socket_r, (struct sockaddr *)&server,
				sizeof(server)) < 0) {
			fprintf(errfile, "Waring: Bind() returned num less 0.\n");
			illsrv_closesocket(&(int){1}, &socket_r);
		}

		listen(socket_r, 10);
		if ((clnt_r = accept(socket_r, (struct sockaddr *)&client,
							(socklen_t *)&socsize)) <= 0) {
			fprintf(errfile, "Waring: Accept() returned num less 0.\n");
			illsrv_closesocket(&(int){2}, &socket_r, &clnt_r);
		}

		mlength = recv(clnt_r, message, MAXTEXTSIZE, 0);
		illsrv_closesocket(&(int){2}, &socket_r, &clnt_r);

		printf(message);
	}

	pthread_exit(0);
}
/**
*	illsrv_client - Функция инициализации сервера для отправки
*	данных.
*/
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
	char *message, recive_msg[MAXTEXTSIZE];
	struct sockaddr_in server;
	int mlength = 0, socket_r;

	if (!ipaddr || ipaddr == NULL || strlen(ipaddr) < 7) {
		fprintf(errfile, "Error: Invalid input data in illsrv_setnode.");
		goto exit_setnode;
	}

	server.sin_addr.s_addr = inet_addr(ipaddr);
	server.sin_port = htons(ILLUM_PORT);
	server.sin_family = AF_INET;
	message = "Here will be pointer to route module.";

	illsrv_createsocket(&socket_r);
	memset(server.sin_zero, '\0', sizeof(server.sin_zero)); 

	if (connect(socket_r, (struct sockaddr *)&server, sizeof(server)) < 0
		|| send(socket_r, message, strlen(message), 0) < 0) {
		fprintf(errfile, "Error: Can't send message to %s.\n", ipaddr);
		goto exit_setnode;
	}

	mlength = recv(socket_r, recive_msg, MAXTEXTSIZE, 0);
	illsrv_closesocket(&(int){1}, &socket_r);
	printf(recive_msg);

	/*Decode a data and create new tasks*/

exit_setnode:
	free(message);
}
