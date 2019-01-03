/**
*	Тестовая часть проверки модулей network и
*	router.
*
*	@mrrva - 2018
*/
#include "../include/illum.h"

int main(int argc, char *args[])
{
	/**
	*	Управляющие структуры модулей.
	*/
	struct illumnetwork network;
	struct illumencrypt encrypt;
	struct illumrouter router;
	struct illumdb storage;
	/**
	*	Необходимые указатели.
	*/
	unsigned char headers[HEADERSIZE], *key, *resp;
	char *path = "./illum.db", *testhash;
	struct illumheaders *b_headers;
	struct illumnodes *p_temp;
	struct illumipport ipport;
	/**
	*	Инициализируем управляющие структуры и 
	*	запускаем потоки приема и отправки.
	*/
	if (illum_database(&storage, &encrypt, path, stderr) == false) {
		printf("Error: Can't init storage module.\n");
		return 1;
	}
	if (illum_encryptinit(&encrypt, &storage, stderr) == false) {
		printf("Error: Can't init encrypt module.\n");
		return 1;
	}
	if (illum_router(&router, &network, &encrypt, &storage, stderr) == false) {
		printf("Error: Can't init router module.\n");
		return 1;
	}
	if (illum_network(&network, &router, stderr) == false) {
		printf("Error: Can't init network module.\n");
		return 1;
	}
	/**
	*	Запуск функций необходимых для инициализации
	*	модулей.
	*/
	storage.setlists();
	/**
	*	Декодирование тестовых заголовков и тест
	*	функции генерации ответа.
	*/
	testhash = "26f9ad6ad2d8b59458d62a8cedf78204602b6d3a3580714f9b2"
		"25b977bf89b43";
	key = encrypt.hex2byte(testhash);
	memcpy(headers + 1, key, HASHSIZE);
	strcpy(ipport.ip, "192.168.1.45");
	ipport.port = 3734;
	headers[0] = 0x01;

	b_headers = router.h_decode(headers, ipport.port);
	resp = router.response(b_headers, ipport);
	printf("Response: %s\n", resp);
	/**
	*	Тестирование функций модуля storage.
	*		1. Попытка записать новую ноду в бд.
	*		2. Получаем указатель на ноды и отображаем их.
	*/
	p_temp = (struct illumnodes *)malloc(sizeof(struct illumnodes));
	memcpy(p_temp->hash, key, HASHSIZE);
	strcpy(p_temp->ip, "192.168.1.45");

	storage.newnode(p_temp);	
	p_temp = storage.p_nodes();

	while (p_temp != NULL) {
		printf("node_ip: %s\n", p_temp->ip);
		p_temp = p_temp->next;
	}

	return 0;
}