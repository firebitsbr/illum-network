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
static char *illenc_byte2hex(uint8_t [], unsigned int);
static void illenc_cbconstants(struct cryptobox_d *);
static struct userkeys illenc_genkeys();
static uint8_t *illenc_hex2byte(char *);
/**
*	Глобальные переменные
*/
static struct cryptobox_d cbl;
static sqlite3 *db;
static FILE *errf;
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

	illenc_cbconstants(&cbl);
	enc->keys = illenc_getkeys();

	enc->hex2byte = illenc_hex2byte;
	enc->byte2hex = illenc_byte2hex;

	if (enc->hex2byte && enc->byte2hex)
		status = true;

	return status;
}
/**
*	illenc_getkeys - Функция записи пар ключей шифрации.
*/
static struct userkeys illenc_getkeys()
{
	char *public, *secret;
	struct userkeys keys;

	if ((public = db->getvar("PUBLICKEY")) != NULL
		&& (secret = db->getvar("SECRETKEY")) != NULL) {
		keys->public = illenc_hex2byte(public);
		keys->secret = illenc_hex2byte(secret);

		goto exit_getkeys;
	}

	crypto_box_keypair(keys->public, keys->secret);
	public = illenc_byte2hex(keys->public, cbl.publickey);
	secret = illenc_byte2hex(keys->secret, cbl.secretkey);

	db->setkey("PUBLICKEY", public);
	db->setkey("SECRETKEY", secret);

exit_getkeys:
	free(public);
	free(secret);
	return keys;
}
/**
*	illenc_cbconstants - Функция записи констант от crypto
*	box в структуру.
*
*	@cbl - Структура констант cryptobox.
*/
static void illenc_cbconstants(struct cryptobox_d *cbl)
{
	cbl->publickey = crypto_box_PUBLICKEYBYTES;
	cbl->secretkey = crypto_box_SECRETKEYBYTES;
}
/**
*	illenc_byte2hex - Функция преобразования байтового
*	массива в hex строку.
*
*	@bin - Битовый массив.
*	@length - Длина буфера.
*/
static char *illenc_byte2hex(uint8_t bin[], unsigned int length)
{
	uint8_t *p0 = (uint8_t *)bin;
	char *hex;

	for (int i = 0; i < length; i++) {
		snprintf(hex, 3, "%02x", *p0);
		p0 += 1;
		hex += 2;
	}

	return hex;
}
/**
*	illenc_hex2byte - Функция преобразования hex строки
*	в байтовый массив.
*
*	@string - Hex строка.
*/
static uint8_t *illenc_hex2byte(char *string)
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