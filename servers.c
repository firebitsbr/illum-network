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
static void *illsrv_sendserver(void *);
static void *illsrv_getserver(void *);
/**
*	Приватные структуры
*/
struct srvdata {
	bool get, send;
	illdb *db;
	FILE *errf;
};
/**
*	illsrv_init - Функция инициализации потоков серверов.
*
*	@srvstruct - Главная управляющая структура.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illsrv_init(illsrv *srvstruct, illdb *db, FILE *errf)
{
	struct srvdata data = {.get = false, .send = false,
		.db = db, .errf = errf};

	if (!srvstruct || srvstruct == NULL || !db || db == NULL
		|| !errf || errf == NULL) {
		fprintf(errf, "Error: Incorrect input data in illsrv_init\n");
		return false;
	}

	if (pthread_create(&srvstruct->getserver, NULL, (void *)illsrv_getserver,
		&data) != 0) {
		fprintf(errf, "Error: Can't start getserver.");
		return false;
	}
	if (pthread_create(&srvstruct->sendserver, NULL, (void *)illsrv_sendserver,
		&data) != 0) {
		fprintf(errf, "Error: Can't start sendserver.");
		return false;
	}

	return true;
}
/**
*	illsrv_getserver - Функция инициализации сервера приема.
*
*	@data - Структура входящих параметров сервера.
*/
static void *illsrv_getserver(void *data)
{
	((struct srvdata *) data)->send = true;
	pthread_exit(0);
}
/**
*	illsrv_sendserver - Функция инициализации сервера отправки.
*
*	@data - Структура входящих параметров сервера.
*/
static void *illsrv_sendserver(void *data)
{
	int socket_r = socket(AF_INET, SOCK_STREAM , 0);
	struct srvdata *srvd = ((struct srvdata *) data);
	struct sockaddr_in server;
	struct stask ctask;
	char *message;

	server.sin_family = AF_INET;
	server.sin_port = htons(ILLUM_PORT);
	srvd->get = true;

	if (!srvd->send) {
		fprintf(srvd->errf, "Error: Get server is down.\n");
		goto exit_sendserver;
	}

	while (true) {
		ctask = srvd->db->currenttask(srvd->db->db, srvd->errf);
		message = ctask.text;

		if (!ctask.ipaddr || ctask.ipaddr == NULL || ctask.text
			|| ctask.text == NULL || strlen(ctask.ipaddr) < 7
			|| strlen(ctask.text) > TEXTSIZE_SEND) {
			sleep(.5);
			printf("++\n");
			continue;
		}
		if (!srvd->send || !srvd->get) {
			fprintf(srvd->errf, "Notice: Get and send server are down.\n");
			goto exit_sendserver;
		}

		server.sin_addr.s_addr = inet_addr(ctask.ipaddr);
		if (connect(socket_r, (struct sockaddr *)&server, sizeof(server)) < 0)
			continue;
		if (ctask.cert && ctask.cert != NULL)
			/* message = srvd->encrypt->encode(ctask.text, ctask.cert);*/
			message = ctask.text;

		if (send(socket_r, message, strlen(message), 0) < 0)
			fprintf(srvd->errf, "Warring: Can't send message to %s.\n", ctask.ipaddr);
		srvd->db->removetask(srvd->db->db, ctask.id, srvd->errf);
	}

exit_sendserver:
	if (message && message != NULL)
		free(message);
	pthread_exit(0);
}