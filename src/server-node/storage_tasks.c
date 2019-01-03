/**
*	storage_tasks.c - Функции модуля storage 
*	децентрализованной сети illum отвечающие
*	за работу приложения со списками задач.
*
*	@mrrva - 2018
*/
#include "include/storage.h"
/**
*	illum_newtask - Функция добавления нового задания
*	в список заданий.
*
*	@new - Структура нового задания.
*/
bool illum_newtask(struct illumtasks *new)
{
	struct illumtasks *temp = tasks;

	if (!new || new == NULL)
		return false;

	new->next = NULL;
	if (tasks == NULL) {
		tasks = new;
		return true;
	}

	while (temp->next != NULL)
		temp = temp->next;
	temp = new;
	return true;
}
/**
*	illum_removetask - Функция удаления первого
*	задания из списка.
*
*/
void illum_removetask(void)
{
	struct illumtasks *temp = tasks;

	if (temp == NULL)
		return;

	tasks = tasks->next;
	free(temp);
}
/**
*	illum_printtasks - Функция вывода невыполненных
*	заданий.
*
*/
void illum_printtasks(void)
{
	struct illumtasks *tmp = tasks;

	printf("List of tasks.\n");
	while (tmp != NULL) {
		printf("%s - %s\n", tmp->ip, tmp->text);
		tmp = tmp->next;
	}
}
/**
*	illum_gettasks - Функция возврата указателя
*	на структуру заданий.
*
*/
struct illumtasks *illum_gettasks()
{
	return tasks;
}