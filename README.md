# ZeroXML

ZeroXML is a lightweight, read-only XML library designed for reading configuration
files with minimal memory overhead. Writing or modifying XML is intentionally out
of scope: the read-only constraint keeps the memory footprint low and the
implementation simple.

To minimise allocations the library maps files directly into memory using `mmap`.
Memory is only allocated when creating an XML-id, when requesting a string value,
when requesting a node name, or when copying a node into a new memory region.

## Quick start

Open a file, read values by path, close:

```c
xmlId *xid = xmlOpen("/tmp/file.xml");
xpos = xmlNodeGetDouble(xid, "/configuration/x-pos");
ypos = xmlNodeGetDouble(xid, "/configuration/y-pos");
zpos = xmlNodeGetDouble(xid, "/configuration/z-pos");
xmlClose(xid);
```

When reading several values from the same parent node, obtain its id first.
This limits the search area and improves performance:

```c
xmlId *xnid = xmlNodeGet(id, "/configuration/setup");
version = xmlNodeGetDouble(xnid, "version");
char *s = xmlNodeGetString(xnid, "author");
if (s) { author = s; }
free(s);
xmlFree(xnid);
```

## Examples

### Walking child nodes one by one

```c
xmlId *xmid = xmlMarkId(id);
int num = xmlNodeGetNum(xmid, "*");
for (int i = 0; i < num; i++) {
    if (xmlNodeGetPos(id, xmid, "*", i) != 0) {
        char buf[1024];
        if (xmlCopyString(xmid, buf, 1024) != 0) {
            printf("%s\n", buf);
        }
    }
}
xmlFree(xmid);
```

### Reading values from the current node

```c
xnid = xmlNodeGet(id, "/path/to/last/node");
int i = xmlGetInt(xnid);
xmlFree(xnid);

xnid = xmlNodeGet(id, "/path/to/specified/node");
if (xmlCompareString(xnid, "value") == 0) printf("We have a match!\n");
xmlFree(xnid);
```

### Reading attributes

```c
int i = xmlAttributeGetInt(id, "n");

char *s = xmlAttributeGetString(id, "type");
if (s) { printf("node is of type '%s'\n", s); }
free(s);
```

### Error detection and reporting

```c
char *err_str  = xmlErrorGetString(id, 0);
int err_lineno = xmlErrorGetLineNo(id, 0);
int err        = xmlErrorGetNo(id, 1); /* also clears the error */
if (err) printf("Error #%i at line %i: '%s'\n", err, err_lineno, err_str);
```

---

## API reference

### Opening and closing

#### `xmlOpen` / `xmlOpenFlags` — open an XML file

The file is memory-mapped; no heap allocation is made for the file contents.

The `XML_COMMENT_AS_NODE`/`XML_IGNORE_COMMENT` and `XML_SCAN_NODES`/`XML_CACHE_NODES`
flags must be supplied at open time via `xmlOpenFlags`.

```c
XML_API xmlId* XML_APIENTRY xmlOpen(const char *fname);
XML_API xmlId* XML_APIENTRY xmlOpenFlags(const char *fname, enum xmlFlags flags);
```

#### `xmlInitBuffer` / `xmlInitBufferFlags` — use a pre-allocated buffer

The buffer must not be freed until `xmlClose` has been called.

```c
XML_API xmlId* XML_APIENTRY xmlInitBuffer(const char *buffer, int size);
XML_API xmlId* XML_APIENTRY xmlInitBufferFlags(const char *buffer, int size, enum xmlFlags flags);
```

#### `xmlClose` — close an XML-id

Releases the memory map and all associated resources. Must be called once for
every id returned by `xmlOpen`, `xmlOpenFlags`, `xmlInitBuffer`, `xmlInitBufferFlags`,
or `xmlNodeCopy`.

```c
XML_API void XML_APIENTRY xmlClose(xmlId *xid);
```

---

### Configuration flags

`xmlSetFlags` sets one or more operational modes on an existing XML-id. Flags
that conflict with the defaults are noted below; the default always takes
precedence when conflicting flags are supplied together.

