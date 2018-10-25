/**
*	functions.c - Главные функции управление сетью
	сетью illum.
*
*	@mrrva - 2018
*/
#include "./functions.h"
/**
*	Прототипы приватных функций
*/
static void illum_testrequest(enum illheader, char *);
static bool illum_initstructs(struct illfunctions *);
static bool illum_firstnode(char *);
static bool illum_sendstraight();
static bool illum_sendonion();
static bool illum_connect();
/**
*	Глобальные переменные
*/
static struct illfunctions fn;
static illum *ill;
static FILE *fp;
/**
*	illum_init - Функция инициализации управляющей
*	структуры сети.
*
*	@illum - Структура сети.
*	@rpath - Путь к каталогу ресурсов.
*/
void illum_init(illum *illum, char *rpath)
{
	if (!illum || illum == NULL || !rpath || rpath == NULL
		|| strlen(rpath) < 2 || rpath[strlen(rpath) - 1] != '/') {
		printf("Error: Incorrect params in illum_init.\n");
		return;
	}

	fn.errfile = (char *)malloc(strlen(rpath) + 11);
	sprintf(fn.errfile, "%slog.txt", rpath);
	fn.dbpath = (char *)malloc(strlen(rpath) + 11);
	sprintf(fn.dbpath, "%sillum.db", rpath);

	if (!(fp = fopen(fn.errfile, "a+"))) {
		printf("Error: Can't open error file.\n");
		return;
	}
	if (!illum_initstructs(&fn) || !(ill = illum)) {
		fprintf(fp, "Error: Can't init main structs.\n");
		return;
	}

	if (fn.db.nodenum() == 0)
		illum->firstnode = illum_firstnode;
	else
		illum->connect = illum_connect;
	illum->sendstraight = illum_sendstraight;
	illum->sendonion = illum_sendonion;
	illum->testrequest = illum_testrequest;
}
/**
*	illum_initstructs - Функция инициализации структур управления
*	модулями сети.
*
*	@func - Структура модулей.
*/
static bool illum_initstructs(struct illfunctions *func)
{
	if (!func || func == NULL)
		return false;

	if (!illdb_init(func->dbpath, &func->db, fp)) {
		fprintf(fp, "Error: Can't init illdb struct.\n");
		return false;
	}
	if (!illenc_init(&func->encrypt, &func->db, fp)) {
		fprintf(fp, "Error: Can't init illenc struct.\n");
		return false;
	}
	if (!illrouter_init(&func->router, &func->db,
						&func->encrypt, fp)) {
		fprintf(fp, "Error: Can't init illrouter struct.\n");
		return false;
	}
	if (!illsrv_init(&func->server, &func->db, &func->router,
					fp)) {
		fprintf(fp, "Error: Can't init illsrv struct.\n");
		return false;
	}

	return true;
}
/**
*	illum_firstnode - Функция подключения к первой ноде
*	сети для получения информации и свободных узлов.
*
*	@ipaddr - Адрес первой ноды.
*/
static bool illum_firstnode(char *ipaddr)
{
	bool status = false;

	if (!ipaddr || ipaddr == NULL) {
		fprintf(fp, "Error: Incorrect ip in illum_firstnode.\n");
		return status;
	}
	if (fn.db.nodenum() != 0) {
		fprintf(fp, "Error: You have some static nodes.\n");
		return status;
	}

	/*status = fn.server.setnode(ipaddr);
	if (status)
		fn.thrd = fn.server.start();*/
	fn.router.new(ILL_NEWNODE, ipaddr);
	fn.thrd = fn.server.start();
	sleep(40);

	return status;
}
/**
*	illum_connect - Функция подключения к сети с обновлением
*	всех нод.
*/
static bool illum_connect()
{
	/*if (fn.db.nodenum() == 0) {
		fprintf(fp, "Error: DB hasn't any static nodes.\n");
		return false;
	}*/

	fn.router.updnodes(false);
	fn.thrd = fn.server.start();

	return true;
}
/**
*	illum_testrequest - Тестовое сообщение ноде для проверки
*	построения маршрута.
*
*	@type - Тип отправляемого сообщения.
*	@ipaddr - Адрес первой ноды.
*/
static void illum_testrequest(enum illheader type, char *ipaddr)
{
	fn.router.new(type, ipaddr);
	//fn.thrd = fn.server.start();
	sleep(40);
}



static bool illum_sendstraight()
{
	return true;
}

static bool illum_sendonion()
{
	return true;
}