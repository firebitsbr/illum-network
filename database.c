/**
*	database.c - Функции для управления базой данных
*	проекта.	
*
*	@mrrva - 2018
*/
#include "./database.h"
/**
*	Прототипы приватных функций
*/
static bool illdb_issetnode(char *, char *, unsigned int, FILE *);
static bool	illdb_newnode(char *, char *, unsigned int, FILE *);
static int	illdb_settask(char *, char *, char *, FILE *);
struct node_list *illdb_nodelist(unsigned int *, FILE *);
static void illdb_currenttask(struct stask *, FILE *);
static bool illdb_issettask(char *, char *, FILE *);
static bool	illdb_removetask(unsigned int, FILE *);
static void illdb_staticnode(char *, FILE *);
static int	illdb_nodenum(FILE *);
static bool	illdb_tables(FILE *);
static void illdb_removecache();
/**
*	Глобальные переменные
*/
static sqlite3 *db;
/**
*	illdb_init - Функция инициализации подключения
*	к базе данных с поледующим создание структуры.
*
*	@dbpath - Путь к базе данных.
*	@dbstruct - Главная управляющая структура.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illdb_init(char *dbpath, illdb *dbstruct, FILE *errf)
{
	int st_sqlite3 = -100;
	bool status = false;

	if (!dbpath || strlen(dbpath) < 8 || !dbstruct || !errf) {
		printf("Error: Input params in illdb_init are incorrect.\n");
		return status;
	}
	if ((st_sqlite3 = sqlite3_open(dbpath, &db)) != SQLITE_OK
		|| !illdb_tables(errf)) {
		if (st_sqlite3 != SQLITE_OK)
			fprintf(errf, "Can't open database: %s\n", sqlite3_errmsg(db));
		return status;
	}
	illdb_removecache();

	dbstruct->currenttask = illdb_currenttask;
	dbstruct->staticnode = illdb_staticnode;
	dbstruct->removetask = illdb_removetask;
	dbstruct->nodelist = illdb_nodelist;
	dbstruct->newtask = illdb_settask;
	dbstruct->newnode = illdb_newnode;
	dbstruct->nodenum = illdb_nodenum;
	dbstruct->isset_node = illdb_issetnode;

	if (dbstruct->removetask && dbstruct->nodelist
		&& dbstruct->newtask && dbstruct->newnode
		&& dbstruct->currenttask && dbstruct->nodenum
		&& dbstruct->staticnode && dbstruct->isset_node)
		status = true;

	return status;
}
/**
*	illdb_removetask - Функция удаления задания из базы данных.
*
*	@id - Id задания в базе данных.
*	@errf - Файловый стрим для записи ошибок.
*/
static bool illdb_removetask(unsigned int id, FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	char *sql = (char *)malloc(200);
	bool status = false;

	if (id > 2147483647) {
		fprintf(errf, "Error: Invalid id in illdb_removetask.\n");
		goto exit_removetask;
	}

	sprintf(sql, "UPDATE `tasks` SET `status`='1' WHERE `id`='%d';",
			id);
	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_OK)
		goto exit_removetask;
	status = true;

