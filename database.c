/**
*	database.c - Функции для управления базой данных
*	проекта.	
*
*	@mrrva - 2018
*/
static bool illdb_newnode(sqlite3 *, char *, char *, int *, char *, char *, FILE *);
static int illdb_settask(sqlite3 *, char *, char *, char *, FILE *);
static bool illdb_tables(sqlite3 *, FILE *);
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
	bool status = false;

	if (!dbpath || strlen(dbpath) < 8 || !dbstruct || !errf)
		return status;
	if (!sqlite3_open(dbpath, &dbstruct->db) || !illdb_tables(dbstruct->db, errf))
		return status;

	dbstruct->removetask = illdb_removetask;
	dbstruct->nodelist = illdb_nodelist;
	dbstruct->settask = illdb_settask;
	dbstruct->newnode = illdb_newnode;

	if (dbstruct->removetask && dbstruct->nodelist && dbstruct->settask 
		&& dbstruct->newnode)
		status = true;

	return status;
}
/**
*	illdb_newnode - Функция занесения новой ноды в базу данных.
*
*	@db - Указатель на подключение к базе данных.
*	@ipaddr - Ip адрес получателя сообщения.
*	@hash - Ключ ноды для распознования в сети.
*	@cert - Публичный сертификат получателя сообщения.
*	@text - Текст сообщения.
*	@errf - Файловый стрим для записи ошибок.
*/
static bool illdb_newnode(sqlite3 *db, char *ipaddr, 
						char *hash, int *mseconds,
						char *about, char *cert)
{
	sqlite3_stmt *rs = NULL;
	char *sql, *mseconds_str;
	unsigned int length = 0;
	bool status = false;

	if (!ipaddr || strlen(ipaddr) < 7 || !hash || strlen(hash) < 10
		|| mseconds < 0 || !about || strlen(about) < 50 || !cert
		|| strlen(cert) < 100) {
		fprintf(errf, "Error: Incorrect input data in illdb_settask.\n");
		return status;
	}

	mseconds_str = (char *)malloc(10);
	itoa(mseconds, mseconds_str, 10);
	if ((length = strlen(ipaddr) + strlen(hash) + strlen(mseconds_str)
		+ strlen(about) + strlen(cert)) > MAX_TEXTSIZE) {
		fprintf(errf, "Error: Text size for task more then you can write.\n");
		goto exit_newnode;
	}
	sql = (char *)malloc(length + 100);
	sprintf(sql, "INSERT INTO `nodes` VALUES (NULL, '%s', '%s', '%s', '%s', '%s');",
			ipaddr, hash, mseconds_str, about, cert);
	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Can't set new node to database.\n");
		goto exit_newnode;
	}
	status = true;

exit_newnode:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(mseconds_str);
	if (sql)
		free(sql);

	return status;
}
/**
*	illdb_settask - Функция создания новой задачи.
*
*	@db - Указатель на подключение к базе данных.
*	@ipaddr - Ip адрес получателя сообщения.
*	@cert - Публичный сертификат получателя сообщения.
*	@text - Текст сообщения.
*	@errf - Файловый стрим для записи ошибок.
*/
static int illdb_settask(sqlite3 *db, char *ipaddr, 
						char *cert, char *text, FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	unsigned int length = 0;
	int id = -1;
	char *sql;

	if (!ipaddr || strlen(ipaddr) < 7 || !text || strlen(text) < 10) {
		fprintf(errf, "Error: Incorrect input data in illdb_settask.\n");
		return id;
	}

	if (cert && cert != NULL)
		length += strlen(cert);
	if ((length += strlen(ipaddr) + strlen(text)) > MAX_TEXTSIZE) {
		fprintf(errf, "Error: Text size for task more then you can write.\n");
		return id;
	}
	
	sql = (char *)malloc(length + 100);
	sprintf(sql, "INSERT INTO `tasks` VALUES (NULL, '%s', '%s', '%s', 0);", ipaddr,
			((!cert || cert == NULL) ? "" : cert), text);
	sqlite3_prepare_v2(db, sql, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Can't create new task.\n");
		goto exit_settask;
	}
	sqlite3_prepare_v2(db, "SELECT last_insert_rowid();", -1, &rs, NULL);
	id = atoi(sqlite3_column_text(rs, 0));

exit_settask:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	free(sql);

	return id; /* -1 is error */
}
/**
*	illdb_tables - Функция создания базы данных проекта.
*
*	@db - Указатель на подключение к базе данных.
*	@errf - Файловый стрим для записи ошибок.
*/
static bool illdb_tables(sqlite3 *db, FILE *errf)
{
	sqlite3_stmt *rs = NULL;
	bool status = false;

	sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS `tasks` ("
	"`id` INTEGER PRIMARY KEY AUTOINCREMENT, `ip` text NOT NULL,"
	"`cert` text, `text` text NOT NULL, `status` int(11) NOT NULL);", -1, &rs, NULL);
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
	"`hash` text NOT NULL, `mseconds` int(11) NOT NULL,"
	"`about` text NOT NULL, `cert` text NOT NULL);", -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_DONE) {
		fprintf(errf, "Error: Sqlite can't create nodes table.\n");
		goto exit_tables;
	}
	status = true;

exit_tables:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	return status;
}