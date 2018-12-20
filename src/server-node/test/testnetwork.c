/**
*	Тестовая часть проверки модулей network и
*	router.
*
*	@mrrva - 2018
*/
#include "../network.h"

int main(int argc, char *args[])
{
	/**
	*	Управляющие структуры модулей.
	*/
	struct illumnetwork network;
	struct illumrouter router;
	/**
	*	Инициализируем управляющие структуры и 
	*	запускаем потоки приема и отправки.
	*/
	if (illum_router(&router, &network, stderr) == false) {
		printf("Error: Can't init router module.\n");
		return 1;
	}
	if (illum_network(&network, &router, stderr) == false) {
		printf("Error: Can't init network module.\n");
		return 1;
	}
	return 0;
}