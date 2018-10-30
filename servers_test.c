
#include "../sources/server.h"

int main()
{
	illdb db;
	illsrv srv;
	illenc enc;
	illrouter router;
	struct threads thrds;
	FILE *fp = fopen("./log.txt", "a+");

	/* Init illdb */
	if (!illdb_init("./illum.db", &db, fp)) {
		printf("Error: Can't init illdb struct.\n");
		return 1;
	}

	if (!illenc_init(&enc, &db, fp)) {
		printf("Error: Can't init illenc struct.\n");
		return 1;
	}

	if (!illrouter_init(&router, &db, &enc, fp)) {
		printf("Error: Can't init illrouter struct.\n");
		return 1;
	}

	/* Init illsrv */
	if (!illsrv_init(&srv, &db, &router, fp)) {
		printf("Error: Can't init illsrv struct.\n");
		return 1;
	}

	//router.new(0, "192.168.1.42", NULL);
	//router.updnodes(false);
	thrds = srv.start(srv);

	/* You can send a text to some ip */
	//printf("Ip addr: ");
	//scanf("%s", ipaddr);

	//id = db.newtask(ipaddr, NULL, "This is teswt task!", "{fgdfg}", fp);
	//printf("Task id:%d\n", id);
	//pthread_join(thrds.server, 0);

	pthread_join(thrds.client, 0);

	//router.read("{ \"nodenum\": 0, \"ipaddr\": \"\", \"hash\": \"cbffadd01e014abdc4a36899e946ebed67fc4aff11d3bf8223c7c3cba0604637\", \"type\": 3 }\r\n\r\n", "192.168.1.42");
	//router.read("{ \"nodenum\": 0, \"ipaddr\": \"\", \"hash\": \"cbffadd01e014abdc4a36899e946ebed67fc4aff11d3bf8223c7c3cba0604637\", \"type\": 3 }\r\n\r\n", "192.168.1.42");

	return 0;
}