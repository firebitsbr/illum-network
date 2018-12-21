/**
*	storage.c - Функции модуля storage 
*	децентрализованной сети illum.
*
*	@mrrva - 2018
*/
#include "./illum.h"
/**
*	Приватные переменные и указатели.
*/
static bool storageinit = false;
static sqlite3 *db;
static FILE *error;
/**
*	Прототипы приватных функций.
*/
static void illum_setvar(char *, char *);
static char *illum_getvar(char *);
static bool illum_tables();
/**
*	illum_database - Функция инициализации подключения
*	к базе данных с поледующим создание структуры.
*
*	@database - Главная управляющая структура.
*	@filepath - Путь к базе данных.
*	@fp - Файловый стрим для записи ошибок.
*/
bool illum_database(struct illumdb *database, char *filepath,
	FILE *fp)
{
	if (!filepath || strlen(filepath) < 8 || !database
		|| !(error = fp) || storageinit) {
		printf("Error: Incorrect args in dbinit.\n");
		return false;
	}
	if (sqlite3_open(filepath, &db) != SQLITE_OK
		|| !illum_tables()) {
		fprintf(error, "Error: Can't create db.\n");
		return false;
	}

	database->get = illum_getvar;
	database->set = illum_setvar;

	return (storageinit = true);
}
/**
*	illum_setvar - Функция установки данных в
*	таблицу settings значения.
*
*	@key - Имя параметра.
*	@value - Значение параметра.
*/
static void illum_setvar(char *key, char *value)
{
	sqlite3_stmt *rs = NULL;
	char *query;

	if (!key || !value || strlen(key) > 100
		|| strlen(value) > 100) {
		fprintf(error, "Error: Can't set value to db.\n");
		return;
	}

	asprintf(&query, "DELETE FROM `settings` WHERE "
		"`name`='%s'", key);
	sqlite3_prepare_v2(db, query, -1, &rs, NULL);
	sqlite3_step(rs);
	free(query);

	asprintf(&query, "INSERT INTO `settings` VALUES "
		"('%s', '%s')", key, value);
	sqlite3_prepare_v2(db, query, -1, &rs, NULL);
	sqlite3_step(rs);
	free(query);

	if (rs && rs != NULL)
		sqlite3_finalize(rs);
}
/**
*	illum_getvar - Функция извлечение данных из таблицы
*	конфигурации.
*
*	@key - Имя параметра.
*/
static char *illum_getvar(char *key)
{
	char *query, *data = NULL;
	sqlite3_stmt *rs = NULL;

	if (!key || strlen(key) > 100) {
		fprintf(error, "Error: Invalid arg in getvar.\n");
		return NULL;
	}

	asprintf(&query, "SELECT * FROM `settings` WHERE "
		"`name`='%s'", key);
	sqlite3_prepare_v2(db, query, -1, &rs, NULL);
	if (sqlite3_step(rs) != SQLITE_ROW)
		goto exit_getvar;
	asprintf(&data, "%s", sqlite3_column_text(rs, 1));

exit_getvar:
	if (query && query != NULL)
		free(query);
	if (rs && rs != NULL)
		sqlite3_finalize(rs);

	return data;
}
/**
*	illum_tables - Функция создания базы данных проекта.
*
*/
static bool illum_tables()
{
	const char *query[] = {
		"CREATE TABLE IF NOT EXISTS " \
		"`nodes` (`id` INTEGER PRIMARY " \
		"KEY AUTOINCREMENT, `ip` text " \
		"NOT NULL,`hash` text NOT NULL," \
		" `use_t` int(15) NOT NULL);",

		"CREATE TABLE IF NOT EXISTS " \
		"`settings` (`name` text NOT" \
		" NULL, `value` text NOT NULL);"
	};
	sqlite3_stmt *rs = NULL;
	bool status = false;

	for (int i = 0; i < 2; i++) {
		sqlite3_prepare_v2(db, query[i], -1, &rs, NULL);
		if (sqlite3_step(rs) != SQLITE_DONE) {
			fprintf(error, "Error: Create nodes table.\n");
			goto exit_tables;
		}
	}
	status = true;

exit_tables:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	return status;
}