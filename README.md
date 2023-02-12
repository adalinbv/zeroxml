# **ZeroXML**
This library is specially designed for reading xml configuration files
and to be as low on memory management as possible. Modifying or writing
xml files is not planned for the future. In most situations being able
to gather data by reading an xml file is more than enough and the read-only
decision provides a number of advantages over a one-size fits all approach.
For instance the memory footprint can be kept low and the library can be
kept simple.

To achieve these goals the mmap function is used to map the configuration file
to a memory region. The only places where memory is allocated is when creating
a new XML-id, when requesting a string from a node, when requesting the node
name or when a request is made to copy a node into a new memory region.

Using this library should be pretty simple for most tasks; just open a file,
read every parameter one by one and close the id again.
`
   xmlId *xid;

   xid = xmlOpen("/tmp/file.xml");
   xpos = xmlNodeGetDouble(xid, "/configuration/x-pos");
   ypos = xmlNodeGetDouble(xid, "/configuration/y-pos");
   zpos = xmlNodeGetDouble(xid, "/configuration/z-pos");
   xmlClose(xid);
`

While it is certainly possible to access every node directly by calling the
xmlNodeGet(Bool/Int/Double/String) functions, when more than one node need to be
gathered from a parent node it is advised to get the id of the parent node
and work from there since the XML-id holds the boundaries of the (parent)node
which limits the searching area resulting in improved searching speed.
`
   xmlId *xnid;
   char *s;

   xnid = xmlNodeGet(id, "/configuration/setup/");
   version = xmlNodeGetDouble(xnid, "version");
   s = xmlNodeGetString(xnid, "author");
   if (s) author = s;
   free(s);
   xmlFree(xnid);
`

## Examples:
-----------------------------------------------------------------------------

### Functions to walk the node tree and process them one by one.
```
  xmid = xmlMarkId(id);
  num = xmlNodeGetNum(xmid, "*");
  for (i=0; i < num; i++) {
     if (xmlNodeGetPos(id, xmid, "*", i) != 0) {
        char buf[1024];
        if ((s = xmlCopyString(xmid, buf, 1024)) != 0) {
           printf("%s\n", s);
        }
     }
  }
  xmlFree(xmid);
```


### These functions work on the current node.
```
  xnid = xmlNodeGet(id, "/path/to/last/node");
  i = xmlGetInt(xnid);
  xmlFree(xnid);
 
  xnid = xmlNodeGet(id, "/path/to/specified/node");
  if (xmlCompareString(xnid, "value") == 0) printf("We have a match!\n");
  xmlFree(xnid);
 ```


### These functions work on a specified atribute
```
  i = xmlAttributeGetInt(id, "n");
 
  s = xmlAttributeGetString(id, "type");
  if (s) printf("node is of type '%s'\n", s);
  free(s);
```


### Error detection and reporting functions
```
  char *err_str = xmlErrorGetString(id, 0);
  size_t err_lineno = xmlErrorGetLineNo(id, 0);
  int err = xmlErrorGetNo(id, 1); /* clear last error */
  if (err) printf("Error #%i at line %u: '%s'\n", err, err_lineno, err_str);
```


## Overview of the available functions:
-----------------------------------------------------------------------------

* Open an XML file for processing.
 
  No data is being allocated for the file. All actions are in mmap-ed
  file buffers.
 
  @param fname path to the file
  @return XML-id which is used for further processing*
```
XML_API xmlId* XML_APIENTRY xmlOpen(const char *fname);
```


* Process a section of XML code in a preallocated buffer.
  The buffer may not be freed until xmlClose has been called.
 
  @param buffer pointer to the buffer
  @param size size of the buffer
  @return XML-id which is used for further processing*
```
XML_API xmlId* XML_APIENTRY xmlInitBuffer(const char *buffer, int size);
```


* Close the XML file after which no further processing is possible.
 
  @param xid XML-id*
```
XML_API void XML_APIENTRY xmlClose(xmlId *xid);
```


* Test whether the node path exists.
 
  A node path may be a solitary node name or a node path separated by the
  slash character '/' (in which case the code walks the XML tree).
 
  Node names adhere to the XML convention for valid node names, may be the
  asterisk character '*' to indicate that any name is acceptable or may
  contain a question mark '?' to indicate that any character is acceptable
  for that particular location.
  Node names may also specify which occurrence of a particular name to look
  up by specifying the number, starting at zero, between straight brackets.
  e.g. node[0] or "*[3]" to get the fourth node, regardless of the names.
 
  If node points to XML_COMMENT then the function will test whether the
  current node is a comment node.
 
  @param xid XML-id
  @param path path to the node containing the subsection
  @return true if the XML-subsection-id exists, false otherwise.*
