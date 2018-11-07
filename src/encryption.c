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
static struct userkeys *keys;
static struct illumdb *db;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static unsigned char *illum_encrypt(unsigned char *, char *);
static unsigned char *illum_decrypt(unsigned char *);
static char *illum_byte2hex(unsigned char *, int);
static unsigned char *illum_hex2byte(char *);
static void illum_genkeys(struct userkeys *);
static char *illum_publickey();
/**
*	illum_encryptinit - Функция инициализации модуля
*	шифрации.
*
*	@encrypt - Главная управляющая структура.
*	@database - Указатель на структуру базы данных.
*	@errfile - Файловый стрим для записи ошибок.
*/
bool illum_encryptinit(struct illumencrypt *encrypt,
	struct illumdb *database, FILE *errfile)
{
	if (!encrypt || !(db = database)
		|| !(error = errfile)) {
		printf("Error: Can't init encryptinit.\n");
		return false;
	}

	illum_genkeys(&encrypt->keys);
	keys = &encrypt->keys;

	encrypt->publickey = illum_publickey;
	encrypt->hex2byte = illum_hex2byte;
	encrypt->byte2hex = illum_byte2hex;
	encrypt->decrypt = illum_decrypt;
	encrypt->encrypt = illum_encrypt;

	return true;
}
/**
*	illum_genkeys - Функция записи пар ключей шифрации.
*
*	@keys - Структура хранения ключей.
*/
static void illum_genkeys(struct userkeys *keys)
{
	unsigned char *byte_public = NULL,
		*byte_secret = NULL;
	char *public, *secret;

	if (!keys || keys == NULL)
		return;

	public = (char *)malloc(101);
	secret = (char *)malloc(101);
	db->getvar("PUBLICKEY", public);
	db->getvar("SECRETKEY", secret);

	if (strlen(public) + strlen(secret) > 100) {
		if (!(byte_public = illum_hex2byte(public))
			|| !(byte_secret = illum_hex2byte(secret)))
			return;

		memcpy(keys->public, byte_public, 
				crypto_box_PUBLICKEYBYTES);
		memcpy(keys->secret, byte_secret, 
				crypto_box_SECRETKEYBYTES);
		goto exit_genkeys;
	}

	crypto_box_keypair(keys->public, keys->secret);

	memcpy(public, illum_byte2hex(keys->public,
		crypto_box_PUBLICKEYBYTES), 100);
	memcpy(secret, illum_byte2hex(keys->secret,
		crypto_box_SECRETKEYBYTES), 100);

	db->setvar("PUBLICKEY", public);
	db->setvar("SECRETKEY", secret);

exit_genkeys:
	if (byte_public != NULL)
		free(byte_public);
	if (byte_secret != NULL)
		free(byte_secret);
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
		|| len > MAXTEXTSIZE)
		return NULL;

	hex = (char *)malloc(MAXTEXTSIZE + 1);
	memset(hex, '\0', MAXTEXTSIZE + 1);
	sodium_bin2hex(hex, MAXTEXTSIZE, bytes, len);

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

	len = MAXTEXTSIZE + crypto_box_SEALBYTES;

	if (!text || strlen((char *)text) > len)
		return NULL;

	buffer = (unsigned char *)malloc(len + 1);
	memset(buffer, '\0', len);

	if (crypto_box_seal_open(buffer, text, len, keys->public,
		keys->secret) != 0)
		fprintf(error, "Error: Can't decrypt message.\n");

	if (ENCDEBUG)
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

	len = MAXTEXTSIZE + 100 + crypto_box_SEALBYTES;

	if (strlen((char *)text) > MAXTEXTSIZE)
		goto exit_encrypt;

	buffer = (unsigned char *)malloc(len);
	phex = illum_hex2byte(pkey);
	memset(buffer, '\0', len);

	if (phex != NULL)
		crypto_box_seal(buffer, text, MAXTEXTSIZE, phex);

	if (ENCDEBUG)
		printf("%s\n", buffer);

exit_encrypt:
	if (phex)
		free(phex);
	return buffer;
}
/**
*	illum_publickey - Функция возврата публичного ключа / логина.
*
*/
static char *illum_publickey()
{
	return illum_byte2hex(keys->public,
			crypto_box_PUBLICKEYBYTES);
}