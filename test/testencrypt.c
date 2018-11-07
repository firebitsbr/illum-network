#include "../src/illum.h"

int main()
{
	unsigned char *encryptm, *decryptm, *message;
	char *public, *private;
	struct illumencrypt encrypt;
	struct illumdb database;
	FILE *fp;

	message = "Test message for encryption!";
	fp = fopen("./log.txt", "a+");

	illum_dbinit(&database, "./illum.db", fp);
	illum_encryptinit(&encrypt, &database, fp);

	/* Keys of some person. */
	public = "a289a541ede4cec8ff7aaf5d2bbbd5527633f5995fd94d0cd95db3321beb5434";
	private = "b4f3b8405f3c62dc10ba37e27385f00a307c3299aca7d1ed8f2cf8b3f68c68cd";

	encryptm = encrypt.encrypt(message, public);
	printf("%s\n\n\n", encryptm);

	decryptm = encrypt.decrypt(encryptm);
	printf("%s\n", decryptm);
}