```XML_API int XML_APIENTRY xmlNodeTest(const xmlId *xid, const char *path); ```


> Locate a subsection of the XML tree for further processing.
> The memory allocated for the XML-subsection-id has to be freed by the
> calling process using xmlFree.
>
> This method adds processing speed since the required nodes will only be
> searched in the subsection.
>
> For a description of node paths see xmlNodeTest.
>
> @param xid XML-id
> @param path path to the node containing the subsection
> @return XML-subsection-id for further processing
`XML_API xmlId* XML_APIENTRY xmlNodeGet(const xmlId *xid, const char *path);`


# This is useful when it's required to process a section of the XML code
# after the file has been closed. The drawback is the added memory
# requirements.
#
# For a description of node paths see xmlNodeTest.
#
# The memory allocated for the XML-subsection-id has to be freed by the
# calling process using xmlClose.
#
# @param xid XML-id
# @param path path to the node containing the subsection
# @return XML-subsection-id for further processing
XML_API xmlId* XML_APIENTRY xmlNodeCopy(const xmlId *xid, const char *path);


# Return the name of this node.
# The returned string has to be freed by the calling process using xmlFree.
#
# @param xid XML-id
# @return a newly allocated string containing the node name
XML_API char* XML_APIENTRY xmlNodeGetName(const xmlId *xid);


# Copy the name of this node in a pre-allocated buffer.
#
# @param xid XML-id
# @param buffer the buffer to copy the string to
# @param buflen length of the destination buffer
# @return the length of the node name
XML_API int XML_APIENTRY xmlNodeCopyName(const xmlId *xid, char *buffer, int buflen);


# Compare the name of a node to a string.
# Comparing is done in a case insensitive way.
#
# @param xid XML-id
# @param str the string to compare to
# @return an integer less than, equal to, or greater than zero if the value
# of the node is found, respectively, to be less than, to match, or be greater
# than str
XML_API int XML_APIENTRY xmlNodeCompareName(const xmlId *xid, const char *str);


# Return the name of the nth attribute.
#
# @param xid XML-id
# @param n position of the attribute in the attribute list. starts at 0.
# @return a newly allocated string containing the name of the attribute.
XML_API char* XML_APIENTRY xmlAttributeGetName(const xmlId *xid, int n);


# Copy the name of the nth attribute in a pre-allocated buffer.
#
# @param xid XML-id
# @param buffer the buffer to copy the string to
# @param buflen length of the destination buffer
# @param n position of the attribute in the attribute list. starts at 0.
# @return the length of the attribute name
XML_API int XML_APIENTRY xmlAttributeCopyName(const xmlId *xid, char *buffer, int buflen, int n);


# Compare the name of the nth attribute to a string.
# Comparing is done in a case insensitive way.
#
# @param xid XML-id
# @param n position of the attribute in the attribute list. starts at 0.
# @param str the string to compare to
# @return an integer less than, equal to, or greater than zero if the value
# of the node is found, respectively, to be less than, to match, or be greater
# than str
XML_API int XML_APIENTRY xmlAttributeCompareName(const xmlId *xid, int n, const char *str);


# Create a marker XML-id that starts out with the same settings as the
# reference XML-id.
# The returned XML-id has to be freed by the calling process using xmlFree.
#
# Marker id's are required when xmlNodeGetNum() and xmlNodeGetPos() are used
# to walk a number of nodes. The xmlNodeGetPos function adjusts the contents
# of the provided XML-id to keep track of it's position within the XML section.
# The returned XML-id is limited to the boundaries of the requested XML tag.
#
# @param xid reference XML-id
# @return a copy of the reference XML-id
XML_API xmlId* XML_APIENTRY xmlMarkId(const xmlId *xid);


# Free an XML-id.
#
# @param p a pointer to the memory location to be freed.
XML_API void XML_APIENTRY xmlFree(void *p);


# Get the number of nodes with the same name from a specified XML path.
#
# For a description of node paths see xmlNodeTest.
#
# @param xid XML-id
# @param path path to the XML node
# @return the number count of the nodename
XML_API int XML_APIENTRY xmlNodeGetNum(const xmlId *xid, const char *path);
XML_API int XML_APIENTRY xmlNodeGetNumRaw(const xmlId *xid, const char *path);


# Get the number of attributes for this node.
#
# @param xid XML-id
# @return the number count of the node
XML_API int XML_APIENTRY xmlAttributeGetNum(const xmlId *xid);


