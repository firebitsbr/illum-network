/**
*	database.c - Функции для управления базой данных
*	проекта.	
*
*	@mrrva - 2018
*/
#include "./illum.h"
/**
*	Приватные переменные и указатели.
*/
static sqlite3 *db;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static int illum_dbsettask(char *, char *, char *);
static bool illum_dbgettask(struct illumtask *);
static bool illum_dbissettask(char *, char *);
static bool illum_dbissetnode(char *, char *);
static bool illum_dbnewnode(char *, char *);
static void illum_dbgetvar(char *, char []);
static bool illum_dbsetvar(char *, char *);
static void illum_dbremovecache();
static bool illum_dbtables();
static int illum_dbnodenum();
/**
*	illum_dbinit - Функция инициализации подключения
*	к базе данных с поледующим создание структуры.
*
*	@filepath - Путь к базе данных.
*	@database - Главная управляющая структура.
*	@errfile - Файловый стрим для записи ошибок.
*/
bool illum_dbinit(struct illumdb *database, char *filepath,
	FILE *errfile)
{
	int status = -100;

	if (!filepath || strlen(filepath) < 8 || !database
		|| !(error = errfile)) {
		printf("Error: Incorrect args in dbinit.\n");
		return false;
	}
	if ((status = sqlite3_open(filepath, &db)) != SQLITE_OK
		|| !illum_dbtables()) {
		if (status != SQLITE_OK)
			fprintf(error, "Can't open database: %s\n",
				sqlite3_errmsg(db));
		return false;
	}
	illum_dbremovecache();

	database->issetnode = illum_dbissetnode;
	database->nodenum = illum_dbnodenum;
	database->gettask = illum_dbgettask;
	database->newtask = illum_dbsettask;
	database->newnode = illum_dbnewnode;
	database->setvar = illum_dbsetvar;
	database->getvar = illum_dbgetvar;

	return true;
}
/**
*	illum_dbnewnode - Функция занесения новой ноды в базу данных.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@hash - Ключ ноды для распознования в сети.
*/
static bool illum_dbnewnode(char *ipaddr, char *hash)
{
	sqlite3_stmt *rs = NULL;
	unsigned int length = 0;
	bool status = false;
	time_t usetime;
	char *sql;

	if (!ipaddr || strlen(ipaddr) < 7 || !hash
		|| strlen(hash) < 10) {
		fprintf(error, "Error: Incorrect params in newnode.\n");
		return status;
	}
	if (illum_dbissetnode(ipaddr, NULL)
		|| illum_dbnodenum() >= MAXNODES)
		return status;

	if ((length = strlen(ipaddr) + strlen(hash)) > 300)
		goto exit_newnode;

	usetime = time(NULL);
	sql = (char *)malloc(length + 100);
	sprintf(sql, "INSERT INTO `nodes` VALUES (NULL, '%s', '%s',"
		"'%ld');", ipaddr, hash, usetime);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(error, "Error: Can't set new node to db.\n");
		goto exit_newnode;
	}
	status = true;

exit_newnode:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	if (sql)
		free(sql);
	return status;
}
/**
*	illum_dbnodenum - Количество нод в базе данных.
*
*/
static int illum_dbnodenum()
{
	sqlite3_stmt *rs = NULL;
	int length = 0;

	sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM `nodes`;",
		-1, &rs, NULL);
	if (sqlite3_step(rs) == SQLITE_ROW)
		goto exit_nodenum;

	length = atoi((const char *)sqlite3_column_text(rs, 0));

