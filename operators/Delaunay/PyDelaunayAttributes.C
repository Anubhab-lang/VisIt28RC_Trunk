/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <PyDelaunayAttributes.h>
#include <ObserverToCallback.h>
#include <stdio.h>
#include <snprintf.h>

// ****************************************************************************
// Module: PyDelaunayAttributes
//
// Purpose: 
//   Attributes for the Delaunay Operator
//
// Note:       Autogenerated by xml2python. Do not modify by hand!
//
// Programmer: xml2python
// Creation:   omitted
//
// ****************************************************************************

//
// This struct contains the Python type information and a DelaunayAttributes.
//
struct DelaunayAttributesObject
{
    PyObject_HEAD
    DelaunayAttributes *data;
    bool        owns;
    PyObject   *parent;
};

//
// Internal prototypes
//
static PyObject *NewDelaunayAttributes(int);

std::string
PyDelaunayAttributes_ToString(const DelaunayAttributes *atts, const char *prefix)
{
    std::string str;
    char tmpStr[1000];

    const char *dimension_names = "Automatic, Triangulation, Tetrahedralization";
    switch (atts->GetDimension())
    {
      case DelaunayAttributes::Automatic:
          SNPRINTF(tmpStr, 1000, "%sdimension = %sAutomatic  # %s\n", prefix, prefix, dimension_names);
          str += tmpStr;
          break;
      case DelaunayAttributes::Triangulation:
          SNPRINTF(tmpStr, 1000, "%sdimension = %sTriangulation  # %s\n", prefix, prefix, dimension_names);
          str += tmpStr;
          break;
      case DelaunayAttributes::Tetrahedralization:
          SNPRINTF(tmpStr, 1000, "%sdimension = %sTetrahedralization  # %s\n", prefix, prefix, dimension_names);
          str += tmpStr;
          break;
      default:
          break;
    }

    return str;
}

