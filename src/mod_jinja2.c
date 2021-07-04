/*
 * mod_jinja2.c - Jinja2 module for Apache httpd server
 * Author: Yhojann Aguilera Aguayo <yhojann.aguilera@gmail.com>
 *
 * MIT License
 * 
 * Copyright (c) 2021 DevelR0X
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mod_jinja2.h"

/**
 * Event: On HTTP request in thread.
 */
static int on_request(request_rec *r) {

    // Is handled?
    if (!r->handler || strcmp(r->handler, "jinja2-handler")){
        return DECLINED;
    }

    // File exist?
    {
        apr_finfo_t finfo;

        // Can load file?
        if (apr_stat(&finfo, r->filename, APR_FINFO_MIN, r->pool) != APR_SUCCESS) {
            return HTTP_NOT_FOUND;
        }

        // File exist and is not directory?
        if(!((finfo.filetype != APR_NOFILE) && (!(finfo.filetype & APR_DIR)))) {
            return HTTP_NOT_FOUND;
        }
    }

    // Current normalized public html (DOCUMENT_LOCATION gets only default)
    const char *document_root = realpath(ap_document_root(r), NULL);

    // Load Jinja2 module
    PyObject *py_jinja2 = PyImport_ImportModule("jinja2");
    if(py_jinja2 == NULL) {

        // Apache log out
        ap_log_rerror(
            APLOG_MARK, APLOG_ERR, 0, r,
            "mod_jinja2 error: "
            "`Jinja2` module for python 3 is required"
        );

        // User error out
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    // Get jinja2 file system loader
    PyObject *fs_loader;
    {
        PyObject *function = PyObject_GetAttrString(py_jinja2, "FileSystemLoader");
        PyObject *args = PyTuple_New(0);
        PyObject *kwargs = Py_BuildValue("{s:s}", "searchpath", document_root);
        fs_loader = PyObject_Call(function, args, kwargs);

        Py_DECREF(kwargs);
        Py_DECREF(args);
        Py_DECREF(function);
    }

    if(fs_loader == NULL) {

        // Apache log out
        ap_log_rerror(
            APLOG_MARK, APLOG_ERR, 0, r,
            "mod_jinja2 error: "
            "`jinja2.FileSystemLoader('%s')` "
            "return a null pointer", document_root
        );

        // Decrease all python pointers
        Py_DECREF(py_jinja2);

        // User error out
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    // Get jinja2 template enviroment
    PyObject *template_env;
    {
        PyObject *function = PyObject_GetAttrString(py_jinja2, "Environment");
        PyObject *args = PyTuple_New(0);
        PyObject *kwargs = Py_BuildValue("{s:O}", "loader", fs_loader);
        template_env = PyObject_Call(function, args, kwargs);

        Py_DECREF(kwargs);
        Py_DECREF(args);
        Py_DECREF(function);
    }

    if(template_env == NULL) {

        // Apache log out
        ap_log_rerror(
            APLOG_MARK, APLOG_ERR, 0, r,
            "mod_jinja2 error: "
            "`jinja2.Environment(Jinja2.FileSystemLoader('%s'))` "
            "return a null pointer", document_root
        );

        // Decrease all python pointers
        Py_DECREF(fs_loader);
        Py_DECREF(py_jinja2);

        // User error out
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    // Secure substraction for relative filename
    char *relative_filename = calloc(strlen(r->filename) - strlen(document_root), 1);
    long unsigned int i;
    for(i = strlen(document_root) + 1; i <= strlen(r->filename); i++) {
        relative_filename[i - strlen(document_root) - 1] = r->filename[i];
    }

    // Substraction for request uri (r->uri is broken when use mod_rewrite)
    char *request_uri = calloc(strlen(r->the_request), 1);
    for(i = strlen(r->method) + 1; i <= (strlen(r->the_request) - strlen(r->protocol) - 1); i++) {
        request_uri[i - strlen(r->method) - 1] = r->the_request[i];
    }

    // Get jinja2 template object with the context
    PyObject *template;
    {
        PyObject *function = PyObject_GetAttrString(template_env, "get_template");
        PyObject *args = PyTuple_New(0);
        PyObject *kwargs = Py_BuildValue("{s:s}", "name", relative_filename);
        template = PyObject_Call(function, args, kwargs);

        Py_DECREF(kwargs);
        Py_DECREF(args);
        Py_DECREF(function);
    }

    if(template == NULL) {

        // Apache log out
        ap_log_rerror(
            APLOG_MARK, APLOG_ERR, 0, r,
            "mod_jinja2 error: "
            "`jinja2.Environment(Jinja2.FileSystemLoader('%s')).get_template('%s')` "
            "return a null pointer", document_root, relative_filename
        );

        // Free all
        free(relative_filename);
        free(request_uri);

        // Decrease all python pointers
        Py_DECREF(template_env);
        Py_DECREF(fs_loader);
        Py_DECREF(py_jinja2);

        // User error out
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    // Render template
    PyObject* template_output;
    {
        PyObject *py_os       = PyImport_ImportModule("os");
        PyObject *py_io       = PyImport_ImportModule("io");
        PyObject *py_datetime = PyImport_ImportModule("datetime");
        PyObject *py_time     = PyImport_ImportModule("time");
        PyObject *py_re       = PyImport_ImportModule("re");
        PyObject *py_json     = PyImport_ImportModule("json");
        PyObject *py_glob     = PyImport_ImportModule("glob");

        PyObject *function = PyObject_GetAttrString(template, "render");
        PyObject *args = PyTuple_New(0);
        PyObject *kwargs = Py_BuildValue(
            "{sOsOsOsOsOsOsOssssss}",
            "os",            py_os,
            "io",            py_io,
            "datetime",      py_datetime,
            "time",          py_time,
            "re",            py_re,
            "json",          py_json,
            "glob",          py_glob,
            "filename",      relative_filename,
            "document_root", document_root,
            "uri",           request_uri
        );
        template_output = PyObject_Call(function, args, kwargs);

        Py_DECREF(kwargs);
        Py_DECREF(args);
        Py_DECREF(function);
        Py_DECREF(py_glob);
        Py_DECREF(py_json);
        Py_DECREF(py_re);
        Py_DECREF(py_time);
        Py_DECREF(py_datetime);
        Py_DECREF(py_io);
        Py_DECREF(py_os);
    }

    if(template_output == NULL) {

        if(PyErr_Occurred()) {

            // Get exception
            PyObject *ptype;
            PyObject *pvalue;
            PyObject *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *err_message = PyUnicode_AsUTF8(PyObject_Repr(pvalue));

            // Apache log out
            ap_log_rerror(
                APLOG_MARK, APLOG_ERR, 0, r,
                "mod_jinja2 template syntax error: %s in %s/%s",
                err_message, document_root, relative_filename
            );

            Py_DECREF(ptraceback);
            Py_DECREF(pvalue);
            Py_DECREF(ptype);

        } else {
            // Apache log out
            ap_log_rerror(
                APLOG_MARK, APLOG_ERR, 0, r,
                "mod_jinja2 error: "
                "`jinja2.Environment(Jinja2.FileSystemLoader('%s')).get_template('%s').render()` "
                "return a null pointer without exception",
                document_root, relative_filename
            );
        }

        // Free all
        free(relative_filename);
        free(request_uri);

        // Decrease all python pointers
        Py_DECREF(template);
        Py_DECREF(template_env);
        Py_DECREF(fs_loader);
        Py_DECREF(py_jinja2);

        // User error out
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    const char *template_rendered = PyUnicode_AsUTF8(template_output);

    // Header content type
    ap_set_content_type(r, "text/html");

    // Out
    ap_rprintf(r, "%s", template_rendered);

    // Free all
    free(relative_filename);
    free(request_uri);

    // Decrease all python pointers
    Py_DECREF(template_output);
    Py_DECREF(template);
    Py_DECREF(template_env);
    Py_DECREF(fs_loader);
    Py_DECREF(py_jinja2);

    // Status 200 OK
    return OK;
}

/**
 * Register module events.
 */
static void register_events(apr_pool_t *pool) {
    Py_Initialize();
    ap_hook_handler(on_request, NULL, NULL, APR_HOOK_LAST);
    // Py_Finalize();
}

// Declare module
module AP_MODULE_DECLARE_DATA mod_jinja2 = {
    STANDARD20_MODULE_STUFF, NULL, NULL, NULL, NULL, NULL, register_events
};