```c
XML_API void XML_APIENTRY xmlSetFlags(const xmlId *xid, enum xmlFlags flags);
```

| Flag | Default | Description |
|------|---------|-------------|
| `XML_INDEX_STARTS_AT_ONE` | ✓ | Node indexes start at 1 |
| `XML_INDEX_STARTS_AT_ZERO` | | Node indexes start at 0 |
| `XML_RETURN_ZERO` | ✓ | Return 0 when a node does not exist |
| `XML_RETURN_NONE_VALUE` | | Return a special `NONE` sentinel when a node does not exist |
| `XML_CASE_SENSITIVE` | ✓ | Case-sensitive name comparison |
| `XML_CASE_INSENSITIVE` | | Case-insensitive name comparison |
| `XML_COMMENT_AS_NODE` | ✓ | Treat comments as nodes |
| `XML_IGNORE_COMMENT` | | Skip comment sections |
| `XML_VALIDATING` | ✓ | Report errors in the XML document |
| `XML_NONVALIDATING` | | Ignore errors in the XML document |
| `XML_CACHE_NODES` | ✓ | Cache nodes for faster repeated access |
| `XML_SCAN_NODES` | | Scan the document without caching |
| `XML_LOCALIZATION` | ✓ | Translate node content to the local character encoding |
| `XML_US_ASCII` | | Ignore character encoding declarations |

---

### Node paths

Several functions accept a *node path*: either a bare node name or a
`/`-separated path that the library walks recursively (e.g. `/config/server/port`).

- The wildcard `*` matches any node name.
- The wildcard `?` matches any single character within a name.
- A 1-based numeric index in square brackets selects a specific occurrence:
  `node[1]` or `*[3]` (the third child, regardless of name).
- Passing `XML_COMMENT` as the path tests whether the current node is a comment.

See `xmlNodeTest` for the canonical description.

---

### Tree navigation

#### `xmlNodeTest` — test whether a node path exists

Returns `XML_TRUE` if the path resolves to a node, `XML_FALSE` otherwise.

```c
XML_API int XML_APIENTRY xmlNodeTest(const xmlId *xid, const char *path);
```

#### `xmlNodeGet` — get a subsection for further processing

Returns a new XML-id whose scope is limited to the matched node, which
speeds up subsequent searches. The caller must free it with `xmlFree`.

```c
XML_API xmlId* XML_APIENTRY xmlNodeGet(const xmlId *xid, const char *path);
```

#### `xmlNodeCopy` — copy a subsection for processing after the file is closed

Like `xmlNodeGet` but makes a heap copy of the node content so the file can
be closed independently. The caller must free it with `xmlClose`.

```c
XML_API xmlId* XML_APIENTRY xmlNodeCopy(const xmlId *xid, const char *path);
```

#### `xmlMarkId` — create a reusable position marker

Required when using `xmlNodeGetNum`/`xmlNodeGetPos` to iterate over sibling
nodes. The returned id is adjusted in place by `xmlNodeGetPos` to track the
current position. Free with `xmlFree`.

```c
XML_API xmlId* XML_APIENTRY xmlMarkId(const xmlId *xid);
```

#### `xmlFree` — free an XML-id

```c
XML_API void XML_APIENTRY xmlFree(void *p);
```

#### `xmlNodeGetNum` / `xmlNodeGetNumRaw` — count nodes matching a path

Returns the number of sibling nodes at the last component of the path.
`xmlNodeGetNumRaw` preserves CDATA markers and comment delimiters in the result.

```c
XML_API int XML_APIENTRY xmlNodeGetNum(const xmlId *xid, const char *path);
XML_API int XML_APIENTRY xmlNodeGetNumRaw(const xmlId *xid, const char *path);
```

#### `xmlNodeGetPos` / `xmlNodeGetPosRaw` — get the nth occurrence of a node

Updates `xid` in place to point at the nth sibling. The return value must
not be freed by the caller. `xmlNodeGetPosRaw` preserves CDATA and comment markers.

```c
XML_API xmlId* XML_APIENTRY xmlNodeGetPos(const xmlId *pid, xmlId *xid, const char *node, int num);
XML_API xmlId* XML_APIENTRY xmlNodeGetPosRaw(const xmlId *pid, xmlId *xid, const char *node, int num);
```

