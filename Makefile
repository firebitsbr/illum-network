database:
	gcc -g -Wall -o db database_test.c ../sources/database.c -std=c11 -lsqlite3

servers:
	gcc -g -Wall -o srv servers_test.c ../sources/servers.c ../sources/database.c -std=c11 -lpthread -lsqlite3
