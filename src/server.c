/**
*	server.c - Функции для управления потоками серверов
*	проекта.	
*
*	@mrrva - 2018
*/
#include "./illum.h"
/**
*	Приватные переменные и указатели.
*/
static struct illumserver *p_server;
static struct illumrouter *p_router;
static struct timeval timeout;
static struct illumdb *p_db;
static int mainsocket;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static bool illum_serverstart();
static void *illum_serverresv();
static void *illum_serversend();
/**
*	illum_serverinit - Функция инициализации главной структуры
*	структуры модуля для управления.
*
*	@server - Управляющая структура модулем сервера.
*	@database - Управляющая структура базы данных.
*	@errfile - Файловый стрим на лог-фаил.
*/
bool illum_serverinit(struct illumserver *server,
	struct illumdb *database, struct illumrouter *router,
	FILE *errfile)
{
	if (!server || !database || !router || !errfile) {
		printf("Error: Incorect args in illum_serverinit.\n");
		return false;
	}

	mainsocket = socket(AF_INET, SOCK_DGRAM, 0);
	timeout.tv_sec = SERVER_TIMEOUT;
	timeout.tv_usec = 0;
	p_router = router;
	p_server = server;
	error = errfile;
	p_db = database;

	if (!(p_server->start = illum_serverstart)) {
		fprintf(error, "Error: Can't create the pointer to "
			"illum_serverstart.\n");
		return false;
	}
	return true;
}
/**
*	illum_serverstart - Функция инициализации потоков серверов.
*
*/
static bool illum_serverstart()
{
	char *timeo = (char *)&timeout;
	int so, times, thr_1, thr_2;

	times = sizeof(timeout);
	so = SOL_SOCKET;

	setsockopt(mainsocket, so, SO_RCVTIMEO, timeo, times);
	setsockopt(mainsocket, so, SO_SNDTIMEO, timeo, times);
	setsockopt(mainsocket, so, SO_REUSEADDR, &(int){1}, 
		sizeof(so));

	thr_1 = pthread_create(&p_server->resv, NULL, illum_serverresv,
		NULL);
	thr_2 = pthread_create(&p_server->send, NULL, illum_serversend,
		NULL);

	if (thr_1 != 0 || thr_2 != 0) {
		fprintf(error, "Error: Can't start threads of server.\n");
		return false;
	}
	return true;
}
/**
*	illum_serverresv - Функция потока сервера для приема сообщений.
*
*/
static void *illum_serverresv()
{
	char message[MAXTEXTSIZE], ipclient[100], *okresp, *failresp,
		*response;
	struct sockaddr_in client, server;
	socklen_t sendsize;
	int length = 0;

	failresp = p_router->headers(OKREQUEST, NULL);
	okresp = p_router->headers(FAILREQUEST, NULL);

	sendsize = sizeof(struct sockaddr_in);
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(ILLUMPORT);
	server.sin_family = AF_INET;

	for (;; response = failresp, length = 0) {
		memset(server.sin_zero, '\0', sizeof(server.sin_zero));
		memset(message, '\0', MAXTEXTSIZE);
		memset(ipclient, '\0', 100);

		bind(mainsocket, (struct sockaddr *)&server, sizeof(server));
		length = recvfrom(mainsocket, message, MAXTEXTSIZE, MSG_DONTWAIT,
						(struct sockaddr *)&client, &sendsize);
		inet_ntop(AF_INET, &client.sin_addr, ipclient, 100);

		if (length < 0 || length > MAXTEXTSIZE)
			continue;
		if (SRVDEBUG)
			printf("Received: %s from %s\n\n", message, ipclient);

		if (p_router->read(ipclient, message))
			response = okresp;

		/*sendto(mainsocket, "234", 3, MSG_DONTWAIT,
				(struct sockaddr *)&client, sizeof(client));*/
	}

	free(failresp);
	free(okresp);
	pthread_exit(0);
}
/**
*	illum_serversend - Функция потока сервера для отправки сообщений.
*
*/
static void *illum_serversend()
{
	//struct sockaddr_in client;
	//unsigned int length = 0;
	char *message;

	//client.sin_port = htons(ILLUMPORT);
	//client.sin_family = AF_INET;
	message = (char *)malloc(1);

	/*for (;;) {

	}*/
	sleep(10);

	free(message);
	pthread_exit(0);
}