#include "../src/illum.h"

int main()
{
	struct illumrouter router;
	struct illumdb database;
	FILE *fp = fopen("./log.txt", "a+");

	illum_dbinit(&database, "./illum.db", fp);
	illum_routerinit(&router, &database, fp);

	router.read("test data ololo", "192.168.1.1"); sleep(1);
	router.read("test data aaaaa", "192.168.1.2"); sleep(1);
	router.read("test data uga-buga", "192.168.1.3");

	getchar();
	getchar();
	getchar();
	getchar();
	getchar();
}