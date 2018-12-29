test:
	gcc -D _GNU_SOURCE -g -Wall -o test_m test.c \
	../network_main.c ../network_clients.c ../router.c ../storage_main.c \
	../storage_nodes.c ../encryption.c -std=c11 -lpthread -lsqlite3 -lsodium
