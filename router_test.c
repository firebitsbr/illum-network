
#include "../sources/router.h"

int main() 
{
	illdb db;
	illrouter rte;
	FILE *fp = fopen("./log.txt", "a+");
	
	/* Init illdb */
	if (!illdb_init("./illum.db", &db, fp)) {
		printf("Error: Can't init illdb struct.\n");
		return 1;
	}

	if (!illrouter_init(&rte, &db, fp)) {
		printf("Error: Can't init illrouter struct.\n");
		return 1;
	}

	rte.read("{\"type\" : 0, \"nodenum\" : 0, \"ipaddr\" : \"\", "
		"\"hash\" : \"dfghdfhdfhhfdhfdfdfdh\"}", "192.168.1.1");

	return 0;
}