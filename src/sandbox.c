#include <Python.h>
#include "tokenizer.h"

typedef struct {
    PyObject_HEAD
    CharStar cs;
} ReccmpTokenizer;

static PyObject* ReccmpTokenizer_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    ReccmpTokenizer* self;
    self = (ReccmpTokenizer*) type->tp_alloc(type, 0);

    if (self != NULL) {
        assert(!PyErr_Occurred());
    }

    return (PyObject*) self;
}

static int ReccmpTokenizer_init(ReccmpTokenizer* self, PyObject* args, PyObject *Py_UNUSED(kwds)) {
    const char* raw;

    if (!PyArg_ParseTuple(args, "s", &raw)) {
        return -1;
    }

    CharStar_init(&self->cs, raw);
    // Py_INCREF(raw); // borrow ?

    return 0;
}

static void ReccmpTokenizer_dealloc(ReccmpTokenizer* self) {
    // Py_XDECREF(self->cs.raw);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject* ReccmpTokenizer_next(ReccmpTokenizer* self) {
    if (CharStar_next(&self->cs)) {
        return Py_BuildValue("(i(ii)s)", self->cs.token.type, self->cs.token.line, self->cs.token.pos, self->cs.token.value);
        //PyObject *ret = PyLong_FromLong(self->cs.token.line);
        //return ret;
        // return PyTuple_Pack(4, self->cs.token.type, self->cs.token.line, self->cs.token.pos, self->cs.token.value);
    }

    return NULL;
}

static PyTypeObject ReccmpTokenizerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ReccmpTokenizer",
    .tp_basicsize = sizeof(ReccmpTokenizer),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) ReccmpTokenizer_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "ReccmpTokenizer object.",
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc) ReccmpTokenizer_next,
    .tp_init = (initproc) ReccmpTokenizer_init,
    .tp_new = ReccmpTokenizer_new
};

PyModuleDef sandbox_module = {
    PyModuleDef_HEAD_INIT,
    "_sandbox", // Module name
    "This is the module docstring",
    -1,   // Optional size of the module state memory
};

PyMODINIT_FUNC PyInit__sandbox(void)
{
    PyObject* m;
    m = PyModule_Create(&sandbox_module);
    if (m == NULL) {
        return NULL;
    }

    if (PyType_Ready(&ReccmpTokenizerType) < 0) {
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&ReccmpTokenizerType);
    if (PyModule_AddObject(m, "ReccmpTokenizer", (PyObject* ) &ReccmpTokenizerType) < 0) {
        Py_DECREF(m);
        Py_DECREF(&ReccmpTokenizerType);
        return NULL;
    }

    return m;
}
