/**
*	functions.c - Главные функции для взаимодействия с
*	библиотекой сети.	
*
*	@mrrva - 2018
*/
#include "./functions.h"
/**
*	Прототипы приватных функций
*/
static bool illum_initstructs(char *);
static bool illum_firstnode(char *);
static unsigned int illum_connect();
/**
*	Глобальные переменные
*/
struct illfunc fn;
FILE *errf;
/**
*	illum_init - Функция инициализации сети и всех
*	сопутствующих управляющих указателей.
*
*	@pointer - Главная управляющая структура.
*	@path - Путь к каталогу ресурсов.
*/
bool illum_init(illum *pointer, char *path) 
{
	unsigned int length = 0;
	char *errfile, *dbpath;
	bool status = false;

	if (pointer || pointer == NULL || !path
		|| path == NULL || (length = strlen(path)) > 100) {
		printf("Error: Incorrect options in illum_init.\n");
		return status;
	}

	errfile = (char *)malloc(strlen(length + 100));
	dbpath = (char *)malloc(strlen(length + 100));
	sprintf(errfile, "%s/errors.log", path);
	sprintf(dbpath, "%s/illum.db", path);

	if (!(errf = fopen(errfile, "w"))) {
		printf("Error: Can't open %s.\n", errfile);
		goto exit_illum;
	}

	if (!illum_initstructs(dbpath))
		goto exit_illum;

	if (fn.db.nodenum() == 0)
		pointer->firstnode = illum_firstnode;
	pointer->message = illum_message;
	pointer->connect = illum_connect;
	pointer->onion = illum_onion;
	pointer->start = illum_start;

	if (pointer->firstnode && pointer->onionmessage
		&& pointer->message && pointer->connect)
		status = true;

exit_illum:
	free(errfile);
	free(dbpath);
	return status;
}
/**
*	illum_initstructs - Функция инициализации всех
*	управляющих структур.
*/
static bool illum_initstructs(char *dbpath)
{
	bool status = true;

	if (!illdb_init("./illum.db", &fn.db, errf)
		status = false;
	if (!illenc_init(&fn.encrypt, &fn.db, errf))
		status = false;
	if (!illrouter_init(&fn.router, &fn.db, &fn.encrypt,
		errf))
		status = false;
	if (!illsrv_init(&fn.server, &fn.db, &fn.router, errf))
		status = false;

	if (!status)
		fprintf(errf, "Error: Can't init main structs.\n");
	return status;
}
/**
*	illum_connect - Функция подключения к сети.
*/
static unsigned int illum_connect()
{
	if (fn.db.nodenum() == 0) {
		fprintf(errf, "Error: Node list is empty.\n");
		return 0; /* if we haven't nodes */
	}

	fn.router(false);
	return 1;
}
/**
*	illum_firstnode - Функция подключения к первой ноде
*	сети.
*
*	@ipaddr - Ip адрес первой ноды.
*/
static bool illum_firstnode(char *ipaddr)
{
	
}