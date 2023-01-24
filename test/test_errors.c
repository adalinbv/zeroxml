
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "xml.h"

#define MAX_PATH_LEN	1024

int main(int argc, char **argv)
{
    const char *fname = SOURCE_DIR"/test/errors.xml";
    char path[MAX_PATH_LEN];
    xmlId *root_id;
    int rv = 0;

    if (argc == 2)
    {
        snprintf(path, MAX_PATH_LEN-1, SOURCE_DIR"/test/%s.xml", argv[1]);
        fname = path;
    }

    root_id = xmlOpen(fname);
    if (root_id)
    {
        xmlClose(root_id);
        rv = 1;
    }
    else {
        printf("Error: %s\n", xmlErrorGetString(root_id, 0));
    }
    printf("\n");

    return rv;
}
