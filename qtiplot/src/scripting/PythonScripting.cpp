/***************************************************************************
	File                 : PythonScripting.cpp
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Knut Franke
	Email (use @ for *)  : knut.franke*gmx.de
	Description          : Execute Python code from within QtiPlot

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
// get rid of a compiler warning
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#include <Python.h>
#include <compile.h>
#include <frameobject.h>
#include <traceback.h>

#define PyString_Check PyUnicode_Check
#define PyString_AsString PyUnicode_AsUTF8
#define PyString_FromString PyUnicode_FromString
#define PyString_AS_STRING PyUnicode_AsUTF8
#define PyInt_Check PyLong_Check
#define PyInt_AsLong PyLong_AsLong
#define PyInt_FromLong PyLong_FromLong
#define PyInt_AS_LONG PyLong_AsLong

#include "PythonScript.h"
#include "PythonScripting.h"
#include <ApplicationWindow.h>

#include <QObject>
#include <QStringList>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QMessageBox>

// includes sip.h, which undefines Qt's "slots" macro since SIP 4.6
#include "sip.h"
extern "C" PyObject* PyInit_qti();

const char* PythonScripting::langName = "Python";

QString PythonScripting::toString(PyObject *object, bool decref)
{
	PyGILState_STATE state = PyGILState_Ensure();
	QString ret("");
	if (object)
	{
		PyObject *repr = PyObject_Str(object);
		if (decref) Py_DECREF(object);
		if (repr)
		{
			ret = PyString_AsString(repr);
			Py_DECREF(repr);
		}
	}
	PyGILState_Release(state);
	return ret;
}

PyObject *PythonScripting::eval(const QString &code, PyObject *argDict, const char *name)
{
	PyGILState_STATE state = PyGILState_Ensure();
	PyObject *args;
	if (argDict)
		args = argDict;
	else
		args = globals;
	PyObject *ret=nullptr;
	PyObject *co = Py_CompileString(code.toUtf8().constData(), name, Py_eval_input);
	if (co)
	{
		ret = PyEval_EvalCode(co, globals, args);
		Py_DECREF(co);
	}
	PyGILState_Release(state);
	return ret;
}

bool PythonScripting::exec (const QString &code, PyObject *argDict, const char *name)
{
	PyGILState_STATE state = PyGILState_Ensure();
	PyObject *args;
	if (argDict)
		args = argDict;
	else
		// "local" variable assignments automatically become global:
		args = globals;
	PyObject *tmp = nullptr;
	PyObject *co = Py_CompileString(code.toUtf8().constData(), name, Py_file_input);
	if (co)
	{
		tmp = PyEval_EvalCode(co, globals, args);
		Py_DECREF(co);
	}
	if (tmp) Py_DECREF(tmp);
	PyGILState_Release(state);
	return (bool) tmp;
}

QString PythonScripting::errorMsg()
{
	PyGILState_STATE state = PyGILState_Ensure();
	PyObject *exception=0, *value=0, *traceback=0;
	PyTracebackObject *excit=0;
	PyFrameObject *frame;
	const char *fname;
	QString msg;
	if (!PyErr_Occurred())
	{
		PyGILState_Release(state);
		return "";
	}
	PyErr_Fetch(&exception, &value, &traceback);
	PyErr_NormalizeException(&exception, &value, &traceback);
	if(PyErr_GivenExceptionMatches(exception, PyExc_SyntaxError))
	{
		QString text = toString(PyObject_GetAttrString(value, "text"), true);
		msg.append(text + "\n");
		PyObject *offset = PyObject_GetAttrString(value, "offset");
		for (int i=0; i<(PyInt_AsLong(offset)-1); i++)
			if (text[i] == '\t')
				msg.append("\t");
			else
				msg.append(" ");
		msg.append("^\n");
		Py_DECREF(offset);
		msg.append("SyntaxError: ");
		msg.append(toString(PyObject_GetAttrString(value, "msg"), true) + "\n");
		msg.append("at ").append(toString(PyObject_GetAttrString(value, "filename"), true));
		msg.append(":").append(toString(PyObject_GetAttrString(value, "lineno"), true));
		msg.append("\n");
		Py_DECREF(exception);
		Py_DECREF(value);
	} else {
		msg.append(toString(exception,true)).remove("exceptions.").append(": ");
		msg.append(toString(value,true));
		msg.append("\n");
	}

	if (traceback) {
		excit = (PyTracebackObject*)traceback;
		while (excit && (PyObject*)excit != Py_None)
		{
			frame = excit->tb_frame;
			PyCodeObject *code = PyFrame_GetCode(frame);
			msg.append("at ").append(toString(PyObject_GetAttrString((PyObject*)code, "co_filename"), true));
			msg.append(":").append(QString::number(excit->tb_lineno));
			PyObject *co_name = PyObject_GetAttrString((PyObject*)code, "co_name");
			if (co_name && *(fname = PyString_AsString(co_name)) != '?')
				msg.append(" in ").append(fname);
			Py_XDECREF(co_name);
			Py_DECREF(code);
			msg.append("\n");
			excit = excit->tb_next;
		}
		Py_DECREF(traceback);
	}

	PyGILState_Release(state);
	return msg;
}

PythonScripting::PythonScripting(ApplicationWindow *parent)
	: ScriptingEnv(parent, langName)
{
	PyGILState_STATE state;
	PyObject *mainmod=nullptr, *qtimod=nullptr, *sysmod=nullptr;
	math = nullptr;
	sys = nullptr;
	d_initialized = false;
	if (Py_IsInitialized())
	{
		state = PyGILState_Ensure();
		mainmod = PyImport_ImportModule("__main__");
		if (!mainmod)
		{
			PyErr_Print();
			PyGILState_Release(state);
			return;
		}
		globals = PyModule_GetDict(mainmod);
	} else {
		PyImport_AppendInittab("qti", PyInit_qti);
		Py_Initialize ();
		if (!Py_IsInitialized ())
			return;

		mainmod = PyImport_AddModule("__main__");

		mainmod = PyImport_AddModule("__main__");
		if (!mainmod)
		{
			PyErr_Print();
			PyEval_SaveThread();
			return;
		}
		globals = PyModule_GetDict(mainmod);

		/* Swap out and return current thread state and release the GIL */
		/*PyThreadState *tstate =*/
		PyEval_SaveThread();
	}

	state = PyGILState_Ensure();
	if (!globals)
	{
		PyErr_Print();
		PyGILState_Release(state);
		return;
	}
	Py_INCREF(globals);

	math = PyDict_New();
	if (!math)
		PyErr_Print();

	PyObject *qtcore = PyImport_ImportModule("PyQt6.QtCore");
	if (qtcore) Py_DECREF(qtcore); else PyErr_Print();
	PyObject *qtgui = PyImport_ImportModule("PyQt6.QtGui");
	if (qtgui) Py_DECREF(qtgui); else PyErr_Print();
	PyObject *qtwidgets = PyImport_ImportModule("PyQt6.QtWidgets");
	if (qtwidgets) Py_DECREF(qtwidgets); else PyErr_Print();

	qtimod = PyImport_ImportModule("qti");
	if (qtimod)
	{
		PyDict_SetItemString(globals, "qti", qtimod);
		PyObject *qtiDict = PyModule_GetDict(qtimod);
		if (!setQObject(d_parent, "app", qtiDict))
			QMessageBox::warning(d_parent, tr("Failed to export QtiPlot API"),
			tr("Accessing QtiPlot functions or objects from Python code won't work. "\
			"Probably your version of Qt/SIP/PyQt differs from the one QtiPlot was compiled against."));

		PyDict_SetItemString(qtiDict, "mathFunctions", math);
		Py_DECREF(qtimod);
	} else
		PyErr_Print();

	sysmod = PyImport_ImportModule("sys");
	if (sysmod)
	{
		sys = PyModule_GetDict(sysmod);
		Py_INCREF(sys);
	} else
		PyErr_Print();

	PyGILState_Release(state);
	d_initialized = true;
}

