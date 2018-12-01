
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* seconddrvtest 
  */
int main(int argc, char **argv)
{
	int fd;
	unsigned char key_vals;
	int cnt = 0;
	
	fd = open("/dev/xyz", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	while (1)
	{
		read(fd, &key_vals, 1);
        printf("key_vals = %d\n",key_vals);
	}
	
	return 0;
}

