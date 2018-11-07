#include "../src/illum.h"

int main()
{
	struct illumdb database;
	struct illumtask task;
	FILE *fp = fopen("./log.txt", "a+");

	illum_dbinit(&database, "./illum.db", fp);

	database.setvar("test", "word");
	database.newnode("12.12.12.12", "sdfgdfgdfgdfgdfgdgdfg43gdfg");
	database.newtask("12.12.12.12", "aaaassssssssssssss", "ffffffffffffffff");
	database.gettask(&task);

	printf("%s - %s\n", task.headers, task.ipaddr);
}