bool PythonScripting::initialize()
{
	if (!d_initialized) return false;

	PyGILState_STATE state = PyGILState_Ensure();
	// Add config folder and application directory to sys.path
	QString addPathCode = QString("import sys\n"
		"for p in ['%1', '%2', '%3']:\n"
		"    if p and p not in sys.path:\n"
		"        sys.path.insert(0, p)\n")
		.arg(d_parent->d_python_config_folder)
		.arg(qApp->applicationDirPath())
		.arg(qApp->applicationDirPath() + "/../qtiplot");
	PyRun_SimpleString(addPathCode.toUtf8().constData());
	PyGILState_Release(state);

	// Redirect output to the print(const QString&) signal.
	// Also see method write(const QString&) and Python documentation on
	// sys.stdout and sys.stderr.
	setQObject(this, "stdout", sys);
	setQObject(this, "stderr", sys);

	bool initialized = loadInitFile(d_parent->d_python_config_folder + "/qtiplotrc");
	if(!initialized)
		initialized = loadInitFile(d_parent->d_python_config_folder + "/.qtiplotrc");
	if(!initialized)
		initialized = loadInitFile(qApp->applicationDirPath() + "/qtiplotrc");
	if(!initialized)
		initialized = loadInitFile(qApp->applicationDirPath() + "/../qtiplot/qtiplotrc");
	if(!initialized)
		initialized = loadInitFile(qApp->applicationDirPath() + "/../../qtiplot/qtiplotrc");

	if(!initialized){
		fprintf(stderr, "Couldn't find Python initialization files in %s\n", d_parent->d_python_config_folder.toUtf8().constData());
		if (!qApp->arguments().contains("-X")) {
			QMessageBox::critical(d_parent, tr("Couldn't find initialization files"),
			tr("Please indicate the correct path to the Python configuration files in the preferences dialog.\nSearched: ") + d_parent->d_python_config_folder + "/qtiplotrc");
		}
	}

	return initialized;
}

