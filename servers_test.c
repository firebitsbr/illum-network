
#include "../sources/servers.h"

int main()
{
	illdb db;
	illsrv srv;
	FILE *fp = fopen("./log.txt", "a+");

	/* Init illdb */
	if (!illdb_init("./illum.db", &db, fp)) {
		printf("Error: Can't init illdb struct.\n");
		return 1;
	}

	/* Init illsrv */
	if (!illsrv_init(&srv, &db, fp)) {
		printf("Error: Can't init illsrv struct.\n");
		return 1;
	}
	pthread_join(srv.getserver, NULL);
	pthread_join(srv.sendserver, NULL);


	return 0;
}