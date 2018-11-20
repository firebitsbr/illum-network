#include "../src/illum.h"

int main()
{
	struct illumdb database;
	struct illumnode *nodes;
	struct illumtask task;
	FILE *fp = fopen("./log.txt", "a+");
	int len = 0;

	illum_dbinit(&database, "./illum.db", fp);

	//database.setvar("test", "word");
	//database.newnode("12.12.12.12", "sdfgdfgdfgdfgdfgdgdfg43gdfg");
	//database.newtask("12.12.12.12", "aaaassssssssssssss", "ffffffffffffffff");
	
	database.gettask(&task);
	printf("%s - %s\n", task.headers, task.ipaddr);
/*
	nodes = database.nodelist(&len);
	printf("Node num: %d\n", len);
	for (int i = 0; i < len; i++)
		printf("%s - %s\n", nodes[i].ipaddr, nodes[i].hash);
*/

}