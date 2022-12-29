
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml.h"

#define ROOTNODE	"/Configuration"
#define OUTPUTNODE	ROOTNODE"/output"
#define MENUNODE	OUTPUTNODE"/menu"
#define	LEAFNODE	"name"
#define PATH		MENUNODE"/"LEAFNODE
#define BUFLEN		4096

#define PRINT_ERROR_AND_EXIT(id) \
  if (xmlErrorGetNo(id, 0) != XML_NO_ERROR) { \
      const char *errstr = xmlErrorGetString(id, 0); \
      int column = xmlErrorGetColumnNo(id, 0); \
      int lineno = xmlErrorGetLineNo(id, 1); \
      printf("\n\tError at line %i, column %i: %s\n", lineno, column, errstr);\
      exit(-1); \
  }

int test(xmlId *root_id)
{
    if (root_id)
    {
        xmlId *path_id, *node_id;
        char *s;
        int i;

        printf("\n");

        node_id = xmlNodeGet(root_id, OUTPUTNODE);
        printf("Testing xmlNodeGetNum for "OUTPUTNODE"/boolean:");
        if ((i = xmlNodeGetNum(node_id, "boolean")) != 5) {
            printf("\t\tfailed.\n\t'%i' should be 5.\n", i);
        } else {
            printf("\t\tsucces.\n");
        }

        printf("Testing xmlNodeGetBool for "OUTPUTNODE"/boolean: (0)");
        if ((i = xmlNodeGetBool(root_id, OUTPUTNODE"/boolean")) != 0) {
            printf("\t\tfailed.\n\t'%i' should be false.\n", i);
        } else {
            printf("\t\tsucces.\n");
        }

        printf("Testing xmlNodeGetBool for "OUTPUTNODE"/boolean[1]: (-1)");
        if ((i = xmlNodeGetBool(node_id, "boolean[1]")) == 0) {
            printf("\tfailed.\n\t'%i' should be true\n", i);
        } else {
            printf("\tsucces.\n");
        }

        printf("Testing xmlNodeGetBool for "OUTPUTNODE"/boolean[2]: (on)");
        if ((i = xmlNodeGetBool(node_id, "boolean[2]")) == 0) {
            printf("\tfailed.\n\t'%i' should be true\n", i);
        } else {
            printf("\tsucces.\n");
        }

        printf("Testing xmlNodeGetBool for "OUTPUTNODE"/boolean[3]: (true)");
        if ((i = xmlNodeGetBool(node_id, "boolean[3]")) == 0) {
            printf("\tfailed.\n\t'%i' should be true\n", i);
        } else {
            printf("\tsucces.\n");
        }

        printf("Testing xmlNodeGetBool for "OUTPUTNODE"/boolean[4]: (yes)");
        if ((i = xmlNodeGetBool(node_id, "boolean[4]")) == 0) {
            printf("\tfailed.\n\t'%i' should be true\n", i);
        } else {
            printf("\tsucces.\n");
        }

        printf("Testing xmlNodeGetString for /*/*/test:");
        if ((s = xmlNodeGetString(root_id , "/*/*/test")) != NULL)
        {
            printf("\t\t\t\t\tfailed.\n\t'%s' should be empty\n", s);
            xmlFree(s);
        }
        else {
            printf("\t\t\t\t\tsucces.\n");
        }

        printf("Testing xmlGetString for /Configuration/output/test:");
        if ((path_id = xmlNodeGet(root_id, "/Configuration/output/test")) != NULL)
        {
            if ((s = xmlGetString(path_id)) != NULL)
            {
                printf("\t\t\tfailed.\n\t'%s' should be empty\n", s);
                xmlFree(s);
            }
            else {
                printf("\t\t\tsucces.\n");
            }
        }
        else {
            PRINT_ERROR_AND_EXIT(root_id);
        }

        path_id = xmlNodeGet(root_id, PATH);
        node_id = xmlNodeGet(root_id, MENUNODE);

        if (path_id && node_id)
        {
            char buf[BUFLEN];

            xmlCopyString(path_id, buf, BUFLEN);
            printf("Testing xmlNodeCopyString against xmlGetString:");
            if ((s = xmlGetString(path_id)) != 0)
            {
                if (strcmp(s, buf)) { /* not the same */
                    printf("\t\t\t\tfailed.\n\t'%s' differs from '%s'\n", s, buf);
                } else {
                    printf("\t\t\t\tsucces.\n");
                }

                printf("Testing xmlCopyString against xmlGetString:");
                xmlCopyString(path_id, buf, BUFLEN);
                if (strcmp(s, buf)) { /* not the same */
                    printf("\t\t\t\tfailed.\n\t'%s' differs from\n\t'%s'\n", s, buf);
                } else {
                    printf("\t\t\t\tsucces.\n");
                }
                xmlFree(s);
            }
            else {
                PRINT_ERROR_AND_EXIT(path_id);
            }

            printf("Testing xmlCopyString against xmlCompareString:");
            if (xmlCompareString(path_id, buf)) { /* not the same */
                printf ("\t\t\t\tfailed.\n\t'%s' differs from\n\t'%s'\n", s, buf);
            } else { 
                printf("\t\t\t\tsucces.\n");
            }

            printf("Testing xmlCopyString against xmlNodeCompareString:");
            if (xmlNodeCompareString(node_id, LEAFNODE, buf)) { /* not the same */
                printf("\t\t\tfailed.\n\t'%s' differs from\n\t'%s'\n", s, buf);
            } else {
                printf("\t\t\tsucces.\n");
            }

            printf("Testing xmlCopyString against xmlNodeGetString:");
            if ((s = xmlNodeGetString(node_id, LEAFNODE)) != 0)
            {
                if (strcmp(s, buf)) { /* not the same */
                    printf("\t\t\t\tfailed.\n\t'%s' differs from\n\t'%s'\n", s, buf);
                } else {
                    printf("\t\t\t\tsucces.\n");
                }
                xmlFree(s);
            }
            else {
                printf("failed.\n\t'%s' not found.\n", LEAFNODE);
            }

            xmlFree(path_id);
            if ((path_id = xmlNodeGet(root_id, "/Configuration/backend/name")) != NULL)
            {
                printf("Testing xmlAttributeCopyString against xmlAttributeCompareString:");
                xmlAttributeCopyString(path_id, "type", buf, BUFLEN);
                if (xmlAttributeCompareString(path_id, "type", buf)) { /* no match */
                    printf("\tfailed.\n\t'%s' differs\n", buf);
                } else {
                    printf("\tsucces.\n");
                }

                printf("Testing xmlAttributeCopyString against xmlAttributeGetString:");
                if ((s = xmlAttributeGetString(path_id, "type")) != 0)
                {
                     if (strcmp(s, buf)) { /* not the same */
                         printf("\t\tfailed.\n\t'%s' differs from '%s'\n", s, buf);
                     } else {
                         printf("\t\tsucces.\n");
                     }
                     xmlFree(s);
                }
                else {
                     PRINT_ERROR_AND_EXIT(path_id);
                }
            }
            else {
                PRINT_ERROR_AND_EXIT(root_id);
            }

            xmlFree(node_id);
            xmlFree(path_id);

            if ((path_id = xmlNodeGet(root_id, "Configuration/output/sample/test")) != NULL)
            {
                xmlNodeCopyString(root_id ,"Configuration/output/menu/name", buf, BUFLEN);
                printf("Testing xmlCompareString against a fixed string:");
                if (xmlCompareString(path_id, buf)) { 	/* no match */
                    printf("\t\t\tfailed.\n\t'%s' differs\n", buf);
                } else {
                    printf("\t\t\tsucces.\n");
                }

                if ((s = xmlGetString(path_id)) != 0)
                {
                    printf("Testing xmlGetString  against a fixed string:");
                    if (strcmp(s, buf)) {			/* mismatch */
                        printf("\t\t\t\tfailed.\n\t'%s' differs from\n\t'%s'\n", s, buf);
                    } else {
                        printf("\t\t\t\tsucces.\n");
                    }

                    printf("Testing xmlCopyString gainst a fixed string:");
                    xmlCopyString(path_id, buf, BUFLEN);
                    if (strcmp(s, buf)) {        		/* mismatch */
                        printf("\t\t\t\tfailed.\n\t'%s' differs from\n\t'%s'\n", s, buf);
                    } else {
                        printf("\t\t\t\tsucces.\n");
                    }
                    xmlFree(s);
                }
                else {
                    PRINT_ERROR_AND_EXIT(path_id);
                }

                xmlFree(path_id);
            }
        }

        if (xmlErrorGetNo(root_id, 0) != XML_NO_ERROR)
        {
            const char *errstr = xmlErrorGetString(root_id, 0);
            int column = xmlErrorGetColumnNo(root_id, 0);
            int lineno = xmlErrorGetLineNo(root_id, 1);

            printf("Error at line %i, column %i: %s\n", lineno, column, errstr);
        }
    }
    else {
        printf("Invalid XML-id\n");
    }
    printf("\n");

    return 0;
}