exit_nodenum:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	return length;
}
/**
*	illum_dbgettask - Функция для получения текущего задания.
*
*	@task - Указатель на структуру задания.
*/
static bool illum_dbgettask(struct illumtask *task)
{
	sqlite3_stmt *rs = NULL;
	bool status = false;
	char *sql;

	if (!task) {
		fprintf(error, "Error: Incorrect arg in dbgettask.\n");
		return false;
	}

	sqlite3_prepare_v2(db, "SELECT * FROM `tasks` LIMIT 1", -1,
		&rs, NULL);
	if (sqlite3_step(rs) != SQLITE_ROW)
		goto exit_gettask;

	task->id = atoi((const char *)sqlite3_column_text(rs, 0));
	task->headers = (char *)sqlite3_column_text(rs, 3);
	task->ipaddr = (char *)sqlite3_column_text(rs, 1);
	task->text = (char *)sqlite3_column_text(rs, 2);

	if (!DBDEBUG) {
		sql = (char *)malloc(1000);
		sprintf(sql, "DELETE FROM `tasks` WHERE `id`='%d'",
			task->id);

		sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
		sqlite3_step(rs);
		free(sql);
	}
	status = true;

exit_gettask:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	return status;
}
/**
*	illum_dbissetnode - Функция проверки существования ноды в бд.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@hash - Ключ ноды для распознования в сети.
*/
static bool illum_dbissetnode(char *ipaddr, char *hash)
{
	bool status = true, type;
	sqlite3_stmt *rs = NULL;
	unsigned int len = 0;
	char *sql, *value;

	if (ipaddr && ipaddr != NULL && strlen(ipaddr) > 6
		&& strlen(ipaddr) < 100) {
		value = ipaddr;
		type = true;
	}
	else if (hash && hash != NULL && strlen(hash) < 101) {
		value = hash;
		type = false;
	}
	else {
		fprintf(error, "Error: Incorrect args in issetnode.\n");
		return status;
	}
	if ((len = strlen(value)) > 200) 
		return status;

	sql = (char *)malloc(len + 100);
	sprintf(sql, "SELECT COUNT(*) FROM `nodes` WHERE `%s`='%s'",
		((type) ? "ip" : "hash"), value);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) == SQLITE_ROW
		&& atoi((const char *)sqlite3_column_text(rs, 0)) == 0)
		status = false;

	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);

	return status;
}
/**
*	illum_dbgetvar - Функция получения переменной из таблицы
*	настроек.
*
*	@name - Имя извлекаемой	переменной.
*	@buffer - Буфер записи значения.
*/
static void illum_dbgetvar(char *name, char buffer[])
{
	sqlite3_stmt *rs = NULL;
	char *sql, *var = NULL;
	unsigned int len = 0;

	if (!name || (len = strlen(name)) < 2 || len > 20) {
		fprintf(error, "Error: Incorrect option in getvar.\n");
		goto exit_getvar;
	}

	sql = (char *)malloc(len + 100);
	sprintf(sql, "SELECT `value` FROM `settings` WHERE `name`="
		"'%s'", name);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_ROW) {
		fprintf(error, "Warring: Can't get %s from db.\n", name);
		goto exit_getvar;
	}

	var = (char *)sqlite3_column_text(rs, 0);
	memset(buffer, '\0', 101);
	memcpy(buffer, var, 100);

exit_getvar:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);
}
/**
*	illum_dbissettask - Функция проверяет одинаковые запросы в бд.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@headers - Заголовки к сообщению.
*/
static bool illum_dbissettask(char *ipaddr, char *headers)
{
	sqlite3_stmt *rs = NULL;
	unsigned int len = 0;
	bool status = true;
	char *sql;

	if (!ipaddr || ipaddr == NULL || !headers
		|| headers == NULL) {
		fprintf(error, "Error: Incorrect args in issettask\n");
		return status;
	}
	if ((len = strlen(ipaddr) + strlen(headers)) > 600)
		return status;

	sql = (char *)malloc(len + 100);
	sprintf(sql, "SELECT COUNT(*) FROM `tasks` WHERE `ip`='%s' "
		"AND `headers`='%s';", ipaddr, headers);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) == SQLITE_ROW
		&& atoi((const char *)sqlite3_column_text(rs, 0)) == 0)
		status = false;

	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);

	return status;
}
/**
*	illum_dbsettask - Функция создания новой задачи.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@text - Текст сообщения.
*	@headers - Заголовки к сообщению.
*/
static int illum_dbsettask(char *ipaddr, char *text, char *headers)
{
	sqlite3_stmt *rs = NULL;
	unsigned int length = 0;
	int id = -1;
	char *sql;

	if (!ipaddr || strlen(ipaddr) < 7
		|| !headers || strlen(headers) < 8) {
		fprintf(error, "Error: Incorrect args in settask.\n");
		return id;
	}
	if (illum_dbissettask(ipaddr, headers)) 
		return id;

	if (text && text != NULL)
		length += strlen(text);
	if ((length += strlen(ipaddr) + strlen(headers)) > MAXTEXTSIZE) {
		fprintf(error, "Error: Text size more then you can write.\n");
		return id;
	}

	sql = (char *)malloc(length + 100);
	sprintf(sql, "INSERT INTO `tasks` VALUES (NULL, '%s', '%s', '%s');",
		ipaddr, ((!text || text == NULL) ? "" : text), headers);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(error, "Error: Can't create new task.\n");
		goto exit_settask;
	}

	sqlite3_prepare_v2(db, "SELECT last_insert_rowid();", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE)
		goto exit_settask;
	id = atoi((const char *)sqlite3_column_text(rs, 0));