static PyObject *
DelaunayAttributes_Notify(PyObject *self, PyObject *args)
{
    DelaunayAttributesObject *obj = (DelaunayAttributesObject *)self;
    obj->data->Notify();
    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
DelaunayAttributes_SetDimension(PyObject *self, PyObject *args)
{
    DelaunayAttributesObject *obj = (DelaunayAttributesObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the dimension in the object.
    if(ival >= 0 && ival < 3)
        obj->data->SetDimension(DelaunayAttributes::Dimension(ival));
    else
    {
        fprintf(stderr, "An invalid dimension value was given. "
                        "Valid values are in the range of [0,2]. "
                        "You can also use the following names: "
                        "Automatic, Triangulation, Tetrahedralization.");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
DelaunayAttributes_GetDimension(PyObject *self, PyObject *args)
{
    DelaunayAttributesObject *obj = (DelaunayAttributesObject *)self;
    PyObject *retval = PyInt_FromLong(long(obj->data->GetDimension()));
    return retval;
}



PyMethodDef PyDelaunayAttributes_methods[DELAUNAYATTRIBUTES_NMETH] = {
    {"Notify", DelaunayAttributes_Notify, METH_VARARGS},
    {"SetDimension", DelaunayAttributes_SetDimension, METH_VARARGS},
    {"GetDimension", DelaunayAttributes_GetDimension, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//

static void
DelaunayAttributes_dealloc(PyObject *v)
{
   DelaunayAttributesObject *obj = (DelaunayAttributesObject *)v;
   if(obj->parent != 0)
       Py_DECREF(obj->parent);
   if(obj->owns)
       delete obj->data;
}

static int
DelaunayAttributes_compare(PyObject *v, PyObject *w)
{
    DelaunayAttributes *a = ((DelaunayAttributesObject *)v)->data;
    DelaunayAttributes *b = ((DelaunayAttributesObject *)w)->data;
    return (*a == *b) ? 0 : -1;
}

PyObject *
PyDelaunayAttributes_getattr(PyObject *self, char *name)
{
    if(strcmp(name, "dimension") == 0)
        return DelaunayAttributes_GetDimension(self, NULL);
    if(strcmp(name, "Automatic") == 0)
        return PyInt_FromLong(long(DelaunayAttributes::Automatic));
    if(strcmp(name, "Triangulation") == 0)
        return PyInt_FromLong(long(DelaunayAttributes::Triangulation));
    if(strcmp(name, "Tetrahedralization") == 0)
        return PyInt_FromLong(long(DelaunayAttributes::Tetrahedralization));


    return Py_FindMethod(PyDelaunayAttributes_methods, self, name);
}

int
PyDelaunayAttributes_setattr(PyObject *self, char *name, PyObject *args)
{
    // Create a tuple to contain the arguments since all of the Set
    // functions expect a tuple.
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, args);
    Py_INCREF(args);
    PyObject *obj = NULL;

    if(strcmp(name, "dimension") == 0)
        obj = DelaunayAttributes_SetDimension(self, tuple);

    if(obj != NULL)
        Py_DECREF(obj);

    Py_DECREF(tuple);
    if( obj == NULL)
        PyErr_Format(PyExc_RuntimeError, "Unable to set unknown attribute: '%s'", name);
    return (obj != NULL) ? 0 : -1;
}

static int
DelaunayAttributes_print(PyObject *v, FILE *fp, int flags)
{
    DelaunayAttributesObject *obj = (DelaunayAttributesObject *)v;
    fprintf(fp, "%s", PyDelaunayAttributes_ToString(obj->data, "").c_str());
    return 0;
}

PyObject *
DelaunayAttributes_str(PyObject *v)
{
    DelaunayAttributesObject *obj = (DelaunayAttributesObject *)v;
    return PyString_FromString(PyDelaunayAttributes_ToString(obj->data,"").c_str());
}

//
// The doc string for the class.
//
#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *DelaunayAttributes_Purpose = "Attributes for the Delaunay Operator";
#else
static char *DelaunayAttributes_Purpose = "Attributes for the Delaunay Operator";
#endif

//
// The type description structure
//
static PyTypeObject DelaunayAttributesType =
{
    //
    // Type header
    //
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                   // ob_size
    "DelaunayAttributes",                    // tp_name
    sizeof(DelaunayAttributesObject),        // tp_basicsize
    0,                                   // tp_itemsize
    //
    // Standard methods
    //
    (destructor)DelaunayAttributes_dealloc,  // tp_dealloc
    (printfunc)DelaunayAttributes_print,     // tp_print
    (getattrfunc)PyDelaunayAttributes_getattr, // tp_getattr
    (setattrfunc)PyDelaunayAttributes_setattr, // tp_setattr
    (cmpfunc)DelaunayAttributes_compare,     // tp_compare
    (reprfunc)0,                         // tp_repr
    //
    // Type categories
    //
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    //
    // More methods
    //
    0,                                   // tp_hash
    0,                                   // tp_call
    (reprfunc)DelaunayAttributes_str,        // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_CHECKTYPES,               // tp_flags
    DelaunayAttributes_Purpose,              // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0                                    // tp_weaklistoffset
};

//
// Helper functions for object allocation.
//

static DelaunayAttributes *defaultAtts = 0;
static DelaunayAttributes *currentAtts = 0;

static PyObject *
NewDelaunayAttributes(int useCurrent)
{
    DelaunayAttributesObject *newObject;
    newObject = PyObject_NEW(DelaunayAttributesObject, &DelaunayAttributesType);
    if(newObject == NULL)
        return NULL;
    if(useCurrent && currentAtts != 0)
        newObject->data = new DelaunayAttributes(*currentAtts);
    else if(defaultAtts != 0)
        newObject->data = new DelaunayAttributes(*defaultAtts);
    else
        newObject->data = new DelaunayAttributes;
    newObject->owns = true;
    newObject->parent = 0;
    return (PyObject *)newObject;
}

static PyObject *
WrapDelaunayAttributes(const DelaunayAttributes *attr)
{
    DelaunayAttributesObject *newObject;
    newObject = PyObject_NEW(DelaunayAttributesObject, &DelaunayAttributesType);
    if(newObject == NULL)
        return NULL;
    newObject->data = (DelaunayAttributes *)attr;
    newObject->owns = false;
    newObject->parent = 0;
    return (PyObject *)newObject;
}

///////////////////////////////////////////////////////////////////////////////
//
// Interface that is exposed to the VisIt module.
//
///////////////////////////////////////////////////////////////////////////////

PyObject *
DelaunayAttributes_new(PyObject *self, PyObject *args)
{
    int useCurrent = 0;
    if (!PyArg_ParseTuple(args, "i", &useCurrent))
    {
        if (!PyArg_ParseTuple(args, ""))
            return NULL;
        else
            PyErr_Clear();
    }

    return (PyObject *)NewDelaunayAttributes(useCurrent);
}

//
// Plugin method table. These methods are added to the visitmodule's methods.
//
static PyMethodDef DelaunayAttributesMethods[] = {
    {"DelaunayAttributes", DelaunayAttributes_new, METH_VARARGS},
    {NULL,      NULL}        /* Sentinel */
};

static Observer *DelaunayAttributesObserver = 0;

std::string
PyDelaunayAttributes_GetLogString()
{
    std::string s("DelaunayAtts = DelaunayAttributes()\n");
    if(currentAtts != 0)
        s += PyDelaunayAttributes_ToString(currentAtts, "DelaunayAtts.");
    return s;
}

static void
PyDelaunayAttributes_CallLogRoutine(Subject *subj, void *data)
{
    typedef void (*logCallback)(const std::string &);
    logCallback cb = (logCallback)data;

    if(cb != 0)
    {
        std::string s("DelaunayAtts = DelaunayAttributes()\n");
        s += PyDelaunayAttributes_ToString(currentAtts, "DelaunayAtts.");
        cb(s);
    }
}

void
PyDelaunayAttributes_StartUp(DelaunayAttributes *subj, void *data)
{
    if(subj == 0)
        return;

    currentAtts = subj;
    PyDelaunayAttributes_SetDefaults(subj);

    //
    // Create the observer that will be notified when the attributes change.
    //
    if(DelaunayAttributesObserver == 0)
    {
        DelaunayAttributesObserver = new ObserverToCallback(subj,
            PyDelaunayAttributes_CallLogRoutine, (void *)data);
    }

}

void
PyDelaunayAttributes_CloseDown()
{
    delete defaultAtts;
    defaultAtts = 0;
    delete DelaunayAttributesObserver;
    DelaunayAttributesObserver = 0;
}

PyMethodDef *
PyDelaunayAttributes_GetMethodTable(int *nMethods)
{
    *nMethods = 1;
    return DelaunayAttributesMethods;
}

bool
PyDelaunayAttributes_Check(PyObject *obj)
{
    return (obj->ob_type == &DelaunayAttributesType);
}

DelaunayAttributes *
PyDelaunayAttributes_FromPyObject(PyObject *obj)
{
    DelaunayAttributesObject *obj2 = (DelaunayAttributesObject *)obj;
    return obj2->data;
}

PyObject *
PyDelaunayAttributes_New()
{
    return NewDelaunayAttributes(0);
}

PyObject *
PyDelaunayAttributes_Wrap(const DelaunayAttributes *attr)
{
    return WrapDelaunayAttributes(attr);
}

void
PyDelaunayAttributes_SetParent(PyObject *obj, PyObject *parent)
{
    DelaunayAttributesObject *obj2 = (DelaunayAttributesObject *)obj;
    obj2->parent = parent;
}

void
PyDelaunayAttributes_SetDefaults(const DelaunayAttributes *atts)
{
    if(defaultAtts)
        delete defaultAtts;

    defaultAtts = new DelaunayAttributes(*atts);
}

