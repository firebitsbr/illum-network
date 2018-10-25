database:
	gcc -g -Wall -o db database_test.c ../sources/database.c -std=c11 -lsqlite3

servers:
	gcc -g -Wall -o srv servers_test.c ../sources/server.c ../sources/database.c -std=c11 -lpthread -lsqlite3

router:
	gcc -g -Wall -o rte router_test.c ../sources/encryption.c ../sources/router.c ../sources/database.c -std=c11 -lsodium -ljson-c -lsqlite3

encryption:
	gcc -g -Wall -o rte encrypt_test.c ../sources/encryption.c ../sources/database.c -std=c11 -lsodium -lsqlite3

functions:
	gcc -g -Wall -o funcs functions_test.c ../sources/functions.c ../sources/server.c ../sources/database.c ../sources/encryption.c ../sources/router.c -std=c11 -lsodium -lsqlite3 -ljson-c -lpthread