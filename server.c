/**
*	server.c - Функции для управления потоками серверов
*	проекта.	
*
*	@mrrva - 2018
*/
#include "./server.h"
/**
*	Прототипы приватных функций
*/
static struct threads illsrv_startservers();
static void illsrv_createsocket(int *);
static void *illsrv_server();
static void *illsrv_client();
/**
*	Приватные переменные
*/
static struct timeval tout;
static int socket_master;
static illdb *database;
static illrouter *rte;
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
	if (!srvstruct || srvstruct == NULL || !db || db == NULL
		|| !route || route == NULL || !errf || errf == NULL)
		return false;

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

	return true;
}
/**
*	illsrv_startservers - Функция инициализации сервера.
*/
static struct threads illsrv_startservers()
{
	struct threads thrds;

	illsrv_createsocket(&socket_master);

	if (pthread_create(&thrds.server, NULL, illsrv_server, NULL) != 0
		|| pthread_create(&thrds.client, NULL, illsrv_client, NULL) != 0) {
		fprintf(errfile, "Error: Can't start servers.\n");
		exit(1);
	}
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
	*soc = socket(AF_INET, /*SOCK_STREAM*/SOCK_DGRAM, 0);

	setsockopt(*soc, so, SO_RCVTIMEO, timeo, times);
	setsockopt(*soc, so, SO_SNDTIMEO, timeo, times);
	setsockopt(*soc, so, SO_REUSEADDR, &(int){1}, sizeof(so));
}
/**
*	illsrv_server - Функция инициализации сервера для прием
*	данных.
*/
static void *illsrv_server()
{
	char message[MAXTEXTSIZE + 1], ipclient[100];
	struct sockaddr_in server, client;
	socklen_t sendsize;
	int mlength;

	sendsize = sizeof(struct sockaddr_in);
	server.sin_port = htons(ILLUM_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;

	while (true) {
		memset(server.sin_zero, '\0', sizeof(server.sin_zero));
		memset(message, '\0', MAXTEXTSIZE);
		memset(ipclient, '\0', 100);
		rte->updnodes(true);

		bind(socket_master, (struct sockaddr *)&server, sizeof(server));
		mlength = recvfrom(socket_master, message, MAXTEXTSIZE, MSG_DONTWAIT,
							(struct sockaddr *)&client, &sendsize);
		inet_ntop(AF_INET, &client.sin_addr, ipclient, 100);

		if (strlen(message) > 10 && MAXTEXTSIZE >= mlength
			&& message[mlength] == '\0')
			rte->read(message, ipclient);
	}

	if (socket_master > 0) {
		shutdown(socket_master, SHUT_RDWR);
		close(socket_master);
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
	struct stask task;
	int length = 0;
	char *message;

	client.sin_port = htons(ILLUM_PORT);
	client.sin_family = AF_INET;
	message = (char *)malloc(1);

	for (;; database->currenttask(&task, errfile), length = 0) {
		if (!task.ipaddr || task.id == 0)
			continue;

		if (task.text && task.text != NULL)
			length += strlen(task.text);
		else
			task.text = "\0";
		length += strlen(task.headers);

		message = (char *)realloc(message, length + 10);
		sprintf(message, "%s\r\n\r\n%s", task.headers, task.text);

		client.sin_addr.s_addr = inet_addr(task.ipaddr);
		memset(client.sin_zero, '\0', sizeof(client.sin_zero));

		if (sendto(socket_master, message, strlen(message) + 1,
					MSG_DONTWAIT, (struct sockaddr *)&client,
					sizeof(client)) < 0)
			fprintf(errfile, "Error: Can't send message (client).\n");
		else
			database->removetask(task.id);
	}

	if (socket_master > 0) {
		shutdown(socket_master, SHUT_RDWR);
		close(socket_master);
	}
	free(message);
	pthread_exit(0);
}
