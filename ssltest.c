#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sodium.h>
//#include "nacl/crypto_box.h" //for libnacl


void randombytes(uint8_t buffer[], unsigned long long size)
{
	int fd;

	fd = open( "/dev/urandom", O_RDONLY );
	if( fd < 0 ) {
		fprintf( stderr, "Failed to open /dev/urandom\n" );
		exit(1);
	}

	int rc;
	if( (rc = read( fd, buffer, size )) >= 0 ) {
		close( fd );
	}
}

char* to_hex( char hex[], const uint8_t bin[], size_t length )
{
	int i;
	uint8_t *p0 = (uint8_t *)bin;
	char *p1 = hex;

	for( i = 0; i < length; i++ ) {
		snprintf( p1, 3, "%02x", *p0 );
		p0 += 1;
		p1 += 2;
	}

	return hex;
}

int is_zero( const uint8_t *data, int len )
{
	int rc = 0;

	for(int i = 0; i < len; i++)
		rc |= data[i];

	return rc;
}

#define MAX_MSG_SIZE 1400

int encrypt(uint8_t encrypted[], const uint8_t pk[], const uint8_t sk[], const uint8_t nonce[], const uint8_t plain[], int length)
{
	uint8_t temp_plain[MAX_MSG_SIZE];
	uint8_t temp_encrypted[MAX_MSG_SIZE];
	int rc;


	if(length+crypto_box_ZEROBYTES >= MAX_MSG_SIZE) {
		return -2;
	}

	memset(temp_plain, '\0', crypto_box_ZEROBYTES);
	memcpy(temp_plain + crypto_box_ZEROBYTES, plain, length);

	rc = crypto_box(temp_encrypted, temp_plain, crypto_box_ZEROBYTES + length, nonce, pk, sk);

	if( rc != 0 ) {
		return -1;
	}

	if( is_zero(temp_plain, crypto_box_BOXZEROBYTES) != 0 ) {
		return -3;
	}

	memcpy(encrypted, temp_encrypted + crypto_box_BOXZEROBYTES, crypto_box_ZEROBYTES + length);

	return crypto_box_ZEROBYTES + length - crypto_box_BOXZEROBYTES;
}
//decrypt(decrypted, eve->public_key, bob->secret_key, nonce, encrypted, rc);
int decrypt(uint8_t plain[], const uint8_t pk[], const uint8_t sk[], const uint8_t nonce[], const uint8_t encrypted[], int length)
{
	uint8_t temp_encrypted[MAX_MSG_SIZE];
	uint8_t temp_plain[MAX_MSG_SIZE];
	int rc;

	if(length+crypto_box_BOXZEROBYTES >= MAX_MSG_SIZE) {
		return -2;
	}

	memset(temp_encrypted, '\0', crypto_box_BOXZEROBYTES);
	memcpy(temp_encrypted + crypto_box_BOXZEROBYTES, encrypted, length);

	rc = crypto_box_open(temp_plain, temp_encrypted, crypto_box_BOXZEROBYTES + length, nonce, pk, sk);

	if( rc != 0 ) {
		return -1;
	}

	if( is_zero(temp_plain, crypto_box_ZEROBYTES) != 0 ) {
		return -3;
	}

	memcpy(plain, temp_plain + crypto_box_ZEROBYTES, crypto_box_BOXZEROBYTES + length);

	return crypto_box_BOXZEROBYTES + length - crypto_box_ZEROBYTES;
}

typedef struct {
	char* name;
	uint8_t public_key[crypto_box_PUBLICKEYBYTES];
	uint8_t secret_key[crypto_box_SECRETKEYBYTES];
} User;

User *new_user(char* name)
{
	User* user;

	user = (User*) malloc(sizeof(User));
	user->name = name;

	crypto_box_keypair(user->public_key, user->secret_key);

	return user;
}

void print_user(User *user)
{
	char phexbuf[2*crypto_box_PUBLICKEYBYTES+1];
	char shexbuf[2*crypto_box_SECRETKEYBYTES+1];

	printf("username: %s\n", user->name);
	printf("public key: %s\n", to_hex(phexbuf, user->public_key, crypto_box_PUBLICKEYBYTES ));
	printf("secret key: %s\n\n", to_hex(shexbuf, user->secret_key, crypto_box_SECRETKEYBYTES ));
}




uint8_t *hex_str_to_uint8(const char* string)
{
	int length, value = 0;
	bool flag = false;
	uint8_t *data;

    if (!string || string == NULL || strlen(string) % 2 != 0)
        return NULL;

    length = strlen(string);
	data = (uint8_t *)malloc(length / 2);
    memset(data, '\0', length / 2);

    for (int i = 0; i < length; i++, value = 0) {
        if (string[i] >= '0' && string[i] <= '9')
            value = string[i] - '0';
        else if (string[i] >= 'A' && string[i] <= 'F')
            value = 10 + string[i] - 'A';
        else if (string[i] >= 'a' && string[i] <= 'f')
            value = 10 + string[i] - 'a';
        else {
        	flag = true;
        	break;
        }

        data[(i / 2)] += value << (((i + 1) % 2) * 4);
    }

    if (flag) {
    	free(data);
    	return NULL;
    }

    return data;
}

#define MESSAGE ((const unsigned char *) "test")
#define MESSAGE_LEN 4
#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + MESSAGE_LEN)

int main( int argc, char **argv )
{/*
	char hexbuf[256];

	int rc;
	User *bob = new_user("bob");
	User *eve = new_user("eve");
	char *msg = "Hello";

	uint8_t nonce[crypto_box_NONCEBYTES];
	randombytes(nonce, crypto_box_NONCEBYTES);

	print_user(bob);
	print_user(eve);

	printf("message: %s\n", msg);

	uint8_t encrypted[1000];
	rc = encrypt(encrypted, bob->public_key, eve->secret_key, nonce, msg, strlen(msg));
	if( rc < 0 ) {
		return 1;
	}
	printf("encrypted: %s\n", to_hex(hexbuf, encrypted, rc ));


	uint8_t decrypted[1000];
	rc = decrypt(decrypted, eve->public_key, bob->secret_key, nonce, encrypted, rc);
	if( rc < 0 ) {
		return 1;
	}

	decrypted[rc] = '\0';
	printf("decrypted: %s\n", decrypted);


	char *hex_d = "a590d5b77e87ce6b94dd06600dea7b7f3ad1ecc2df9618707007ff397767a725";
	uint8_t public_key[crypto_box_PUBLICKEYBYTES];
	char phexbuf[2*crypto_box_PUBLICKEYBYTES+1];


	printf("\n\n> %s\n\n", to_hex(phexbuf, hex_str_to_uint8(hex_d), crypto_box_PUBLICKEYBYTES ));
*/

	User *me = new_user("me");
	User *eve = new_user("eve");

	//unsigned char key[crypto_secretbox_KEYBYTES];
	unsigned char nonce[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char ciphertext[CIPHERTEXT_LEN];
	unsigned char decrypted[MESSAGE_LEN + 1];
	memset(decrypted, '\0', MESSAGE_LEN);
	//crypto_secretbox_keygen(key);

	crypto_box_easy(ciphertext, MESSAGE, MESSAGE_LEN, nonce, me->public_key, eve->secret_key);
	printf("%s\n", ciphertext);


	if (crypto_box_open_easy(decrypted, ciphertext, CIPHERTEXT_LEN, nonce,
		eve->public_key, me->secret_key) != 0) {
	  printf("----");
	}
	 printf(decrypted);

	return 0;
}