#include "../sources/database.h"

int main()
{
	illdb db;
	FILE *fp = fopen("./log.txt", "a+");
	struct node_list *nodes;
	unsigned int length = 0, i = 0;

	/* Init illdb */
	if (!illdb_init("./illum.db", &db, fp))
		printf("Error: Can't init illdb struct.\n");

	/* Entering value to database ... */
	db.newnode(db.db, "8.8.8.8", "s0m3hashs0m3hashs0m3hash", 500, 
		"Some info abount server, Some info abount server, Some info abount server, Some info abount server,"
		"Some info abount server, Some info abount server, Some info abount server, ",
		"CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT_CERT", fp);

	/* Getting node list from database */
	nodes = db.nodelist(db.db, &length, fp);
	printf("Number of nodes: %i\n", length);

	for (;i < length; i++)
		printf("Ip:%s Hash:%s\n", nodes[i].ipaddr, nodes[i].hash);


	/* Free memory */
	fclose(fp);
	illdb_free(&db);

	return 0;
}
