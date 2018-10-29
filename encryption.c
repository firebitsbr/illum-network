/**
*	encryption.c - Функции для управления шифрованием
*	контента illum.	
*
*	@mrrva - 2018
*/
#include "./encryption.h"
/**
*	Прототипы приватных функций
*/
static unsigned char *illenc_encrypt(unsigned char *, char *);
static unsigned char *illenc_decrypt(unsigned char *);
static char *illenc_byte2hex(unsigned char *, int);
static unsigned char *illenc_hex2byte(char *);
static void illenc_genkeys(struct userkeys *);
static char *illenc_publickey();
/**
*	Глобальные переменные
*/
struct userkeys *pkeys;
static FILE *errf;
static illdb *db;
/**
*	illenc_init - Функция инициализации модуля
*	шифрации.
*
*	@enc - Главная управляющая структура.
*	@dbstruct - Указатель на структуру базы данных.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illenc_init(illenc *enc, illdb *dbstruct, FILE *errfile)
{
	bool status = false;

	if (!(db = dbstruct) || !(errf = errfile)) {
		printf("Error: Can't init illenc_init.\n");
		return status;
	}

	illenc_genkeys(&enc->keys);
	pkeys = &enc->keys;

	enc->publickey = illenc_publickey;
	enc->hex2byte = illenc_hex2byte;
	enc->byte2hex = illenc_byte2hex;
	enc->decrypt = illenc_decrypt;
	enc->encrypt = illenc_encrypt;

	if (enc->hex2byte && enc->byte2hex && enc->publickey
		&& enc->decrypt && enc->encrypt)
		status = true;

	return status;
}
/**
*	illenc_genkeys - Функция записи пар ключей шифрации.
*
*	@keys - Структура хранения ключей.
*/
static void illenc_genkeys(struct userkeys *keys)
{
	unsigned char *byte_public = NULL, *byte_secret = NULL;
	char *public, *secret;

	if (!keys || keys == NULL)
		return;

	public = (char *)malloc(101);
	secret = (char *)malloc(101);

	db->getvar("PUBLICKEY", public);
	db->getvar("SECRETKEY", secret);

	if (strlen(public) > 10 && strlen(secret) > 10) {
		byte_public = illenc_hex2byte(public);
		byte_secret = illenc_hex2byte(secret);

		if (!byte_public || !byte_secret)
			return;
		memcpy(keys->public, byte_public, 
				crypto_box_PUBLICKEYBYTES);
		memcpy(keys->secret, byte_secret, 
				crypto_box_SECRETKEYBYTES);

		goto exit_genkeys;
	}

	crypto_box_keypair(keys->public, keys->secret);
	memcpy(public, illenc_byte2hex(keys->public,
		crypto_box_PUBLICKEYBYTES), 100);
	memcpy(secret, illenc_byte2hex(keys->secret,
		crypto_box_SECRETKEYBYTES), 100);

	db->setvar("PUBLICKEY", public);
	db->setvar("SECRETKEY", secret);

exit_genkeys:
	if (byte_public != NULL && byte_secret != NULL) {
		free(byte_public);
		free(byte_secret);
	}
	free(public);
	free(secret);
}
/**
*	illenc_byte2hex - Функция преобразования байтового
*	массива в hex строку.
*
*	@bytes - Запись для перевода в hex.
*	@len - Длина записи.
*/
static char *illenc_byte2hex(unsigned char *bytes, int len)
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
*	illenc_hex2byte - Функция преобразования hex строки
*	в байтовый массив.
*
*	@string - Hex строка.
*/
static unsigned char *illenc_hex2byte(char *string)
{
	int length, value = 0;
	unsigned char *data;
	bool flag = false;

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
/**
*	illenc_decrypt - Функция разкодирования сообщения.
*
*	@text - Шифрованная байт-строка.
*/
static unsigned char *illenc_decrypt(unsigned char *text)
{
	unsigned char *buffer = NULL;
	unsigned int len;

	len = MAXTEXTSIZE + crypto_box_SEALBYTES;

	if (!text || text == NULL || strlen((char *)text) > len)
		return NULL;

	buffer = (unsigned char *)malloc(len + 1);
	memset(buffer, '\0', len);

	if (crypto_box_seal_open(buffer, text, len, pkeys->public,
		pkeys->secret) != 0)
		fprintf(errf, "Error: Can't decrypt message.\n");
	
	return buffer;
}
/**
*	illenc_encrypt - Функция кодирования сообщения.
*
*	@text - Строка кодирования.
*	@ipaddr - Ip адрес отвравителя.
*/
static unsigned char *illenc_encrypt(unsigned char *text,
	char *ipaddr)
{
	unsigned char *buffer = NULL, *pkey;
	struct node_list node;
	unsigned int len;
	
	if (!text || text == NULL || !ipaddr || strlen(ipaddr) < 7)
		return NULL;

	len = MAXTEXTSIZE + 100 + crypto_box_SEALBYTES;
	node = db->nodeinfo(ipaddr);

	if (strlen((char *)text) > MAXTEXTSIZE
		|| !node.ipaddr || !node.hash)
		goto exit_encrypt;

	buffer = (unsigned char *)malloc(len);
	pkey = illenc_hex2byte(node.hash);
	memset(buffer, '\0', len);

	crypto_box_seal(buffer, text, MAXTEXTSIZE, pkey);

exit_encrypt:
	if (pkey)
		free(pkey);
	return buffer;
}
/**
*	illenc_publickey - Функция возврата публичного ключа / логина.
*/
static char *illenc_publickey()
{
	return illenc_byte2hex(pkeys->public,
			crypto_box_PUBLICKEYBYTES);
}
