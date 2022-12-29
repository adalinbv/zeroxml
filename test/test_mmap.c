
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "xml.h"

int test(xmlId *root_id);

int main(int argc, char **argv)
{
    const char *fname = SOURCE_DIR"/test/sample.xml";
    xmlId *root_id;
    int rv = 0;

    root_id = xmlOpen(fname);
    if (root_id)
    {
        rv = test(root_id);
        xmlClose(root_id);
    }
    else {
        printf("File not found: %s\n", fname);
    }
    printf("\n");

    return rv;
}
