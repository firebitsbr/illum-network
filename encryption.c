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
static void illenc_rbytes(uint8_t [], long long);
static void illenc_genkeys(struct userkeys *);
static char *illenc_decrypt(char *, char *);
static char *illenc_encrypt(char *, char *);
static uint8_t *illenc_hex2byte(char *);
static char *illenc_publickey();
/**
*	Глобальные переменные
*/
static struct cryptobox_d cbl;
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
	illenc_cbconstants(&cbl);
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
	char *public, *secret;

	public = db->getvar("PUBLICKEY");
	secret = db->getvar("SECRETKEY");

	if (public != NULL && secret != NULL) {
		memcpy(keys->public, illenc_hex2byte(public), cbl.publickey);
		memcpy(keys->secret, illenc_hex2byte(secret), cbl.secretkey);
		goto exit_genkeys;
	}

	crypto_box_keypair(keys->public, keys->secret);
	public = illenc_byte2hex(keys->public, cbl.publickey);
	secret = illenc_byte2hex(keys->secret, cbl.secretkey);
	db->setvar("PUBLICKEY", public);
	db->setvar("SECRETKEY", secret);

exit_genkeys:
	free(public);
	free(secret);
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
	cbl->nonce = crypto_secretbox_NONCEBYTES;
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
/**
*	illenc_publickey - Функция возврата публичного ключа / логина.
*/
static char *illenc_publickey()
{
	return illenc_byte2hex(pkeys->public, cbl.publickey);
}
/**
*	illenc_decrypt - Функция разкодирования сообщения.
*
*	@hex - Hex строка.
*	@ipaddr - Ip адрес отвравителя.
*/
static char *illenc_decrypt(char *hex, char *ipaddr)
{
	uint8_t *message, *bpkey, nonce[8] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
	unsigned char *buffer;
	struct node_list node;

	if (!hex || hex == NULL || !ipaddr || ipaddr == NULL) {
		fprintf(errf, "Error: Can't decrypt message.\n");
		return NULL;
	}

	buffer = (unsigned char *)malloc(MAX_TEXTSIZE);
	memset(buffer, '\0', MAX_TEXTSIZE);
	message = illenc_hex2byte(hex);
	node = db->nodeinfo(ipaddr);

	if (node.hash == NULL)
		goto exit_decrypt;
	bpkey = illenc_hex2byte(node.hash);

	if (crypto_box_open_easy(buffer, message, MAX_TEXTSIZE,
		nonce, bpkey, pkeys->secret) != 0)
		fprintf(errf, "Error: Can't open cryptobox.\n");

exit_decrypt:
	if (message && message != NULL)
		free(message);
	if (bpkey && bpkey != NULL)
		free(bpkey);
	if (node.hash != NULL) {
		free(node.ipaddr);
		free(node.hash);
	}
	return (char *)buffer;
}
/**
*	illenc_encrypt - Функция кодирования сообщения.
*
*	@text - Строка кодирования.
*	@ipaddr - Ip адрес отвравителя.
*/
static char *illenc_encrypt(char *text, char *ipaddr)
{

}
/**
*	illenc_rbytes - Функция получения рандомных значений.
*
*	@buffer - Буфер для записи.
*	@size - Длина записи.
*/
static void illenc_rbytes(uint8_t buffer[], long long size)
{
	int fd, rc;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Error: Failed to open /dev/urandom.\n");
		return;
	}

	if ((rc = read(fd, buffer, size)) >= 0)
		close(fd);
}
