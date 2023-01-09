
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "xml.h"

int test(xmlId *root_id);

int main(int argc, char **argv)
{
    const char *fname = SOURCE_DIR"/test/errors.xml";
    xmlId *root_id;
    int rv = 0;

    root_id = xmlOpen(fname);
    if (root_id)
    {
        rv = test(root_id);
        xmlClose(root_id);
    }
    else {
        printf("Error: %s\n", xmlErrorGetString(root_id, 0));
    }
    printf("\n");

    return rv;
}
