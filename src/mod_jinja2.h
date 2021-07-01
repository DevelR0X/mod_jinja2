#include <stdlib.h>
// calloc()

#include <string.h>
// strcmp()
// strlen()
 
#include "httpd.h"
// request_rec
// HTTP_NOT_FOUND
// HTTP_INTERNAL_SERVER_ERROR
// DECLINED
// OK

#include "apr_file_info.h"
// apr_finfo_t
// APR_NOFILE
// APR_FINFO_MIN

#include "apr_hooks.h"
// ap_hook_handler()
// APR_HOOK_LAST
// STANDARD20_MODULE_STUFF
// AP_MODULE_DECLARE_DATA

#include "apr_errno.h"
// APR_SUCCESS
 
#include "http_log.h"
// ap_log_rerror()
// APLOG_MARK
// APLOG_ERR
 
#include "http_protocol.h"
// ap_rprintf
// ap_set_content_type
 
#include "http_core.h"
// ap_document_root()
 
#define PY_SSIZE_T_CLEAN
#include <Python.h>
// Py_Initialize()
// PyImport_ImportModule()
// PyObject()
// PyObject_GetAttrString()
// PyTuple_New()
// Py_BuildValue()
// PyObject_Call()
// PyErr_Occurred()
// PyErr_Fetch()
// PyUnicode_AsUTF8()
// PyObject_Repr()
// Py_DECREF()