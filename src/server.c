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
static struct illumnode *nodelist;
static int mainsocket, nodelength;
static struct timeval timeout;
static struct illumdb *p_db;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static void illum_cyclesend(struct sockaddr_in *, char *, char *,
	char *, int);
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
	nodelist = database->nodelist(&nodelength);
	timeout.tv_sec = SERVER_TIMEOUT;
	timeout.tv_usec = 0;
	p_router = router;
	p_server = server;
	error = errfile;
	p_db = database;

	if (mainsocket == 0) {
		fprintf(error, "Error: Can't create socket.\n");
		return false;
	}
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
	char message[MAXTEXTSIZE], ipclient[100], *okresp,
		*failresp, *response;
	struct sockaddr_in client, server;
	socklen_t ssize;
	int len = 0;

	failresp = p_router->headers(OKREQUEST, NULL);
	okresp = p_router->headers(FAILREQUEST, NULL);

	if (strlen(failresp) > 590 || strlen(okresp) > 590)
		goto exit_resv;

	strncat(failresp, "\r\n\r\n", 4);
	strncat(okresp, "\r\n\r\n", 4);

	memset(server.sin_zero, '\0', sizeof(server.sin_zero));
	ssize = sizeof(struct sockaddr_in);
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(ILLUMPORT);
	server.sin_family = AF_INET;

	for (;; response = failresp, len = 0) {
		memset(message, '\0', MAXTEXTSIZE);
		memset(ipclient, '\0', 100);

		bind(mainsocket, (struct sockaddr *)&server, sizeof(server));
		recvfrom(mainsocket, message, MAXTEXTSIZE,
			MSG_WAITALL, (struct sockaddr *)&client, &ssize);
		inet_ntop(AF_INET, &client.sin_addr, ipclient, 100);

		if ((len = strlen(message)) < 10 || len > MAXTEXTSIZE)
			continue;

		if (SRVDEBUG)
			fprintf(error, "Received: %s from %s\n\n",
				message, ipclient);

		if (p_router->read(ipclient, message))
			response = okresp;

		sendto(mainsocket, response, strlen(response), MSG_WAITALL,
			(struct sockaddr *)&client, sizeof(client));
	}

exit_resv:
	if (mainsocket > 0) {
		shutdown(mainsocket, SHUT_RDWR);
		close(mainsocket);
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
	struct sockaddr_in client;
	char *message, *response;
	struct illumtask task;
	int length = 0;

	response = (char *)malloc(MAXTEXTSIZE + 1);
	client.sin_port = htons(ILLUMPORT);
	client.sin_family = AF_INET;
	message = (char *)malloc(1);

	for (;; p_db->gettask(&task), length = 0) {
		if (task.id == 0)
			continue;

		if (task.text && task.text != NULL)
			length += strlen(task.text);
		else
			task.text = "\0";

		length += strlen(task.headers);
		message = (char *)realloc(message, length + 10);

		sprintf(message, "%s\r\n\r\n%s", task.headers, task.text);
		memset(client.sin_zero, '\0', sizeof(client.sin_zero));
		memset(response, '\0', MAXTEXTSIZE);

		illum_cyclesend(&client, message, response,
			task.ipaddr, task.type);
	}

	if (mainsocket > 0) {
		shutdown(mainsocket, SHUT_RDWR);
		close(mainsocket);
	}
	free(message);
	free(response);
	pthread_exit(0);
}
/**
*	illum_serversend - Функция отправки сообщения.
*
*	@sockstr - Структура данных для отправки.
*	@message - Сообщение.
*	@response - Ответ.
*	@ipaddr - Ip адрес клиента.
*/
static void illum_cyclesend(struct sockaddr_in *sockstr,
	char *message, char *response, char *ipaddr, int type)
{
	struct sockaddr_in client = *sockstr;
	char *tmpip = ipaddr;
	socklen_t sendsize;
	int len = 0;

	if (!sockstr || !message || !response || !ipaddr) {
		fprintf(error, "Error: Incorrect args in cyclesend.\n");
		return;
	}

	sendsize = sizeof(struct sockaddr_in);

	for (int i = -1; i < nodelength; i++) {
		if (i != -1)
			tmpip = nodelist[i].ipaddr;

		client.sin_addr.s_addr = inet_addr(tmpip);
		sendto(mainsocket, message, strlen(message),
			MSG_WAITALL, (struct sockaddr *)&client,
			sizeof(client));

		if (SRVDEBUG)
			fprintf(error, "Sent: %s to %s\n\n", message, tmpip);
		if (type != (int)MONION)
			break;

		recvfrom(mainsocket, response, MAXTEXTSIZE,
			MSG_WAITALL, (struct sockaddr *)&client, &sendsize);

		if ((len = strlen(message)) < 10 || len > MAXTEXTSIZE)
			continue;
	}
}