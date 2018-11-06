#include "../sources/functions.h"

int main()
{
	illum illum;
	int type;

	illum_init(&illum, "./");

	if (illum.firstnode)
		illum.firstnode("192.168.1.42");
	else
		illum.connect();

	illum.module.server.start();

	/*
		Next: You can use sendstraight() function
			or use sendonion() function.
	*/
	while (true) {
		printf("Type: ");
		scanf("%d", &type);
		//illum.testrequest((enum illheader)type, "192.168.1.46");
	}
	return 0;
}