# Get the nth occurrence of node in the parent node.
# The return value should never be altered or freed by the caller.
#
# @param pid XML-id of the parent node of this node
# @param xid XML-id
# @param node name of the node to search for
# @param num specify which occurrence to return. starts at 0.
# @return XML-subsection-id for further processing or NULL if unsuccessful
XML_API xmlId* XML_APIENTRY xmlNodeGetPos(const xmlId *pid, xmlId *xid, const char *node, int num);
XML_API xmlId* XML_APIENTRY xmlNodeGetPosRaw(const xmlId *pid, xmlId *xid, const char *node, int num);


# Copy the nth occurrence of node in the parent node.
# The return value should be freed by the caller using xmlFree.
#
# @param pid XML-id of the parent node of this node
# @param xid XML-id which will get unusbale after the call, use the returned
#            id as the new XML-id
# @param node name of the node to search for
# @param num specify which occurrence to return. starts at 0.
# @return XML-subsection-id for further processing or NULL if unsuccessful
XML_API xmlId* XML_APIENTRY xmlNodeCopyPos(const xmlId *pid, xmlId *xid, const char *node, int num);


# Get a string of characters from the current node.
# The returned string has to be freed by the calling process using xmlFree.
#
# xmlGetStringRaw returns the unformatted string including leading and trailing
# spaces and includes comments and the ![CDATA[]] sequence in the result.
#
# xmlGetString does not include comments and the ![CDATA[]] sequence in the
# result and omits leading and trailing spaces.
#
# @param xid XML-id
# @return a newly allocated string containing the contents of the node
XML_API char* XML_APIENTRY xmlGetString(const xmlId *xid);
XML_API char* XML_APIENTRY xmlGetStringRaw(const xmlId *xid);


# Get a string of characters from the current node.
# This function has the advantage of not allocating its own return buffer,
# keeping the memory management to an absolute minimum but the disadvantage
# is that it's unreliable in multithread environments.
#
# @param xid XML-id
# @param buffer the buffer to copy the string to
# @param buflen length of the destination buffer
# @return the length of the string
XML_API int XML_APIENTRY xmlCopyString(const xmlId *xid, char *buffer, int buflen);


# Compare the value of this node to a reference string.
# Comparing is done in a case insensitive way.
#
# @param xid XML-id
# @param str the string to compare to
# @return an integer less than, equal to, or greater than zero if the value
# of the node is found, respectively, to be less than, to match, or be greater
# than str
XML_API int XML_APIENTRY xmlCompareString(const xmlId *xid, const char *str);


# Get a string of characters from a specified XML path.
# The returned string has to be freed by the calling process using xmlFree.
#
# For a description of node paths see xmlNodeTest.
#
# @param xid XML-id
# @param path path to the XML node
# @return a newly allocated string containing the contents of the node
XML_API char* XML_APIENTRY xmlNodeGetString(const xmlId *xid, const char *path);


# Get a string of characters from a specified XML path.
#
# This function has the advantage of not allocating its own return buffer,
# keeping the memory management to an absolute minimum but the disadvantage
# is that it's unreliable in multithread environments.
#
# For a description of node paths see xmlNodeTest.
#
# @param xid XML-id
# @param path path to the XML node
# @param buffer the buffer to copy the string to
# @param buflen length of the destination buffer
# @return the length of the string
XML_API int XML_APIENTRY xmlNodeCopyString(const xmlId *xid, const char *path, char *buffer, int buflen);


# Compare the value of a node to the value of a node at a specified XML path.
# Comparing is done in a case insensitive way.
#
# For a description of node paths see xmlNodeTest.
#
# @param xid XML-id
# @param path path to the XML node to compare to
# @param str the string to compare to
# @return an integer less than, equal to, or greater than zero if the value
# of the node is found, respectively, to be less than, to match, or be greater
# than str
XML_API int XML_APIENTRY xmlNodeCompareString(const xmlId *xid, const char *path, const char *str);


# Get a string of characters from a named attribute.
# The returned string has to be freed by the calling process using xmlFree.
#
# @param xid XML-id
# @param name name of the attribute to acquire
# @return the contents of the node converted to an integer value
XML_API char* XML_APIENTRY xmlAttributeGetString(const xmlId *xid, const char *name);


 * Get a string of characters from a named attribute.
 * This function has the advantage of not allocating its own return buffer,
 * keeping the memory management to an absolute minimum but the disadvantage
 * is that it's unreliable in multithread environments.
 *
 * @param xid XML-id
 * @param name name of the attribute to acquire.
 * @param buffer the buffer to copy the string to
 * @param buflen length of the destination buffer
 * @return the length of the string
 */
