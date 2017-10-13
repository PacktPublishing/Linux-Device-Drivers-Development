#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/select.h>

#define TEST_SYSFS_TRIGGER  "/sys/hello/trigger"
#define TEST_SYSFS_NOTIFY   "/sys/hello/notify"

int main (void)
{

    fd_set read_fds;
    int i, max_fd, ret, cnt;
    int notify_fd, trigger_fd;
    char attrData[100];
    FD_ZERO (&read_fds);

    if ((notify_fd = open(TEST_SYSFS_NOTIFY, O_RDWR)) < 0){
        perror("Unable to open notify");
        exit(1);
    }

    if ((trigger_fd = open(TEST_SYSFS_TRIGGER, O_RDWR)) < 0) {
        perror("Unable to open trigger");
        exit(1);
    }

    FD_SET (notify_fd, &read_fds);
    FD_SET (trigger_fd, &read_fds);
    max_fd = notify_fd > trigger_fd ? notify_fd : trigger_fd;

    /* We first need to read data until the end of the file */
    cnt = read( notify_fd, attrData, 100 );
    cnt = read( trigger_fd, attrData, 100 );

    ret = select(max_fd + 1, NULL, NULL, &read_fds, NULL);
    if (ret < 0)
        perror("pselect events");
    if (FD_ISSET(notify_fd, &read_fds))
        printf("Change detected in /sys/hello/notify\n");
    if (FD_ISSET(trigger_fd, &read_fds))
        printf("Change detected in /sys/hello/trigger\n");

    close( trigger_fd );
    close( notify_fd );
    return 0;
}

