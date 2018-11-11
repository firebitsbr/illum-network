#include "../src/illum.h"

int main()
{
	struct illumrouter router;
	struct illumdb database;
	FILE *fp = fopen("./log.txt", "a+");
	char *message;

	illum_dbinit(&database, "./illum.db", fp);
	illum_routerinit(&router, &database, fp);

	/*message = "{\"ipaddr\": \"\", \"hash\": \"dfghdfghdfghdfghdfh\", \"type\": 0}\r\n\r\n";
	router.read(message, "192.168.1.1");*/

	message = router.headers(FAILREQUEST, NULL);
	printf("%s\n", message);

	sleep(80);
}