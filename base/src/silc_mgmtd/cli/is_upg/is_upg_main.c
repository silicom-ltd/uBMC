#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv [])
{
	char cmd[256];
	int i, ret;

	setuid(0);
	clearenv();
	strcpy(cmd, "/usr/sbin/is_upgrade.sh ");
	for(i=1; i<argc; i++)
	{
		strcat(cmd, argv[i]);
		strcat(cmd, " ");
	}
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	return ret;
}
