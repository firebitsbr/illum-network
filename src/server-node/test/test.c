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
	struct illumheaders *b_headers;
	struct illumipport ipport;
	char *path = "./illum.db";
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
	if (illum_router(&router, &network, &encrypt, stderr) == false) {
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
	key = encrypt.hex2byte("26f9ad6ad2d8b59458d62a8cedf78204602b6d3a3580714f9b225b977bf89b43");
	memcpy(headers + 1, key, HASHSIZE);
	strcpy(ipport.ip, "192.168.1.45");
	ipport.port = 3734;
	headers[0] = 0x03;

	b_headers = router.h_decode(headers, ipport.port);
	resp = router.response(b_headers, ipport);
	printf("Response: %s\n", resp);

	return 0;
}