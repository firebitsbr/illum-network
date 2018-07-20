
#include "../sources/servers.h"

int main()
{
	illdb db;
	illroute router;
	illsrv srv;
	FILE *fp = fopen("./log.txt", "a+");
	struct threads thrds;

	/* Init illdb */
	if (!illdb_init("./illum.db", &db, fp)) {
		printf("Error: Can't init illdb struct.\n");
		return 1;
	}

	/* Init illsrv */
	if (!illsrv_init(&srv, &db, &router, fp)) {
		printf("Error: Can't init illsrv struct.\n");
		return 1;
	}

	thrds = srv.start(srv);

	pthread_join(thrds.server, 0);

	return 0;
}
