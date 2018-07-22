
#include "../sources/servers.h"

int main()
{
	illdb db;
	illsrv srv;
	illroute router;
	char ipaddr[20];
	struct threads thrds;
	FILE *fp = fopen("./log.txt", "a+");

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

	/* You can send a text to some ip */
	printf("Ip addr: ");
	scanf("%s", ipaddr);

	db.newtask(ipaddr, "", "This is teswt task!", fp);
	pthread_join(thrds.server, 0);
	pthread_join(thrds.client, 0);

	return 0;
}
