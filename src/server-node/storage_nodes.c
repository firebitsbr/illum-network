/**
*	storage_lists.c - Функции модуля storage 
*	децентрализованной сети illum отвечающие
*	за работу приложения со списками нод.
*
*	@mrrva - 2018
*/
#include "include/storage.h"
/**
*	illum_nodeinsert_list - Функция добавления новой ноды
*	в одщий список.
*
*	@new - Указатель на структуру новой ноды.
*/
bool illum_nodeinsert_list(struct illumnodes *new)
{
	if (!new || new == NULL) {
		fprintf(error, "Error: Invalid pointer in "
			"nodeinsert.\n");
		return false;
	}

	new->next = nodes;
	nodes = new;
	return true;
}
/**
*	illum_nodeselect - Функция заполнения списка
*	известных нод из бд.
*
*/
void illum_nodeselect(void)
{
	size_t st_size = sizeof(struct illumnodes);
	const unsigned char *(*sql_data)();
	unsigned char *b_hash = NULL;
	struct illumnodes *temp;
	sqlite3_stmt *rs;
	char *tempdata;

	sqlite3_prepare_v2(db, "SELECT * FROM `nodes`", -1,
		&rs, NULL);
	sql_data = sqlite3_column_text;

	while (sqlite3_step(rs) == SQLITE_ROW) {
		temp = (struct illumnodes *)malloc(st_size);

		tempdata = (char *)sql_data(rs, 1);
		memcpy(temp->ip, tempdata, strlen(tempdata) + 1);
		temp->use_t = atol((char *)sql_data(rs, 3));
		tempdata = (char *)sql_data(rs, 2);
		b_hash = p_enc->hex2byte(tempdata);

		if (!b_hash || b_hash == NULL) {
			free(temp);
			continue;
		}

		memcpy(temp->hash, b_hash, HASHSIZE);
		if (!illum_nodeinsert_list(temp))
			free(temp);
		free(b_hash);
	}

	if (rs && rs != NULL)
		sqlite3_finalize(rs);
}
/**
*	illum_nodeexists - Функция проверки существования
*	ноды в списке.
*
*	@ipaddr - Ip адрес ноды сети.
*/
bool illum_nodeexists(char *ipaddr)
{
	struct illumnodes *temp = nodes;

	if (!ipaddr || strlen(ipaddr) > 20)
		return true;

	while (temp != NULL) {
		if (strcmp(temp->ip, ipaddr) == 0)
			return true;
		temp = temp->next;
	}
	return false;
}
/**
*	illum_newnode - Функция добавления новой ноды в 
*	общий список.
*
*	@node - Структура новой ноды сети.
*/
bool illum_newnode(struct illumnodes *node)
{
	time_t utime = time(NULL);
	char *query, *hash = NULL;
	sqlite3_stmt *rs = NULL;
	bool status = false;

	if (!node || node == NULL
		|| illum_nodeexists(node->ip))
		return false;

	hash = p_enc->byte2hex(node->hash, 32);
	if (hash == NULL)
		goto exit_newnode;

	asprintf(&query, "INSERT INTO `nodes` VALUES"
		" (NULL, '%s', '%s', '%ld');", node->ip,
		hash, utime);
	sqlite3_prepare_v2(db, query, -1, &rs, NULL);

	if (sqlite3_step(rs) != SQLITE_DONE)
		goto exit_newnode;

	illum_freenode();
	illum_nodeselect();
	status = true;

exit_newnode:
	if (rs && rs != NULL)
		sqlite3_finalize(rs);
	if (query && query != NULL)
		free(query);
	if (hash != NULL)
		free(hash);

	return status;
}
/**
*	illum_freenode - Функция удаления указателя
*	на список нод.
*
*/
void illum_freenode(void)
{
	struct illumnodes *temp = nodes;

	if (!nodes || nodes == NULL)
		return;

	do {
		temp = nodes;
		nodes = nodes->next;

		free(temp);
	}
	while(nodes != NULL);
}
/**
*	illum_printnodes - Функция отображение всех
*	известных нод сети.
*
*/
void illum_printnodes(void)
{
	struct illumnodes *temp = nodes;
	char *hash;

	if (!temp || temp == NULL)
		return;

	while (temp != NULL) {
		hash = p_enc->byte2hex(temp->hash, 32);
		printf("-> %s / %s\n", temp->ip, hash);

		temp = temp->next;
		free(hash);
	}
}
/**
*	illum_getnodes - Функция извлечения указателя
*	на список нод.
*
*/
struct illumnodes *illum_getnodes()
{
	return nodes;
}