#### `xmlNodeCopyPos` — copy the nth occurrence of a node

Like `xmlNodeGetPos` but returns a new independent copy. Free with `xmlFree`.

```c
XML_API xmlId* XML_APIENTRY xmlNodeCopyPos(const xmlId *pid, xmlId *xid, const char *node, int num);
```

---

### Node names

#### `xmlNodeGetName` — return the name of this node as a new string

The returned string must be freed by the caller with `xmlFree`.

```c
XML_API char* XML_APIENTRY xmlNodeGetName(const xmlId *xid);
```

#### `xmlNodeCopyName` — copy the name of this node into a caller-supplied buffer

Returns the number of bytes written.

```c
XML_API int XML_APIENTRY xmlNodeCopyName(const xmlId *xid, char *buffer, int buflen);
```

#### `xmlNodeCompareName` — compare this node's name to a string

Comparison is case-insensitive. Returns an integer less than, equal to, or
greater than zero when the name is less than, equal to, or greater than `str`.

```c
XML_API int XML_APIENTRY xmlNodeCompareName(const xmlId *xid, const char *str);
```

---

### Node string values

#### `xmlGetString` / `xmlGetStringRaw` — get the value of the current node

Returns a newly allocated string; the caller must free it with `xmlFree`.

`xmlGetString` strips leading/trailing whitespace and removes CDATA markers
and comments. `xmlGetStringRaw` returns the raw content unchanged.

```c
XML_API char* XML_APIENTRY xmlGetString(const xmlId *xid);
XML_API char* XML_APIENTRY xmlGetStringRaw(const xmlId *xid);
```

#### `xmlCopyString` — copy the value of the current node into a caller-supplied buffer

Does not allocate memory. Not safe for concurrent use on the same `xid`.
Returns the number of bytes written.

```c
XML_API int XML_APIENTRY xmlCopyString(const xmlId *xid, char *buffer, int buflen);
```

#### `xmlCompareString` — compare the value of this node to a string

Comparison is case-insensitive. Returns an integer less than, equal to, or
greater than zero when the node value is less than, equal to, or greater than `str`.

```c
XML_API int XML_APIENTRY xmlCompareString(const xmlId *xid, const char *str);
```

#### `xmlNodeGetString` — get the value of a node at a path

Returns a newly allocated string; the caller must free it with `xmlFree`.

```c
XML_API char* XML_APIENTRY xmlNodeGetString(const xmlId *xid, const char *path);
```

#### `xmlNodeCopyString` — copy the value of a node at a path into a caller-supplied buffer

Does not allocate memory. Returns the number of bytes written.

```c
XML_API int XML_APIENTRY xmlNodeCopyString(const xmlId *xid, const char *path, char *buffer, int buflen);
```

#### `xmlNodeCompareString` — compare the value of a node at a path to a string

Comparison is case-insensitive. Returns an integer less than, equal to, or
greater than zero when the node value is less than, equal to, or greater than `str`.

```c
XML_API int XML_APIENTRY xmlNodeCompareString(const xmlId *xid, const char *path, const char *str);
```

---

### Node boolean, integer and double values

#### `xmlGetBool` — boolean value of the current node

True values: `on`, `yes`, `true`, or any non-zero number. Everything else is false.

```c
XML_API int XML_APIENTRY xmlGetBool(const xmlId *xid);
```

#### `xmlNodeGetBool` — boolean value of a node at a path

```c
XML_API int XML_APIENTRY xmlNodeGetBool(const xmlId *xid, const char *path);
```

#### `xmlGetInt` — integer value of the current node

```c
XML_API long int XML_APIENTRY xmlGetInt(const xmlId *xid);
```

#### `xmlNodeGetInt` — integer value of a node at a path

```c
XML_API long int XML_APIENTRY xmlNodeGetInt(const xmlId *xid, const char *path);
```

#### `xmlGetDouble` — double value of the current node

```c
XML_API double XML_APIENTRY xmlGetDouble(const xmlId *xid);
```

