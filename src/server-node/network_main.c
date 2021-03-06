/**
*	network_main.c - Функции модуля network 
*	децентрализованной сети illum отвечающие
*	за работу приложения с сетью.
*
*	@mrrva - 2018
*/
#include "include/network.h"
/**
*	Приватные переменные и указатели.
*/
static struct sockaddr_in st_rcv, st_snd;
static struct illumrouter *p_router;
static struct illumnetwork *p_net;
static pthread_mutex_t userdata;
static struct timeval timeout;
static int msocket;
/**
*	illum_network - Функция инициализации модуля
*	пересылки сообщениями.
*
*	@network - Указатель на управляющую структуру.
*	@router - Указатель на структуру router.
*	@fp - Указатель на файловый стрим лог файла.
*/
bool illum_network(struct illumnetwork *network,
	struct illumrouter *router, FILE *fp)
{
	int sol = SOL_SOCKET, opt = sizeof(timeout);
	char *timeopt;

	if (networkinit || !fp || fp == NULL) {
		printf("Error: Incorrect args in illum_network.\n");
		return false;
	}

	msocket = socket(AF_INET, SOCK_DGRAM, 0);
	network->exit_server = false;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;
	p_router = router;
	p_net = network;
	users = NULL;
	error = fp;

	st_rcv.sin_addr.s_addr = INADDR_ANY;
	st_rcv.sin_port = htons(ILLUMPORT);
	st_rcv.sin_family = AF_INET;

	st_snd.sin_port = htons(ILLUMPORT);
	st_snd.sin_family = AF_INET;

	if (msocket == 0) {
		fprintf(error, "Error: Can't create socket.\n");
		return false;
	}
	timeopt = (char *)&timeout;
	setsockopt(msocket, sol, SO_RCVTIMEO, timeopt, opt);
	setsockopt(msocket, sol, SO_SNDTIMEO, timeopt, opt);

	pthread_mutex_init(&userdata, NULL);

	if (pthread_create(&p_net->receiver, NULL, illum_receiver
		, NULL) != 0) {
		fprintf(fp, "Error: Can't start receive thread.\n");
		return false;
	}
	if (pthread_create(&p_net->sender, NULL, illum_sender
		, NULL) != 0) {
		fprintf(fp, "Error: Can't start send thread.\n");
		return false;
	}

	return (networkinit = true);
}
/**
*	illum_useract - Функция регистрации и генерации ответа
*	подключенному клиенту.
*
*	@ipport - Ip и порт нового клиента.
*	@buffer - Сообщение клиента.
*/
unsigned char *illum_useract(struct illumipport ipport,
	unsigned char *buffer)
{
	struct illumheaders *headers;
	unsigned char *text;

	if (!buffer) {
		fprintf(error, "Error: Invalid args of new user.\n");
		return NULL;
	}
	
	headers = p_router->h_decode(buffer);

	if (!headers || headers == NULL) {
		fprintf(error, "Error: Can't decode headers.\n");
		return NULL;
	}

	pthread_mutex_lock(&userdata);
	illum_removeusers(users);
	illum_register(users, headers->hash, ipport);
	pthread_mutex_unlock(&userdata);
	text = p_router->response(headers, ipport);

	free(headers);
	return text;
}
/**
*	illum_replayto - Функция создания ответа подключенному
*	клиенту сети.
*
*	@arguments - Структура параметров потока.
*/
void *illum_replayto(void *arguments)
{
	struct thrarr *args = (struct thrarr *)arguments;
	struct illumipport ipport;
	struct sockaddr *s_client;
	unsigned char *resp = NULL;

	if (!arguments || arguments == NULL) {
		fprintf(error, "Error: Invalid args in replayto.\n");
		goto exit_replayto;
	}

	inet_ntop(AF_INET, &args->client.sin_addr, ipport.ip, 20);
	s_client = (struct sockaddr *)&args->client;
	ipport.port = ntohs(args->client.sin_port);
	resp = illum_useract(ipport, args->buffer);

	if (resp && resp != NULL) {
		sendto(msocket, resp, HEADERSIZE, 0x100, s_client,
			sizeof(struct sockaddr_in));
		free(resp);
	}

exit_replayto:
	if (arguments && arguments != NULL)
		free(arguments);
	pthread_exit(0);
}
/**
*	illum_thrarray - Функция создания структуры для передачи
*	данных в поток.
*
*	@var1 - Структура клиента.
*	@var2 - Буффер данных.
*/
struct thrarr *illum_thrarray(void *var1, void *var2)
{
	struct thrarr *array = NULL;

	if (!var1 || !var2) {
		fprintf(error, "Error: Invalid args in thrarray.\n");
		return NULL;
	}
	array = (struct thrarr *)malloc(sizeof(struct thrarr));
	if (!array || array == NULL)
		return NULL;

	memcpy(&array->client, var1, sizeof(struct sockaddr_in));
	memcpy(array->buffer, var2, FULLSIZE);

	return array;
}
/**
*	illum_receiver - Функция потока приема данных от
*	клиентов сети и их обработка.
*
*/
void *illum_receiver(void)
{
	socklen_t ln = sizeof(struct sockaddr_in);
	unsigned char buff[FULLSIZE + 1];
	struct sockaddr *cli, *receiver;
	struct sockaddr_in client;
	pthread_t replay;
	void *arr;

	receiver = (struct sockaddr *)&st_rcv;
	cli = (struct sockaddr *)&client;

	do {
		bind(msocket, receiver, sizeof(st_rcv));
		recvfrom(msocket, buff, FULLSIZE, 0x100, cli, &ln);

		arr = (void *)illum_thrarray(&client, buff);
		pthread_create(&replay, NULL, illum_replayto, arr);
		memset(buff, '\0', FULLSIZE + 1);
	}
	while (p_net->exit_server == false && msocket != 0);

	if (msocket != 0) {
		shutdown(msocket, SHUT_RDWR);
		close(msocket);
	}
	p_net->exit_server = true;
	pthread_exit(0);
}
/**
*	illum_sender - Функция потока отправки данных
*	клиентам данной ноды.
*
*/
void *illum_sender(void)
{
	

	pthread_exit(0);
}