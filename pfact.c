/* file: pfact.c */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "filter.h"

/* start filtering */
int start(int n, int sqrt_n, int* factors, int* n_factors, int* n_filters);

/* main entry */
int main(int argc, char* argv[]) {
    int n;          /* the number to be checked */
    char* end_ptr;  /* pointer pointing the the end of string */
    int n_factors;  /* number of factors */
    int factors[2]; /* factors */
    int n_filters;  /* number of filters */
    int rc;         /* return code */
    int sqrt_n;     /* square root of n */

    /* check the number of arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage:\n\tpfact n\n");
        return 1;
    }

    /* check if n is a positive integer */
    n = strtol(argv[1], &end_ptr, 10);
    if (n < 1 || *end_ptr != '\0') {
        fprintf(stderr, "Usage:\n\tpfact n\n");
        return 1;
    }

    /* compute the square root of n */
    sqrt_n = ceil(sqrt(n));

    /* start filtering */
    if ((rc = start(n, sqrt_n, factors, &n_factors, &n_filters)) != 0) {
        return rc;
    }

    /* check results */
    if (n_factors == 0) {
        /* find no factors, then n must be prime */
        printf("%d is prime\n", n);
    } else if (n_factors == 1) {
        /* find exactly one factor, n may be the product of two primes */
        if (factors[0] == sqrt_n) {
            /* n is a perfect square */
            printf("%d %d %d\n", n, factors[0], factors[0]);
        } else {
            /* n is not a perfect square */
            printf("%d is not the product of two primes\n", n);
        }
    } else {
        /* find two factors, n may be the product of two primes */
        if (factors[1] >= sqrt_n) {
            /* the second factor is greater than or
               equal to the square root of n */
            printf("%d %d %d\n", n, factors[0], factors[1]);
        } else {
            /* both factors are less than the square root of n */
            printf("%d is not the product of two primes\n", n);
        }
    }

    /* print out the number of filters */
    printf("Number of filters = %d\n", n_filters);

    return 0;
}

int start(int n, int sqrt_n, int* factors, int* n_factors, int* n_filters) {
    int m;      /* filter value */
    pid_t pid;  /* process id */
    int status; /* return status */
    int i;      /* loop variable */
    int fd[2];  /* pipe file descriptors */

    m = 2;          /* m starts from 2 */
    *n_filters = 0; /* no filters created yet */
    *n_factors = 0; /* no factors found yet */

    /* make a pipe */
    if (pipe(fd) < 0) {
        perror("pipe");
        return 2;
    }

    /* write the inital list into pipe */
    for (i = m + 1; i < n + 1; ++i) {
        if (write(fd[1], &i, sizeof(int)) == -1) {
            return 2;
        }
    }

    /* put -1 at the end of the list */
    i = -1;
    if (write(fd[1], &i, sizeof(int)) == -1) {
        return 2;
    }

    while (1) {
        /* check if m is a factor of n */
        if (n % m == 0) {
            factors[*n_factors] = m;
            *n_factors += 1;
        }

        /* check if m is greater than or equal to the square root of n */
        if (m >= sqrt_n) {
            break;
        }

        /* create child process */
        pid = fork();
        if (pid < 0) {
            perror("fork");
            return 2;
        }

        if (pid == 0) {
            /* --- child process --- */
            filter(m, fd[0], fd[1]);
            /* return the number of filters so far */
            exit(1 + *n_filters);
        }

        /* --- parent process --- */
        /* waiting for child process terminated */
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return 2;
        }
        /* check if child process exits normally */
        if (WIFEXITED(status)) {
            *n_filters = WEXITSTATUS(status);
        } else {
            fprintf(stderr, "'filter' process didn't exit properly\n");
        }
        /* read the first number of list into m */
        if (read(fd[0], &m, sizeof(int)) == -1) {
            perror("read()");
            return 2;
        }
        /* check if already get two factors */
        if (*n_factors == 2) {
            break;
        }
    }

    /* close pipe file descriptors */
    close(fd[0]);
    close(fd[1]);

    return 0;
}