exit_settask:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);
	return id; /* -1 is error */
}
/**
*	illum_dbsetvar - Функция записи значения в таблицу
*	настроек.
*
*	@name - Ip адрес получателя сообщения.
*	@value - Значение для записи.
*/
static bool illum_dbsetvar(char *name, char *value)
{
	unsigned int len1 = 0, len2 = 0;
	sqlite3_stmt *rs = NULL;
	bool status = false;
	char *sql;

	if (!name || (len1 = strlen(name)) < 2 || len1 > 20
		|| !value || (len2 = strlen(value)) < 2
		|| len2 > 101) {
		fprintf(error, "Error: Incorrect option in getvar.\n");
		return status;
	}

	sql = (char *)malloc(len1 + len2 + 100);
	sprintf(sql, "DELETE FROM `settings` WHERE `name`='%s'",
		name);
	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(error, "Error: Can't remove data from settings.\n");
		goto exit_setvar;
	}

	memset(sql, '\0', strlen(sql));
	sprintf(sql, "INSERT INTO `settings` VALUES ('%s', '%s');",
		name, value);
	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(error, "Error: Can't set data in settings.\n");
		goto exit_setvar;
	}

	status = true;

exit_setvar:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);

	return status;
}
/**
*	illdb_removecache - Функция удаления лишних записей из базы.
*
*/
static void illum_dbremovecache()
{
	sqlite3_stmt *rs = NULL;

	sqlite3_prepare_v2(db, "DELETE FROM `onion`", -1,
		&rs, NULL);
	sqlite3_step(rs);

	sqlite3_prepare_v2(db, "DELETE FROM `tasks`", -1,
		&rs, NULL);
	sqlite3_step(rs);

	if (rs && rs != NULL)
		sqlite3_finalize(rs);
}
/**
*	illum_dbtables - Функция создания базы данных проекта.
*
*/
static bool illum_dbtables()
{
	sqlite3_stmt *rs = NULL;
	bool status = false;

	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `tasks` ("
	"`id` INTEGER PRIMARY KEY AUTOINCREMENT, `ip` text NOT NULL,"
	"`text` text, `headers` text NOT NULL);", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(error, "Error: Sqlite can't create tasks table.\n");
		goto exit_tables;
	}
	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `nodes` ("
	"`id` INTEGER PRIMARY KEY AUTOINCREMENT, `ip` text NOT NULL,"
	"`hash` text NOT NULL, `use_t` int(15) NOT NULL);",
	-1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(error, "Error: Sqlite can't create nodes table.\n");
		goto exit_tables;
	}
	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `settings` ("
	"`name` text NOT NULL, `value` text NOT NULL);", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(error, "Error: Sqlite can't create settings table.\n");
		goto exit_tables;
	}
	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `onion` ("
	"`hash` text NOT NULL, `from` text NOT NULL, `to` text NOT NULL);",
	-1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(error, "Error: Sqlite can't create onion table.\n");
		goto exit_tables;
	}
	status = true;

exit_tables:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	return status;
}