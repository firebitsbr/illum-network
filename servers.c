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
static struct timeval tout;
static illdb *database;
static illrouter *rte;
static bool sockflag;
static FILE *errfile;
/**
*	illsrv_init - Функция инициализации потоков серверов.
*
*	@srvstruct - Главная управляющая структура.
*	@db - Структура управления базой данных.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illsrv_init(illsrv *srvstruct, illdb *db, illrouter *route, FILE *errf)
{
	unsigned int len = 0;

	if (!srvstruct || srvstruct == NULL || !db || db == NULL
		|| !route || route == NULL || !errf || errf == NULL)
		return false;

	db->nodelist(&len, errf);
	tout.tv_sec = SERVER_TIMEOUT;
	sockflag = false;
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
	int so = SOL_SOCKET, times = sizeof(tout);
	char *timeo = (char *)&tout;
	*soc = socket(AF_INET, SOCK_STREAM, 0);

	setsockopt(*soc, so, SO_RCVTIMEO, timeo, times);
	setsockopt(*soc, so, SO_SNDTIMEO, timeo, times);
	setsockopt(*soc, so, SO_REUSEADDR, &(int){1}, sizeof(so));
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
	char message[MAXTEXTSIZE + 1], ipclient[100];
	int mlength, socsize, socket_r, clnt_r;
	struct sockaddr_in server, client;

	socsize = sizeof(struct sockaddr_in);
	server.sin_port = htons(ILLUM_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;

	for (;; sockflag = false) {
		memset(server.sin_zero, '\0', sizeof(server.sin_zero));
		memset(message, '\0', MAXTEXTSIZE);
		memset(ipclient, '\0', 100);
		illsrv_createsocket(&socket_r);

		if (bind(socket_r, (struct sockaddr *)&server,
				sizeof(server)) < 0) {
			fprintf(errfile, "Waring: Bind() returned num less 0.\n");
			illsrv_closesocket(&(int){1}, &socket_r);
		}

		sockflag = true;
		listen(socket_r, 10);

		if ((clnt_r = accept(socket_r, (struct sockaddr *)&client,
							(socklen_t *)&socsize)) <= 0) {
			fprintf(errfile, "Waring: Accept() returned num less 0.\n");
			illsrv_closesocket(&(int){2}, &socket_r, &clnt_r);
		}

		mlength = recv(clnt_r, message, MAXTEXTSIZE, 0);
		inet_ntop(AF_INET, &client.sin_addr, ipclient, 100);

		illsrv_closesocket(&(int){2}, &socket_r, &clnt_r);
		if (MAXTEXTSIZE >= mlength && message[mlength] == '\0')
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
	struct sockaddr_in client;
	int socket_r, length = 0;
	struct stask task;
	char *message;

	client.sin_port = htons(ILLUM_PORT);
	client.sin_family = AF_INET;
	message = (char *)malloc(1);

	for (;; database->currenttask(&task, errfile), length = 0) {
		if (sockflag || !task.ipaddr || task.id == 0)
			continue;

		if (task.cert && task.cert != NULL)
			// task.headers = enc->encrypt(task.text, task.cert);
			printf("Hah, lol\n");

		if (task.text && task.text != NULL)
			length += strlen(task.text);
		else
			task.text = "\0";
		length += strlen(task.headers);

		message = (char *)realloc(message, length + 10);
		sprintf(message, "%s\r\n\r\n%s", task.headers, task.text);

		database->removetask(task.id);
		illsrv_createsocket(&socket_r);
		client.sin_addr.s_addr = inet_addr(task.ipaddr);
		memset(client.sin_zero, '\0', sizeof(client.sin_zero));

		if (connect(socket_r, (struct sockaddr *)&client, sizeof(client)) < 0
			|| send(socket_r, message, strlen(message) + 1, 0) < 0)
			fprintf(errfile, "Error: Can't send message (client).\n");

		illsrv_closesocket(&(int){1}, &socket_r);
	}

	free(message);
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
		|| send(socket_r, message, strlen(message) + 1, 0) < 0) {
		fprintf(errfile, "Error: Can't send message (setnode).\n");
		goto exit_setnode;
	}

	mlength = recv(socket_r, recive_msg, MAXTEXTSIZE, 0);
	illsrv_closesocket(&(int){1}, &socket_r);
	if (MAXTEXTSIZE >= mlength && recive_msg[mlength] == '\0')
		printf(recive_msg);

	/*Decode a data and create new tasks*/

exit_setnode:
	illsrv_closesocket(&(int){1}, &socket_r);
	free(message);
}
