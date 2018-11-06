#include "../sources/encryption.h"
#include <time.h>

#define MESSAGE (const unsigned char *) "Message"
#define MESSAGE_LEN 7
#define CIPHERTEXT_LEN (crypto_box_SEALBYTES + MESSAGE_LEN)


unsigned char *illenc_hex2byte(char *string)
{
	int length, value = 0;
	bool flag = false;
	unsigned char *data;

	if (!string || string == NULL || strlen(string) % 2 != 0)
		return NULL;

	length = strlen(string);
	data = (unsigned char *)malloc(length / 2);
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

int main()
{
	illdb db;
	illenc enc;
	FILE *fp = fopen("./log.txt", "a+");

	if (!illdb_init("./illum.db", &db, fp)) {
		printf("Error: Can't init illdb struct.\n");
		return 1;
	}
	if (!illenc_init(&enc, &db, fp)) {
		printf("Error: Can't init illenc struct.\n");
		return 1;
	}

	//printf("%s", enc.encrypt("Test Message just for fun... bro lol!", "192.168.1.42"));
	//char *own_public = "ef11984f03d6de4ab35eb8ec5a9a952ea7aeb3860c8f92cdcf757f1cc52bc73d";
	//char *own_secret = "4af0a35344c7fadf09bef3ba78a80e2005f9cd67ff8c544d109195cd5a285bf2";
	//////////////////////////////////////////////////////////////////////////////////////
	char *sec_public = "cbffadd01e014abdc4a36899e946ebed67fc4aff11d3bf8223c7c3cba0604637";
	char *sec_secret = "0fa663c960a3fc76caa10eab462062c3cf118bb2400e5e1c4120dea1cbaa7e90";
	//////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	//unsigned char *own_public_byte = illenc_hex2byte(own_public);
	//unsigned char *own_secret_byte = illenc_hex2byte(own_secret);
	//////////////////////////////////////////////////////////////////////////////////////
	unsigned char *sec_public_byte = illenc_hex2byte(sec_public);
	unsigned char *sec_secret_byte = illenc_hex2byte(sec_secret);

	unsigned char decrypted[MAXTEXTSIZE + 1];
	char *dec;
	dec = (char *)malloc(1000);
	memset(dec, '\0', 1000);
	memset(decrypted, '\0', MAXTEXTSIZE + 1);

/*


	crypto_box_seal(decrypted, MESSAGE, MESSAGE_LEN, sec_public_byte);

	char *hexxxaa = enc.byte2hex(decrypted, crypto_box_SEALBYTES + MESSAGE_LEN);
	unsigned char *purebyte = enc.hex2byte(hexxxaa);



if (crypto_box_seal_open(decrypted, purebyte, crypto_box_SEALBYTES + MESSAGE_LEN,
                         sec_public_byte, sec_secret_byte) != 0) {
		printf("error\n");
	}else
	printf("2) %s\n\n", decrypted);
*/



//sodium_bin2hex(decrypted, 1000, own_public_byte, crypto_box_PUBLICKEYBYTES);
/*

	unsigned char encrypted[1000];
	memset(encrypted, '\0', 1000);

printf("%s - %d\n\n\n\n", sec_public_byte, CIPHERTEXT_LEN);

for (int i = 0; i < 8; i++) {
	crypto_box_seal(encrypted, MESSAGE, MESSAGE_LEN, sec_public_byte);
	sodium_bin2hex(dec, 1000, encrypted, CIPHERTEXT_LEN);
	printf("1) %s |-|-| %s\n\n\n", encrypted, dec);

}



	
	if (crypto_box_seal_open(decrypted, encrypted, CIPHERTEXT_LEN,
                         sec_public_byte, sec_secret_byte) != 0) {
		printf("error\n");
	}
	printf("2) %s\n\n", decrypted);
*/

	//char *hexmess = enc.encrypt("Hello world!7779 привет пидорас", "192.168.1.42");
	//unsigned char *sdf = illenc_hex2byte("a9fc5433432d5e6bf895913d8313e5eb3988297c8de51e101cdc6296e4749a7d2c96d7bed8364a8ba11345df39db8db30b40586f4fda049c44ac1810793e74d05048fd162f14265f08ae0e982accb683ea61f14fd10e4d626d9c1975b08cacb17878");
	//printf("aaaaaaaaaaa<<< %s\n\n|%ld - %d|\n\n", hexmess, strlen((char *)hexmess), crypto_box_SEALBYTES);

/*
	if (crypto_box_seal_open(decrypted, hexmess, MAXTEXTSIZE + crypto_box_SEALBYTES,
                         sec_public_byte, sec_secret_byte) != 0) {
		printf("error\n");
	}
	printf("2) %s\n\n", decrypted);
*/
	//printf("%s\n", enc.decrypt(hexmess));
	//unsigned char *decrtt = 


	char *hexxxxaaf = enc.encrypt("Hello bro, Hii lol!.", "192.168.1.42");
	printf("%s\n\n", enc.decrypt(hexxxxaaf));
	return 0;
}