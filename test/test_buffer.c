
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "xml.h"

int test(xmlId *root_id);

int main(int argc, char **argv)
{
    const char *fname = SOURCE_DIR"/test/sample.xml";
    int fd, rv = 0;

    fd = open(fname, O_RDONLY);
    if (fd)
    {
        struct stat sbuf;
        if (fstat(fd, &sbuf) == 0)
        {
            char *buf = malloc(sbuf.st_size);
            if (buf && (read(fd, buf, sbuf.st_size) == sbuf.st_size))
            {
                xmlId *root_id = xmlInitBuffer(buf,sbuf.st_size);
                rv = test(root_id);
                xmlClose(root_id);
                free(buf);
            }
            else if (!buf) {
                printf("Not enough memory.\n");
            } else {
                printf("Read error: %s\n", strerror(errno));
            }
        }
        close(fd);
    }
    else {
        printf("File not found: %s\n", fname);
    }
    printf("\n");

    return rv;
}