PythonScripting::~PythonScripting()
{
	PyGILState_STATE state = PyGILState_Ensure();
	Py_XDECREF(globals);
	Py_XDECREF(math);
	Py_XDECREF(sys);
	PyGILState_Release(state);
}

bool PythonScripting::loadInitFile(const QString &path)
{
	PyGILState_STATE state = PyGILState_Ensure();
	QString nativePath = QDir::toNativeSeparators(path);
	QFileInfo pyFile(nativePath+".py");
	bool success = false;
	
	if (pyFile.exists()) {
		QFile f(pyFile.filePath());
		if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QByteArray data = f.readAll();
			success = (PyRun_SimpleString(data.constData()) == 0);
			if (!success) {
				PyErr_Print();
			}
			f.close();
		}
	}
	PyGILState_Release(state);
	return success;
}

bool PythonScripting::isRunning() const
{
	PyGILState_STATE state = PyGILState_Ensure();
	bool isinit = Py_IsInitialized();
	PyGILState_Release(state);
	return isinit;
}

extern const sipAPIDef *sipAPI_qti;

bool PythonScripting::setQObject(QObject *val, const char *name, PyObject *dict)
{
	if(!val) return false;
	PyObject *pyobj=nullptr;

	PyGILState_STATE state = PyGILState_Ensure();
	if (sipAPI_qti && sipAPI_qti->api_find_type && sipAPI_qti->api_convert_from_type) {
		const auto klass = sipAPI_qti->api_find_type(val->metaObject()->className());
		if (klass) pyobj = sipAPI_qti->api_convert_from_type(val, klass, nullptr);
	}

	if (pyobj) {
		if (dict)
			PyDict_SetItemString(dict,name,pyobj);
		else
			PyDict_SetItemString(globals,name,pyobj);
		Py_DECREF(pyobj);
	}

	PyGILState_Release(state);
	return (bool) pyobj;
}

bool PythonScripting::setInt(int val, const char *name, PyObject *dict)
{
	PyGILState_STATE state = PyGILState_Ensure();
	PyObject *pyobj = Py_BuildValue("i",val);
	if (pyobj) {
		if (dict)
			PyDict_SetItemString(dict,name,pyobj);
		else
			PyDict_SetItemString(globals,name,pyobj);
		Py_DECREF(pyobj);
	}
	PyGILState_Release(state);
	return (bool) pyobj;
}

bool PythonScripting::setDouble(double val, const char *name, PyObject *dict)
{
	PyGILState_STATE state = PyGILState_Ensure();
	PyObject *pyobj = Py_BuildValue("d",val);
	if (pyobj) {
		if (dict)
			PyDict_SetItemString(dict,name,pyobj);
		else
			PyDict_SetItemString(globals,name,pyobj);
		Py_DECREF(pyobj);
	}
	PyGILState_Release(state);
	return (bool) pyobj;
}

const QStringList PythonScripting::mathFunctions() const
{
	PyGILState_STATE state = PyGILState_Ensure();
	QStringList flist;
	PyObject *key, *value;
	Py_ssize_t i=0;
	while(PyDict_Next(math, &i, &key, &value))
		if (PyCallable_Check(value))
			flist << PyString_AsString(key);
	PyGILState_Release(state);
	flist.sort();
	return flist;
}

const QString PythonScripting::mathFunctionDoc(const QString &name) const
{
	PyGILState_STATE state = PyGILState_Ensure();
	PyObject *mathf = PyDict_GetItemString(math, name.toUtf8().constData()); // borrowed
	QString qdocstr("");
	if (mathf) {
		PyObject *pydocstr = PyObject_GetAttrString(mathf, "__doc__"); // new
		qdocstr = QString::fromUtf8(PyUnicode_AsUTF8(pydocstr));
		Py_XDECREF(pydocstr);
	}
	PyGILState_Release(state);
	return qdocstr;
}

const QStringList PythonScripting::fileExtensions() const
{
	QStringList extensions;
	extensions << "py";
	return extensions;
}
