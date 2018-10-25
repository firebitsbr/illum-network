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
static bool illdb_issetnode(char *, char *, unsigned int);
static struct node_list *illdb_nodelist(unsigned int *);
static int	illdb_settask(char *, char *, char *);
static struct node_list illdb_nodeinfo(char *);
static void illdb_currenttask(struct stask *);
static bool illdb_issettask(char *, char *);
static bool	illdb_removetask(unsigned int);
static void illdb_getvar(char *, char[]);
static bool	illdb_newnode(char *, char *);
static bool illdb_setvar(char *, char *);
static void illdb_staticnode(char *);
static void illdb_removecache();
static void illdb_ping(char *);
static int	illdb_nodenum();
static bool	illdb_tables();
/**
*	Глобальные переменные
*/
static sqlite3 *db;
static FILE *errf;
/**
*	illdb_init - Функция инициализации подключения
*	к базе данных с поледующим создание структуры.
*
*	@dbpath - Путь к базе данных.
*	@dbstruct - Главная управляющая структура.
*	@errf - Файловый стрим для записи ошибок.
*/
bool illdb_init(char *dbpath, illdb *dbstruct, FILE *errfile)
{
#ifdef ILLUMDEBUG
	printf("\nDatabase init...\n");
#endif
	int st_sqlite3 = -100;
	bool status = false;

	if (!dbpath || strlen(dbpath) < 8 || !dbstruct || !(errf = errfile)) {
		printf("Error: Input params in illdb_init are incorrect.\n");
		return status;
	}
	if ((st_sqlite3 = sqlite3_open(dbpath, &db)) != SQLITE_OK
		|| !illdb_tables()) {
		if (st_sqlite3 != SQLITE_OK)
			fprintf(errf, "Can't open database: %s\n", sqlite3_errmsg(db));
		return status;
	}
	illdb_removecache();

	dbstruct->currenttask = illdb_currenttask;
	dbstruct->staticnode = illdb_staticnode;
	dbstruct->removetask = illdb_removetask;
	dbstruct->isset_node = illdb_issetnode;
	dbstruct->nodelist = illdb_nodelist;
	dbstruct->nodeinfo = illdb_nodeinfo;
	dbstruct->newtask = illdb_settask;
	dbstruct->newnode = illdb_newnode;
	dbstruct->nodenum = illdb_nodenum;
	dbstruct->getvar = illdb_getvar;
	dbstruct->setvar = illdb_setvar;
	dbstruct->ping = illdb_ping;

	if (dbstruct->removetask && dbstruct->nodelist
		&& dbstruct->newtask && dbstruct->newnode
		&& dbstruct->currenttask && dbstruct->nodenum
		&& dbstruct->staticnode && dbstruct->isset_node
		&& dbstruct->nodeinfo && dbstruct->ping)
		status = true;

#ifdef ILLUMDEBUG
	printf("%s.\n", status ? "Ok" : "Fail");
#endif
	return status;
}
/**
*	illdb_removetask - Функция удаления задания из базы данных.
*
*	@id - Id задания в базе данных.
*/
static bool illdb_removetask(unsigned int id)
{
	sqlite3_stmt *rs = NULL;
	char *sql = (char *)malloc(200);
	bool status = false;
	
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
*	illdb_ping - Функция обновления времени последнего запроса.
*
*	@ipaddr - Ip ноды.
*/
static void illdb_ping(char *ipaddr)
{
	sqlite3_stmt *rs = NULL;
	unsigned int len = 0;
	time_t ping;
	char *sql;

	if (!ipaddr || ipaddr == NULL || (len = strlen(ipaddr)) < 7
		|| len > 100) {
		fprintf(errf, "Error: Incorrect ip in illdb_ping.\n");
		return;
	}

	ping = time(NULL);
	sql = (char *)malloc(len + 100);
	sprintf(sql, "UPDATE `nodes` SET `ping_t`='%ld' WHERE "
		"`ip`='%s' AND `status`='1';", ping, ipaddr);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE)
		fprintf(errf, "Error: Can't update ping time.\n");

	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);
}
/**
*	illdb_staticnode - Функция подтверждения статического подключения
*	между нодами.
*
*	@hash - Хэш ноды.
*/
static void illdb_staticnode(char *hash)
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
*/
static bool illdb_issetnode(char *ipaddr, char *hash, unsigned int id)
{
	char *sql, *stat = "\0", *value;
	bool status = true, type;
	sqlite3_stmt *rs = NULL;
	unsigned int len = 0;

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
		fprintf(errf, "Error: Incorrect params in illdb_issetnode.\n");
		return status;
	}
	if ((len = strlen(value)) > 200) 
		return status;

	if (id != 2)
		stat = (id == 0) ? "AND `status`='0'" : "AND `status`='1'";

	sql = (char *)malloc(len + 100);
	sprintf(sql, "SELECT COUNT(*) FROM `nodes` WHERE `%s`='%s' %s",
		((type) ? "ip" : "hash"), value, stat);

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
*	illdb_nodeinfo - Функция получения информации о конкретной
*	ноде в бд.
*
*	@ipaddr - Ip адрес ноды.
*/
static struct node_list illdb_nodeinfo(char *ipaddr)
{
	struct node_list node;
	sqlite3_stmt *rs = NULL;
	unsigned int len = 0;
	char *sql;

	if (!ipaddr || ipaddr == NULL || (len = strlen(ipaddr)) < 7
		|| len > 100) {
		fprintf(errf, "Error: Incorrect ip in nodeinfo\n");
		node.hash = NULL;
		return node;
	}

