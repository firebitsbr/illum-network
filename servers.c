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
*	@db - Структура управления базой данных.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illsrv_init(illsrv *srvstruct, illdb *db, FILE *errf)
{
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

	return true;
}
/**
*	illsrv_getserver - Функция инициализации сервера приема.
*
*	@data - Структура входящих параметров сервера.
*/
static void *illsrv_getserver(void *data)
{
	//printf("dfgdfddf324\n");
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
	struct stask *ctask = (struct stask *)malloc(sizeof(struct stask));
	struct srvdata *srvd = ((struct srvdata *) data);
	int socket_r;
	struct sockaddr_in server;
	struct timeval timeout;
	char *message;

	server.sin_port = htons(ILLUM_PORT);
	timeout.tv_sec = SEND_TIMEOUT;
	server.sin_family = AF_INET;
	timeout.tv_usec = 0;
	srvd->get = true;

	if (!srvd->send) {
		fprintf(srvd->errf, "Error: Get server is down.\n");
		goto exit_sendserver;
	}

	while (true) {
		srvd->db->currenttask(ctask, srvd->errf);
		if (!ctask->ipaddr || ctask->ipaddr == NULL || !ctask->text
			|| ctask->text == NULL || strlen(ctask->ipaddr) < 7
			|| strlen(ctask->text) > TEXTSIZE_SEND) {
			sleep(.5);
			continue;
		}
		if (!srvd->send || !srvd->get) {
			fprintf(srvd->errf, "Notice: Get and send server are down.\n");
			goto exit_sendserver;
		}

		socket_r = socket(AF_INET, SOCK_STREAM , 0);
		if (setsockopt(socket_r, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
			sizeof(timeout))< 0) {
			fprintf(srvd->errf, "Error: Can't set a timeout for "
					"illsrv_sendserver.\n");
			goto exit_sendserver;
		}

		memset(server.sin_zero, '\0', sizeof(server.sin_zero)); 
		message = ctask->text;
		if (ctask->cert && ctask->cert != NULL)
			// message = srvd->encrypt->encode(ctask->text, ctask->cert); 
			message = ctask->text;

		server.sin_addr.s_addr = inet_addr(ctask->ipaddr);
		if (connect(socket_r, (struct sockaddr *)&server, sizeof(server)) < 0)
			continue;
		if (send(socket_r, message, strlen(message), 0) < 0)
			fprintf(srvd->errf, "Warring: Can't send message to %s.\n",
					ctask->ipaddr);
		srvd->db->removetask(ctask->id, srvd->errf);

		close(socket_r);
	}

exit_sendserver:
	/*if (message && message != NULL)
		free(message);*/
	pthread_exit(0);
}
