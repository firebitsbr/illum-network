#include "../src/illum.h"

int main()
{
	struct illumserver server;
	struct illumrouter router;
	struct illumdb database;
	FILE *fp = fopen("./log.txt", "a+");

	illum_serverinit(&server, &database, &router, fp);
	server.start();

	pthread_join(server.resv, 0);
}