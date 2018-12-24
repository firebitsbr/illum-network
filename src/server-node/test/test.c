/**
*	Тестовая часть проверки модулей network и
*	router.
*
*	@mrrva - 2018
*/
#include "../illum.h"

int main(int argc, char *args[])
{
	/**
	*	Управляющие структуры модулей.
	*/
	struct illumnetwork network;
	struct illumencrypt encrypt;
	struct illumrouter router;
	struct illumdb storage;
	char *path = "./illum.db";
	/**
	*	Инициализируем управляющие структуры и 
	*	запускаем потоки приема и отправки.
	*/
	if (illum_database(&storage, path, stderr) == false) {
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
	return 0;
}