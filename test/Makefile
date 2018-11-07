servers:
	rm -f ./server
	gcc -g -Wall -o server testserver.c ../src/server.c ../src/encryption.c ../src/router.c ../src/database.c -std=c11 -lsodium -lsqlite3 -ljson-c -lpthread

database:
	rm -f ./database
	gcc -g -Wall -o database testdb.c ../src/database.c -std=c11 -lsodium -lsqlite3 -ljson-c -lpthread

encrypt:
	rm -f ./encrypt
	gcc -g -Wall -o encrypt testencrypt.c ../src/encryption.c ../src/database.c -std=c11 -lsodium -lsqlite3 -ljson-c -lpthread