#### `xmlNodeGetDouble` — double value of a node at a path

```c
XML_API double XML_APIENTRY xmlNodeGetDouble(const xmlId *xid, const char *path);
```

---

### Attributes

#### `xmlAttributeExists` — test whether a named attribute exists

Returns `XML_TRUE` or `XML_FALSE`.

```c
XML_API int XML_APIENTRY xmlAttributeExists(const xmlId *xid, const char *name);
```

#### `xmlAttributeGetNum` — return the number of attributes on this node

```c
XML_API int XML_APIENTRY xmlAttributeGetNum(const xmlId *xid);
```

#### `xmlAttributeGetName` — return the name of the nth attribute as a new string

The returned string must be freed by the caller with `xmlFree`.

```c
XML_API char* XML_APIENTRY xmlAttributeGetName(const xmlId *xid, int n);
```

#### `xmlAttributeCopyName` — copy the name of the nth attribute into a caller-supplied buffer

Returns the number of bytes written.

```c
XML_API int XML_APIENTRY xmlAttributeCopyName(const xmlId *xid, char *buffer, int buflen, int n);
```

#### `xmlAttributeCompareName` — compare the name of the nth attribute to a string

Comparison is case-insensitive. Returns an integer less than, equal to, or
greater than zero when the attribute name is less than, equal to, or greater than `str`.

```c
XML_API int XML_APIENTRY xmlAttributeCompareName(const xmlId *xid, int n, const char *str);
```

#### `xmlAttributeGetString` — get the value of a named attribute as a new string

The returned string must be freed by the caller with `xmlFree`.

```c
XML_API char* XML_APIENTRY xmlAttributeGetString(const xmlId *xid, const char *name);
```

#### `xmlAttributeCopyString` — copy the value of a named attribute into a caller-supplied buffer

Does not allocate memory. Returns the number of bytes written.

```c
XML_API int XML_APIENTRY xmlAttributeCopyString(const xmlId *xid, const char *name, char *buffer, int buflen);
```

#### `xmlAttributeCompareString` — compare the value of a named attribute to a string

Comparison is case-insensitive. Returns an integer less than, equal to, or
greater than zero when the attribute value is less than, equal to, or greater than `str`.

```c
XML_API int XML_APIENTRY xmlAttributeCompareString(const xmlId *xid, const char *name, const char *str);
```

#### `xmlAttributeGetBool` — boolean value of a named attribute

True values: `on`, `yes`, `true`, or any non-zero number. Everything else is false.

```c
XML_API int XML_APIENTRY xmlAttributeGetBool(const xmlId *xid, const char *name);
```

#### `xmlAttributeGetInt` — integer value of a named attribute

```c
XML_API long int XML_APIENTRY xmlAttributeGetInt(const xmlId *xid, const char *name);
```

#### `xmlAttributeGetDouble` — double value of a named attribute

```c
XML_API double XML_APIENTRY xmlAttributeGetDouble(const xmlId *xid, const char *name);
```

---

### Error reporting

Error information is stored per XML-id. Pass `clear = 1` to reset the error
state after reading it.

#### `xmlErrorGetNo` — error code of the last error

Returns 0 when no error has been recorded.

```c
XML_API int XML_APIENTRY xmlErrorGetNo(const xmlId *xid, int clear);
```

#### `xmlErrorGetLineNo` — line number of the last error

```c
XML_API int XML_APIENTRY xmlErrorGetLineNo(const xmlId *xid, int clear);
```

#### `xmlErrorGetColumnNo` — column number of the last error

```c
XML_API int XML_APIENTRY xmlErrorGetColumnNo(const xmlId *xid, int clear);
```

#### `xmlErrorGetString` — human-readable description of the last error

```c
XML_API const char* XML_APIENTRY xmlErrorGetString(const xmlId *xid, int clear);
```

---

### Encoding

#### `xmlGetEncoding` — encoding declared in the XML prolog

Returns the encoding string as declared in `<?xml version="1.0" encoding="..."?>`,
or an empty string if no declaration was present.

```c
XML_API const char* XML_APIENTRY xmlGetEncoding(const xmlId *xid);
```
