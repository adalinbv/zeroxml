
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
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#include "xml.h"

#ifdef WIN32
# include <io.h>
#define open	_open
#define close	_close
#define read	_read
#endif

int fuzz(const char*, size_t);

#define MAX_SMALL_BUF	 256
#define MAX_LARGE_BUF	4096

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
            char *buf = (char*)malloc(sbuf.st_size);
            if (buf && (read(fd, buf, sbuf.st_size) == sbuf.st_size))
            {
                rv = fuzz(buf, sbuf.st_size);
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

size_t len = 1;
char name[MAX_LARGE_BUF+1] = "/";
void walk_xml(xmlId *id, char *name, unsigned int len)
{
    xmlId *xid = xmlMarkId(id);
    int num, i;

    num = xmlNodeGetNum(xid, "*");
    for (i=0; i<xmlAttributeGetNum(xid); ++i)
    {
        char value[MAX_SMALL_BUF+1];
        char attr[MAX_SMALL_BUF+1];

        xmlAttributeCopyName(xid, attr, MAX_SMALL_BUF, i);
        if (xmlErrorGetNo(xid, 0) != XML_NO_ERROR) {
            printf("Error for xmlAttributeCopyName: %s\n",
                    xmlErrorGetString(xid, XML_TRUE));
        }

        xmlAttributeCopyString(xid, attr, value, MAX_SMALL_BUF);
//      printf("%s[@%s] = \"%s\"\n", name, attr, value);
        if (xmlErrorGetNo(xid, 0) != XML_NO_ERROR) {
            printf("Error for xmlAttributeCopyString: %s\n",
                    xmlErrorGetString(xid, XML_TRUE));
        }
    }

    if (num == 0)
    {
        char s[MAX_SMALL_BUF+1] = "";
        xmlCopyString(xid, s, MAX_SMALL_BUF);
//      printf("%s = \"%s\"\n", name, s);
        if (xmlErrorGetNo(xid, 0) != XML_NO_ERROR) {
            printf("Error for xmlCopyString: %s\n",
                    xmlErrorGetString(xid, XML_TRUE));
        }
    }
    else
    {
        int i;

        name[len++] = '/';
        for (i=0; i<num; i++)
        {
            if (xmlNodeGetPos(id, xid, "*", i) != 0)
            {
                if (xmlNodeTest(xid, XML_COMMENT)) continue;

                unsigned int res, i = MAX_LARGE_BUF - len;
                if ((res = xmlNodeCopyName(xid, (char *)&name[len], i)) != 0)
                {
                    unsigned int index = xmlAttributeGetInt(xid, "n");
                    if (index)
                    {
                        unsigned int pos = len+res;

                        name[pos++] = '[';
                        i = snprintf((char *)&name[pos], MAX_LARGE_BUF-pos,
                                     "%i", index);
                        name[pos+i] = ']';
                        name[pos+i+1] = 0;
                        res += i+2;
                    }
                }
                else {
                    printf("Error for xmlNodeCopyName: %s\n",
                            xmlErrorGetString(xid, XML_TRUE));
                }
                walk_xml(xid, name, len+res);
            }
            else {
                printf("Error for xmlNodeGetPos: %s\n",
                        xmlErrorGetString(xid, XML_TRUE));
            }
        }
    }
    xmlFree(xid);
}

int fuzz(const char *buf, size_t size)
{
    xmlId *root_id;
    char *fuzzbuf;
    int j, k, l;
    int rv = 0;

    if ((fuzzbuf = (char*)malloc(size)) == NULL)
    {
        printf("Not enough memory.\n");
        return -1;
    }

    srand(time(NULL));
    for (l=0; l<10; ++l)
    {
        for (j=0; j<100; ++j)
        {
            memcpy(fuzzbuf, buf, size);

            // Let's fuzz things up.
            for(k=0; k<l; ++k)
            {
                off_t offs = rand() % size;
                fuzzbuf[offs] = rand() % 256;
            }

            root_id = xmlInitBuffer(fuzzbuf, size);
            if (root_id)
            {
                int i, num, res;
                xmlId *xid;

                xid = xmlMarkId(root_id);
                num = xmlNodeGetNum(xid, "*");
                for (i=0; i<num; i++)
                {
                    char name[MAX_LARGE_BUF+1] = "/";
                    if (xmlNodeGetPos(root_id, xid, "*", i) != 0)
                    {
                        if (xmlNodeTest(xid, XML_COMMENT)) continue;

                        res = xmlNodeCopyName(xid, name+1, MAX_LARGE_BUF-1);
                        walk_xml(xid, name, res+1);
                    }
                }
                free(xid);
                
                xmlClose(root_id);
            }
        }
    }
    free(fuzzbuf);

    return rv;
}
