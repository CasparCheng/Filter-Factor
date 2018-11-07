/* file: filter.c */
#include <stdio.h>
#include <unistd.h>

int filter(int m, int read_fd, int write_fd) {
    int num = 0; /* a single number */
    int rc;      /* return code */

    /* loop until -1 (end of list) */
    while (num != -1) {
        if ((rc = read(read_fd, &num, sizeof(int))) == -1) {
            perror("read()");
            return 1;
        } else if (rc == 0) {
            break;
        }
        if (num % m != 0) {
            /* keep this number */
            if ((rc = write(write_fd, &num, sizeof(int))) == -1) {
                perror("write()");
                return 1;
            } else if (rc == 0) {
                break;
            }
        }
    }

    return 0;
}