	sql = (char *)malloc(len + 100);
	sprintf(sql, "SELECT * FROM `nodes` WHERE `ip`='%s' "
		"AND `status`='1';", ipaddr);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_ROW) {
		fprintf(errf, "Waring: Node doesn't exist (nodeinfo)\n");
		node.hash = NULL;
		goto exit_nodeinfo;
	}

	node.ping = atol((const char *)sqlite3_column_text(rs, 3));
	node.id = atoi((const char *)sqlite3_column_text(rs, 0));
	node.ipaddr = (char *)sqlite3_column_text(rs, 1);
	node.hash = (char *)sqlite3_column_text(rs, 2);

exit_nodeinfo:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);
	return node;
}
/**
*	illdb_newnode - Функция занесения новой ноды в базу данных.
*
*	@ipaddr - Ip адрес получателя сообщения.
*	@hash - Ключ ноды для распознования в сети.
*	@text - Текст сообщения.
*	@mseconds - Время отклика узла.
*/
static bool illdb_newnode(char *ipaddr, char *hash)
{
	sqlite3_stmt *rs = NULL;
	unsigned int length = 0;
	bool status = false;
	time_t time_p;
	char *sql;

	if (!ipaddr || strlen(ipaddr) < 7 || !hash || strlen(hash) < 10) {
		fprintf(errf, "Error: Incorrect input data in illdb_newnode.\n");
		return status;
	}
	if (illdb_issetnode(ipaddr, NULL, 2))
		return status;

	if ((length = strlen(ipaddr) + strlen(hash)) > MAXTEXTSIZE) {
		fprintf(errf, "Error: Text size for task more than you can write.\n");
		goto exit_newnode;
	}

	time_p = time(NULL);
	sql = (char *)malloc(length + 100);
	sprintf(sql, "INSERT INTO `nodes` VALUES (NULL, '%s', '%s', '%ld', 0);",
			ipaddr, hash, time_p);

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
*	@data - Указатель для записи заданий.
*/
static void illdb_currenttask(struct stask *data)
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
*/
static struct node_list *illdb_nodelist(unsigned int *num)
{
	struct node_list *data = NULL;
	sqlite3_stmt *rs = NULL;
	int i = -1, tmp = 0;

	if ((*num = illdb_nodenum()) < 1) {
		*num = 0;
		goto exit_nodelist;
	}

	data = (struct node_list *)malloc(sizeof(struct node_list) * (*num));
	sqlite3_prepare_v2(db, "SELECT * FROM `nodes` WHERE `status`='1'",
		-1, &rs, NULL);

	while (i++, sqlite3_step(rs) == SQLITE_ROW) {
		if (i > (*num) - 1)
			break;

		data[i].ping = atol((const char *)sqlite3_column_text(rs, 3));
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
*/
static bool illdb_issettask(char *ipaddr, char *headers)
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
*/
static int illdb_settask(char *ipaddr, char *text, char *headers)
{
	sqlite3_stmt *rs = NULL;
	unsigned int length = 0;
	int id = -1;
	char *sql;

	if (!ipaddr || strlen(ipaddr) < 7 || !headers || strlen(headers) < 8) {
		fprintf(errf, "Error: Incorrect input data in illdb_settask.\n");
		return id;
	}

	if (illdb_issettask(ipaddr, headers)) 
		return id;

	if (text && text != NULL)
		length += strlen(text);
	if ((length += strlen(ipaddr) + strlen(headers)) > MAXTEXTSIZE) {
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
*/
static int illdb_nodenum()
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
*	illdb_getvar - Функция получения переменной из таблицы
*	настроек.
*
*	@name - Ip адрес получателя сообщения.
*/
static void illdb_getvar(char *name, char buffer[])
{
	sqlite3_stmt *rs = NULL;
	char *sql, *var = NULL;
	unsigned int len = 0;

	if (!name || name == NULL || (len = strlen(name)) < 2
		|| len > 20) {
		fprintf(errf, "Error: Incorrect option in getvar.\n");
		goto exit_getvar;
	}

	sql = (char *)malloc(len + 100);
	sprintf(sql, "SELECT `value` FROM `settings` WHERE `name`='%s'",
		name);

	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_ROW) {
		fprintf(errf, "Warring: Can't get %s from db.\n", name);
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
*	illdb_setvar - Функция записи значения в таблицу
*	настроек.
*
*	@name - Ip адрес получателя сообщения.
*	@value - Значение для записи.
*/
static bool illdb_setvar(char *name, char *value)
{
	unsigned int len1 = 0, len2 = 0;
	sqlite3_stmt *rs = NULL;
	bool status = false;
	char *sql;

	if (!name || (len1 = strlen(name)) < 2 || len1 > 20
		|| !value || (len2 = strlen(value)) < 2
		|| len2 > 101) {
		fprintf(errf, "Error: Incorrect option in getvar.\n");
		return status;
	}

	sql = (char *)malloc(len1 + len2 + 100);
	sprintf(sql, "DELETE FROM `settings` WHERE `name`='%s'",
		name);
	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Can't remove data from settings.\n");
		goto exit_setvar;
	}

	memset(sql, '\0', strlen(sql));
	sprintf(sql, "INSERT INTO `settings` VALUES ('%s', '%s');",
		name, value);
	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Can't set data in settings.\n");
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
*	illdb_tables - Функция создания базы данных проекта.
*/
static bool illdb_tables()
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
	"`hash` text NOT NULL, `ping_t` int(15) NOT NULL, `status`"
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