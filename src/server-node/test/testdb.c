/**
*	Тестовая часть проверки модуля storage
*
*	@mrrva - 2018
*/
#include "../illum.h"

int main(int argc, char *args[])
{
	/**
	*	Управляющие структуры модулей.
	*/
	char *path = "./illum.db", *data;
	struct illumdb db;
	/**
	*	Инициализируем управляющие структуры.
	*/
	if (illum_database(&db, path, stderr) == false) {
		printf("Error: Can't init db struct.\n");
		return 1;
	}
	/**
	*	Получение данных из базы данных.
	*/
	data = db.get("EXAMPLE");
	printf("%s\n", data);

	if (!data || data == NULL)
		db.set("EXAMPLE", "test_data");

	return 0;
}