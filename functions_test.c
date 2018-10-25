#include "../sources/functions.h"

int main()
{
	illum illum;
	int type = 0;

	illum_init(&illum, "./");
	illum.firstnode("192.168.1.46");

	while (true) {
		printf("Type: ");
		scanf("%d", &type);

		illum.testrequest((enum illheader)type, "192.168.1.46");
	}

	return 0;
}