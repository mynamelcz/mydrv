
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
/* seconddrvtest 
  */
int main(int argc, char **argv)
{
	int fd;
	unsigned char key_vals;
	int ret = 0;
    struct pollfd pfd[1];

	fd = open("/dev/xyz", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}
    pfd[0].fd = fd;
    pfd[0].events = POLLIN;   
	while (1)
	{
        ret =  poll(pfd, 1, 1000);
        if(ret == 0)
        {
            printf("time out\n");

        }
        else
        {
        	read(fd, &key_vals, 1);
            printf("key_vals = %d\n",key_vals);
        }
    }	
	return 0;
}