exit_removetask:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);
	return status;
}
/**
*	illdb_staticnode - Функция подтверждения статического подключения
*	между нодами.
*
*	@hash - Хэш ноды.
*	@errf - Файловый стрим для записи ошибок.
*/
static void illdb_staticnode(char *hash, FILE *errf)
{
	sqlite3_stmt *rs;
	char *sql;

	if (!hash || strlen(hash) < 10 || strlen(hash) > 100) {
		fprintf(errf, "Error: Incorrest hash.\n");
		goto exit_staticnode;
	}

	sql = (char *)malloc(100 + strlen(hash));
	sprintf(sql, "UPDATE `nodes` SET `status`='1' WHERE "
		"`hash`='%s'", hash);
	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	sqlite3_step(rs);

exit_staticnode:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	if (sql && sql != NULL)
		free(sql);
	return;
}
/**
*	illdb_removecache - Функция удаления лишних записей из базы.
*/
static void illdb_removecache()
{
	sqlite3_stmt *rs = NULL;

	sqlite3_prepare_v2(db, "DELETE FROM `onion`", -1, &rs, NULL);
	sqlite3_step(rs);
	sqlite3_prepare_v2(db, "DELETE FROM `nodes` WHERE `status`='0'",
		-1, &rs, NULL);
	sqlite3_step(rs);

	if (rs && rs != NULL)
		sqlite3_finalize(rs);
}
/**
*	illdb_issetnode - Функция проверки существования ноды в бд.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@hash - Ключ ноды для распознования в сети.
*	@id - Тип фильтрации ноды.
*	@errf - Файловый стрим для записи ошибок.
*/
static bool illdb_issetnode(char *ipaddr, char *hash,
	unsigned int id, FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	unsigned int len = 0;
	bool status = true;
	char *sql, *stat = "\0";

	if (!ipaddr || ipaddr == NULL || !hash || hash == NULL || id < 0) {
		fprintf(errf, "Error: Incorrect params in illdb_issetnode.\n");
		return status;
	}
	if ((len = strlen(ipaddr) + strlen(hash)) > 200) 
		return status;

	if (id != 2)
		stat = (id == 0) ? " AND `status`='0'" : " AND `status`='1'";

	sql = (char *)malloc(len + 100);
	sprintf(sql, "SELECT COUNT(*) FROM `nodes` WHERE `hash`='%s'%s",
		hash, stat);
printf("%s %s - %s\n", ipaddr, hash, sql);
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
*	illdb_newnode - Функция занесения новой ноды в базу данных.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@hash - Ключ ноды для распознования в сети.
*	@text - Текст сообщения.
*	@mseconds - Время отклика узла.
*	@errf - Файловый стрим для записи ошибок.
*/
static bool illdb_newnode(char *ipaddr, char *hash,
	unsigned int mseconds, FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	unsigned int length = 0;
	bool status = false;
	char *sql;

	if (!ipaddr || strlen(ipaddr) < 7 || !hash || strlen(hash) < 10
		|| mseconds < 0) {
		fprintf(errf, "Error: Incorrect input data in illdb_newnode.\n");
		return status;
	}printf("2\n");
	if (illdb_issetnode(ipaddr, hash, 2, errf))
		return status;

	if ((length = strlen(ipaddr) + strlen(hash)) > MAX_TEXTSIZE) {
		fprintf(errf, "Error: Text size for task more than you can write.\n");
		goto exit_newnode;
	}
	sql = (char *)malloc(length + 100);
	sprintf(sql, "INSERT INTO `nodes` VALUES (NULL, '%s', '%s', '%i', 0);",
			ipaddr, hash, mseconds);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Can't set new node to database.\n");
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
*	illdb_currenttask - Функция извлекающая текущее задание для
*	сервера.
*
*	@errf - Файловый стрим для записи ошибок.
*/
static void illdb_currenttask(struct stask *data, FILE *errf)
{
	sqlite3_stmt *rs = NULL;

	if (!data || data == NULL) {
		data->id = 0;
		fprintf(errf, "Error: Incorrect pointer in "
			"illdb_currenttask.\n");
		goto exit_currenttask;
	}

	sqlite3_prepare_v2(db, "SELECT * FROM `tasks` WHERE `status`='0' "
		"ORDER BY `id` DESC LIMIT 1", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_ROW) {
		data->id = 0;
		goto exit_currenttask;
	}

	data->id = atoi((const char *)sqlite3_column_text(rs, 0));
	data->headers = (char *)sqlite3_column_text(rs, 3);
	data->ipaddr = (char *)sqlite3_column_text(rs, 1);
	data->text = (char *)sqlite3_column_text(rs, 2);

exit_currenttask:
	if (rs && rs != NULL)
		sqlite3_finalize(rs); // id = 0; when was error or tasklist is empty
}	
/**
*	illdb_nodelist - Функция извлекающая из базы список всех нод.
*
*	@num - Количество нод в базе.
*	@errf - Файловый стрим для записи ошибок.
*/
struct node_list *illdb_nodelist(unsigned int *num, FILE *errf)
{
	struct node_list *data = NULL;
	sqlite3_stmt *rs = NULL;
	int i = -1, tmp = 0;

	if ((*num = illdb_nodenum(errf)) < 1) {
		*num = 0;
		goto exit_nodelist;
	}

	data = (struct node_list *)malloc(sizeof(struct node_list) * (*num));
	sqlite3_prepare_v2(db, "SELECT * FROM `nodes` WHERE `status`='1'",
		-1, &rs, NULL);

	while (i++, sqlite3_step(rs) == SQLITE_ROW) {
		if (i > (*num) - 1)
			break;

		data[i].id = atoi((const char *)sqlite3_column_text(rs, 0));
		data[i].ipaddr = (char *)malloc(16);
		data[i].hash = (char *)malloc(100);

		if ((tmp = strlen((const char *)sqlite3_column_text(rs, 1)))
			<= 15)
			memcpy(data[i].ipaddr, sqlite3_column_text(rs, 1), tmp + 1);
		if ((tmp = strlen((const char *)sqlite3_column_text(rs, 2)))
			<= 99)
			memcpy(data[i].hash, sqlite3_column_text(rs, 2), tmp + 1);
	}

exit_nodelist:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	return data;
}
/**
*	illdb_issettask - Функция проверяет одинаковые запросы в бд.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@headers - Заголовки к сообщению.
*	@errf - Файловый стрим для записи ошибок.
*/
static bool illdb_issettask(char *ipaddr, char *headers, FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	unsigned int len = 0;
	bool status = true;
	char *sql;

	if (!ipaddr || ipaddr == NULL || !headers || headers == NULL) {
		fprintf(errf, "Error: Incorrect params in issettask\n");
		return status;
	}
	if ((len = strlen(ipaddr) + strlen(headers)) > 600)
		return status;

	sql = (char *)malloc(len + 100);
	sprintf(sql, "SELECT COUNT(*) FROM `tasks` WHERE `ip`='%s' "
		"AND `headers`='%s' AND `status`='0';", ipaddr, headers);

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
*	illdb_settask - Функция создания новой задачи.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@text - Текст сообщения.
*	@headers - Заголовки к сообщению.
*	@errf - Файловый стрим для записи ошибок.
*/
static int illdb_settask(char *ipaddr, char *text, char *headers, 
	FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	unsigned int length = 0;
	int id = -1;
	char *sql;

	if (!ipaddr || strlen(ipaddr) < 7 || !headers || strlen(headers) < 8) {
		fprintf(errf, "Error: Incorrect input data in illdb_settask.\n");
		return id;
	}

	if (illdb_issettask(ipaddr, headers, errf)) 
		return id;

	if (text && text != NULL)
		length += strlen(text);
	if ((length += strlen(ipaddr) + strlen(headers)) > MAX_TEXTSIZE) {
		fprintf(errf, "Error: Text size for task more then you can write.\n");
		return id;
	}

	sql = (char *)malloc(length + 100);
	sprintf(sql, "INSERT INTO `tasks` VALUES (NULL, '%s', '%s', '%s', 0);",
		ipaddr, ((!text || text == NULL) ? "" : text), headers);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Can't create new task.\n");
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
*	illdb_nodenum - Функция подсчета количества нод.
*
*	@errf - Файловый стрим для записи ошибок.
*/
static int illdb_nodenum(FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	int number = 0;

	sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM `nodes` WHERE `status`='1'",
		-1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_ROW) {
		fprintf(errf, "Error: Can't select number of nodes\n");
		goto exit_nodenum;
	}
	number = atoi((const char *)sqlite3_column_text(rs, 0));

exit_nodenum:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	return number;
}
/**
*	illdb_tables - Функция создания базы данных проекта.
*
*	@errf - Файловый стрим для записи ошибок.
*/
static bool illdb_tables(FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	bool status = false;

	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `tasks` ("
	"`id` INTEGER PRIMARY KEY AUTOINCREMENT, `ip` text NOT NULL,"
	"`text` text, `headers` text NOT NULL, `status` int(11) NOT "
	"NULL DEFAULT 0);", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Sqlite can't create tasks table.\n");
		goto exit_tables;
	}
	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `response` ("
	"`id` INTEGER PRIMARY KEY AUTOINCREMENT,`id_tsk` int(11) NOT NULL,"
	"`text` text NOT NULL);", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Sqlite can't create response table.\n");
		goto exit_tables;
	}
	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `nodes` ("
	"`id` INTEGER PRIMARY KEY AUTOINCREMENT, `ip` text NOT NULL,"
	"`hash` text NOT NULL, `mseconds` int(11) NOT NULL, `status`"
	"int(11) NOT NULL DEFAULT 0);", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Sqlite can't create nodes table.\n");
		goto exit_tables;
	}
	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `settings` ("
	"`name` text NOT NULL, `value` text NOT NULL);", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Sqlite can't create settings table.\n");
		goto exit_tables;
	}
	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `onion` ("
	"`hash` text NOT NULL, `from` text NOT NULL, `to` text NOT NULL);",
	-1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Sqlite can't create onion table.\n");
		goto exit_tables;
	}
	status = true;

exit_tables:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	return status;
}
/**
*	dbstruct - Функция освобождения памяти из под
*	структуры illdb.
*
*	@dbstruct - Главная управляющая структура.
*/
void illdb_free(illdb *dbstruct)
{
	// Perhaps here will be more code then now :)
	sqlite3_close(db);
}
