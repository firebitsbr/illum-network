/**
*	encryption.c - Функции для управления шифрованием
*	контента illum.	
*
*	@mrrva - 2018
*/
#include "./illum.h"
/**
*	Глобальные переменные.
*/
static struct illumkeys keys;
static struct illumdb *p_db;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static unsigned char *illum_encrypt(unsigned char *, char *);
static unsigned char *illum_decrypt(unsigned char *);
static char *illum_byte2hex(unsigned char *, int);
static unsigned char *illum_hex2byte(char *);
static void illum_genkeys(struct illumkeys *);
/**
*	illum_encrypt - Функция инициализации модуля
*	шифрации.
*
*	@encrypt - Главная управляющая структура.
*	@database - Указатель на структуру базы данных.
*	@fp - Файловый стрим для записи ошибок.
*/
bool illum_encryptinit(struct illumencrypt *encrypt,
	struct illumdb *database, FILE *fp)
{
	if (!encrypt || !(p_db = database)
		|| !(error = fp)) {
		printf("Error: Can't init encryptinit.\n");
		return false;
	}

	illum_genkeys(&keys);

	encrypt->hex2byte = illum_hex2byte;
	encrypt->byte2hex = illum_byte2hex;
	encrypt->decrypt = illum_decrypt;
	encrypt->encrypt = illum_encrypt;
	encrypt->keys = &keys;

	return true;
}
/**
*	illum_genkeys - Функция записи пар ключей шифрации.
*
*	@keys - Структура хранения ключей.
*/
static void illum_genkeys(struct illumkeys *keys)
{
	unsigned char *b_public = NULL, *b_secret = NULL;
	char *public, *secret;

	if (!keys || keys == NULL)
		return;

	public = p_db->get("PUBLICKEY");
	secret = p_db->get("SECRETKEY");

	if (public && secret) {
		if (!(b_public = illum_hex2byte(public))
			|| !(b_secret = illum_hex2byte(secret)))
			return;

		memcpy(keys->public, b_public, HASHSIZE);
		memcpy(keys->secret, b_secret, HASHSIZE);
		goto exit_genkeys;
	}

	crypto_box_keypair(keys->public, keys->secret);
	public = illum_byte2hex(keys->public, HASHSIZE);
	secret = illum_byte2hex(keys->secret, HASHSIZE);

	p_db->set("PUBLICKEY", public);
	p_db->set("SECRETKEY", secret);	

exit_genkeys:
	if (b_public != NULL)
		free(b_public);
	if (b_secret != NULL)
		free(b_secret);
#if DEBUG
	printf("Public key: %s\nSecret key: %s\n", public
		, secret);
#endif
	free(public);
	free(secret);
}
/**
*	illum_byte2hex - Функция преобразования байтового
*	массива в hex строку.
*
*	@bytes - Запись для перевода в hex.
*	@len - Длина записи.
*/
static char *illum_byte2hex(unsigned char *bytes, int len)
{
	char *hex;

	if (!bytes || bytes == NULL || len == 0
		|| len > TEXTSIZE)
		return NULL;

	hex = (char *)malloc(TEXTSIZE + 1);
	memset(hex, '\0', TEXTSIZE + 1);
	sodium_bin2hex(hex, TEXTSIZE, bytes, len);

	return hex;
}
/**
*	illum_hex2byte - Функция преобразования hex строки
*	в байтовый массив.
*
*	@string - Hex строка.
*/
static unsigned char *illum_hex2byte(char *string)
{
	unsigned int len, slen;
	unsigned char *data;

	if (!string || string == NULL
		|| (slen = strlen(string)) % 2 != 0)
		return NULL;

	len = slen / 2 + 1;
	data = (unsigned char *)malloc(len);
	sodium_hex2bin(data, len, string, slen, NULL,
					NULL, NULL);
	return data;
}
/**
*	illum_decrypt - Функция разкодирования сообщения.
*
*	@text - Шифрованная байт-строка.
*/
static unsigned char *illum_decrypt(unsigned char *text)
{
	unsigned char *buffer = NULL;
	unsigned int len;

	len = TEXTSIZE + crypto_box_SEALBYTES;

	if (!text || strlen((char *)text) > len)
		return NULL;

	buffer = (unsigned char *)malloc(len + 1);
	memset(buffer, '\0', len);

	if (crypto_box_seal_open(buffer, text, len, keys.public,
		keys.secret) != 0)
		fprintf(error, "Error: Can't decrypt message.\n");

	if (DEBUG)
		printf("%s\n", buffer);

	return buffer;
}
/**
*	illum_encrypt - Функция кодирования сообщения.
*
*	@text - Строка кодирования.
*	@pkey - Публичный ключ.
*/
static unsigned char *illum_encrypt(unsigned char *text,
	char *pkey)
{
	unsigned char *buffer = NULL, *phex;
	unsigned int len;
	
	if (!text || !pkey || strlen(pkey) < 7)
		return NULL;

	len = TEXTSIZE + 100 + crypto_box_SEALBYTES;

	if (strlen((char *)text) > TEXTSIZE)
		goto exit_encrypt;

	buffer = (unsigned char *)malloc(len);
	phex = illum_hex2byte(pkey);
	memset(buffer, '\0', len);

	if (phex != NULL)
		crypto_box_seal(buffer, text, TEXTSIZE, phex);

	if (DEBUG)
		printf("%s\n", buffer);

exit_encrypt:
	if (phex)
		free(phex);
	return buffer;
}