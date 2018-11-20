/**
*	constants.h - Заголовочный файл хранения констант
*	проекта, в будующем будет перенесен в базу данных.
*
*	ВНИМАНИЕ: Не изменять значения, в ином случае
*	проект может неработать или работать со сбоями!
*
*	ПРИМЕЧАНИЕ: Значение констант может изменяться
*	в каждом обновлении.
*
*	@mrrva - 2018
*/
#ifndef ILL_CONSTANTS
#define ILL_CONSTANTS
/**
*	Описание констант:
*
*	MAXTEXTSIZE - Максимальное число символов, которое
*	может принять или отправить сервер.
*
*	MAXNODES - Максимальное количество статических
*	связей между клиентами сети.
*
*	UPDTIME - Время обновления статуса клиентов сети.
*
*	SERVER_TIMEOUT - Время ожидания ответа от клиента
*	сети.
*
*	ILLUM_PORT - Порт, который слушает сервер для приема
*	и отправки данных.
*
*	THREAD_LIMIT - Количество потоков (не задействовано).
*
*	ILLUMDEBUG - Режим отладки, с выводом описания каждого
*	шага проекта.
*/
#define MAXNODES		180
#define UPDTIME			10000
#define MAXTEXTSIZE		9000
#define SERVER_TIMEOUT	10
#define ILLUM_PORT		110
#define THREAD_LIMIT	5
#define ILLUMDEBUG		1

#endif