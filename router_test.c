
#include "../sources/router.h"

int main() 
{
	illdb db;
	illenc enc;
	illrouter rte;
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

	if (!illrouter_init(&rte, &db, &enc, fp)) {
		printf("Error: Can't init illrouter struct.\n");
		return 1;
	}

	/*rte.read("{\"type\" : 1, \"nodenum\" : 0, \"ipaddr\" : \"\", "
		"\"hash\" : \"bvmnbvmnbvmnvbmnbvvbmn\"}", "192.168.1.1");*/
	/*
	> NEW NODE + BE FRIENDS

	rte.read("{\"type\" : 0, \"nodenum\" : 0, \"ipaddr\" : \"\", "
		"\"hash\" : \"bvmnbvmnbvmnvbmnbvvbmn\"}", "192.168.1.1");

	rte.read("{\"type\" : 3, \"nodenum\" : 0, \"ipaddr\" : \"\", "
		"\"hash\" : \"bvmnbvmnbvmnvbmnbvvbmn\"}", "192.168.1.1");

	rte.read("{\"type\" : 3, \"nodenum\" : 0, \"ipaddr\" : \"\", "
		"\"hash\" : \"bvmnbvmnbvmnvbmnbvvbmn\"}", "192.168.1.1");
	*/

	return 0;
}