XML_API int XML_APIENTRY xmlAttributeCopyString(const xmlId *xid, const char *name, char *buffer, int buflen);


# Compare the value of an attribute to a reference string.
# Comparing is done in a case insensitive way.
#
# @param xid XML-id
# @param name name of the attribute to acquire.
# @param str the string to compare to
# @return an integer less than, equal to, or greater than zero if the value
# of the node is found, respectively, to be less than, to match, or be greater
# than str
XML_API int XML_APIENTRY xmlAttributeCompareString(const xmlId *xid, const char *name, const char *str);


# Get the boolean value from the current node/
#
# @param xid XML-id
# @return the contents of the node converted to an boolean value
XML_API int XML_APIENTRY xmlGetBool(const xmlId *xid);


# Get an boolean value from a specified XML path.
#
# For a description of node paths see xmlNodeTest.
#
# Boolean values for true are (without quotes) 'on', 'yes', 'true' or
# any non-zero number. Boolean values for false is anything else.
#
# @param xid XML-id
# @param path path to the XML node
# @return the contents of the node converted to an boolean value
XML_API int XML_APIENTRY xmlNodeGetBool(const xmlId *xid, const char *path);


 * Get the boolean value from the named attribute.
 *
 * Boolean values for true are (without quotes) 'on', 'yes', 'true' or
 * any non-zero number. Boolean values for false is anything else.
 *
 * @param xid XML-id
 * @param name name of the attribute to acquire
 * @return the contents of the node converted to an boolean value
 */
XML_API int XML_APIENTRY xmlAttributeGetBool(const xmlId *xid, const char *name);


# Get the integer value from the current node/
#
# @param xid XML-id
# @return the contents of the node converted to an integer value
XML_API long int XML_APIENTRY xmlGetInt(const xmlId *xid);


# Get an integer value from a specified XML path.
#
# For a description of node paths see xmlNodeTest.
#
# @param xid XML-id
# @param path path to the XML node
# @return the contents of the node converted to an integer value
XML_API long int XML_APIENTRY xmlNodeGetInt(const xmlId *xid, const char *path);


# Get the integer value from the named attribute.
#
# @param xid XML-id
# @param name name of the attribute to acquire
# @return the contents of the node converted to an integer value
XML_API long int XML_APIENTRY xmlAttributeGetInt(const xmlId *xid, const char *name);


# Get the double value from the curent node/
#
# @param xid XML-id
# @return the contents of the node converted to a double value
XML_API double XML_APIENTRY xmlGetDouble(const xmlId *xid);

# Get a double value from a specified XML path/
#
# For a description of node paths see xmlNodeTest.
#
# @param xid XML-id
# @param path path to the XML node
# @return the contents of the node converted to a double value
XML_API double XML_APIENTRY xmlNodeGetDouble(const xmlId *xid, const char *path);

# Get the double value from the named attribute.
#
# @param xid XML-id
# @param name name of the attribute to acquire
# @return the contents of the node converted to an integer value
XML_API double XML_APIENTRY xmlAttributeGetDouble(const xmlId *xid, const char *name);


# Test whether the named attribute does exist.
#
# @param xid XML-id
# @param name name of the attribute to acquire
# @return the contents of the node converted to an integer value
XML_API int XML_APIENTRY xmlAttributeExists(const xmlId *xid, const char *name);


# Get the error number of the last error and clear it.
#
# @param xid XML-id
# @param clear clear the error state if non zero
# @return the numer of the last error, 0 means no error detected.
XML_API int XML_APIENTRY xmlErrorGetNo(const xmlId *xid, int clear);


# Get the line number of the last detected syntax error in the XML file.
#
# @param xid XML-id
# @param clear clear the error state if non zero
# @return the line number of the detected syntax error.
XML_API int XML_APIENTRY xmlErrorGetLineNo(const xmlId *xid, int clear);


# Get the column number of the last detected syntax error in the XML file.
#
# @param xid XML-id
# @param clear clear the error state if non zero
# @return the line number of the detected syntax error.
XML_API int XML_APIENTRY xmlErrorGetColumnNo(const xmlId *xid, int clear);


# Get a string that explains the last error.
#
# @param xid XML-id
# @param clear clear the error state if non zero
# @return a string that explains the last error.
XML_API const char* XML_APIENTRY xmlErrorGetString(const xmlId *xid, int clear);


# Get the encoding as specified by the XML document.
#
# @param xid XML-id
# @return a string containing the encoding as specified by the XML document
XML_API const char* XML_APIENTRY xmlGetEncoding(const xmlId *xid);
