/* vim: set filetype=cpp : */
/* due to a limitation of sip, this file has to use Unix style line endings */
/***************************************************************************
	File                 : qti.sip
	Project              : QtiPlot
--------------------------------------------------------------------
	Copyright            : (C) 2006 by Knut Franke, Ion Vasilief, Michael Roemer, Jonas Bähr
	Email                : knut.franke*gmx.de, ion_vasilief*yahoo.fr,
						   roemer*nano.uni-hannover.de, Jonas.Baehr*web.de
	Description          : Specifications for Python bindings

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

%Module qti


%ModuleCode
#include <QObject>
#include <QWidget>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QLocale>
#include <QStringList>
#include <QPixmap>
#include <QImage>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QFont>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QPrinter>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_symbol.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_color_map.h>
#include <qwt_raster_data.h>
%End

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class ScriptingEnv;

class ScriptEdit: QTextEdit
{
%TypeHeaderCode
#include "ScriptEdit.h"
%End
public:
  ScriptEdit(ScriptingEnv *env, QWidget *parent=0, const QString& name = QString());
private:
  ScriptEdit(const ScriptEdit &);
  ScriptEdit& operator=(const ScriptEdit &);
public slots:
  void print();

  void exportPDF(const QString& fileName);
  QString save();
  QString exportASCII(const QString &file=QString())/PyName=saveAs/;
  QString importASCII(const QString &file=QString());


};

class MdiSubWindow: QMdiSubWindow /PyName=MDIWindow/
{
%TypeHeaderCode
#include "MdiSubWindow.h"
%End
public:
  MdiSubWindow(const QString& label = QString(), ApplicationWindow *app = 0, const QString& name = QString(), Qt::WindowFlags f = Qt::Widget);
  enum CaptionPolicy{Name = 0, Label = 1, Both = 2};

  SIP_PYOBJECT windowLabel();
%MethodCode
  sipRes = PyUnicode_FromString(sipCpp->windowLabel().toUtf8().constData());
%End

  CaptionPolicy captionPolicy();
  void setCaptionPolicy(CaptionPolicy);

  Folder* folder();

  void askOnCloseEvent(bool)/PyName=confirmClose/;


};

class Table: MdiSubWindow
{
%Docstring
The Table class handles spreadsheet data.
Note: Column and row indices start at 1.
%End
  Table(ScriptingEnv *env, int r,int c, const QString &label, ApplicationWindow* parent, const QString& name = QString(), Qt::WindowFlags f = Qt::Widget);
private:
    Table(const Table &);











%TypeHeaderCode
#include "Table.h"
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <datetime.h> // python include
#define CHECK_TABLE_COL(arg)\
    int col;\
    if (PyLong_Check(arg)) {\
      col = (int)PyLong_AsLong(arg) - 1;\
		if (col < 0 || col >= sipCpp->numCols()) {\
			sipIsErr = 1;\
			PyErr_Format(PyExc_ValueError, "There's no column %d in table %s!", col+1, sipCpp->name().toUtf8().constData());\
		}\
	 } else {\
      PyObject *tmp = PyObject_Str(arg);\
		if (!tmp) {\
			sipIsErr = 1;\
			PyErr_Format(PyExc_TypeError, "Column argument must be either int or string.");\
		} else {\
			col = sipCpp->colIndex(PyUnicode_AsUTF8(tmp));\
			if (col < 0) {\
				sipIsErr = 1;\
				PyErr_Format(PyExc_ValueError, "There's no column named %s in table %s!", PyUnicode_AsUTF8(tmp),\
						sipCpp->name().toUtf8().constData());\
				Py_DECREF(tmp);\
			}\
		}\
	 }

#define CHECK_TABLE_ROW(arg)\
   int row = arg-1;\
	if (row < 0 || row >= sipCpp->numRows()) {\
		sipIsErr = 1;\
		PyErr_Format(PyExc_ValueError, "There's no row %d in table %s!", row+1, sipCpp->name().toUtf8().constData());\
	}

static int setCellDataHelper(Table* table, int row, int col, PyObject* item) {
  PyTypeObject* item_type = item->ob_type;
  if (item == Py_None) {
    table->clearCell(row, col);
    return 0;
  }
  switch (table->columnType(col)) {
  case Table::Numeric:
    item = PyNumber_Float(item);
    if (item) {
      table->setCell(row, col, PyFloat_AsDouble(item));
      Py_DECREF(item);
    }
    else {
      if (PyType_Ready(item_type) == 0)
        PyErr_Format(PyExc_TypeError, "Can't convert <%s> into a float!", item_type->tp_name);
      return 1;
    }
    break;
  case Table::Text:
    item = PyObject_Str(item);
    if (item) {
      table->setText(row, col, PyUnicode_AsUTF8(item));
      Py_DECREF(item);
    }
    else {
      if (PyType_Ready(item_type) == 0)
        PyErr_Format(PyExc_TypeError, "Can't convert <%s> into a string!", item_type->tp_name);
      return 1;
    }
    break;
  case Table::Date:
    if (sipCanConvertToType(item, sipFindType("QDateTime"), 0)) {
      int isErr = 0;
      QDateTime* dateTime = reinterpret_cast<QDateTime*>(sipConvertToType(item, sipFindType("QDateTime"),
                            0/*sipTransferObj*/, 0/*flags*/, 0/*state*/, &isErr));
      if (isErr)
        return 1;
      table->setText(row, col, dateTime->toString(table->columnFormat(col)));
      //delete dateTime;
    }
    else {
      if (PyType_Ready(item_type) == 0)
        PyErr_Format(PyExc_TypeError, "Can't convert <%s> into a datetime!", item_type->tp_name);
      return 1;
    }
    break;
  case Table::Time:
    if (sipCanConvertToType(item, sipFindType("QTime"), 0)) {
      int isErr = 0;
      QTime* time = reinterpret_cast<QTime*>(sipConvertToType(item, sipFindType("QTime"),
                            0/*sipTransferObj*/, 0/*flags*/, 0/*state*/, &isErr));
      if (isErr)
        return 1;
      table->setText(row, col, time->toString(table->columnFormat(col)));
      //delete time;
    }
    else {
      if (PyType_Ready(item_type) == 0)
        PyErr_Format(PyExc_TypeError, "Can't convert <%s> into a time!", item_type->tp_name);
      return 1;
    }
    break;
  case Table::Month:
    item = PyNumber_Long(item);
    if (item) {
      int m = PyLong_AsLong(item) % 12; // no type checking required since we casted above
      if (!m)
        m = 12;
      QString format = table->columnFormat(col);
      Py_DECREF(item);
      if (format == "M")
          table->setText(row, col, QLocale().monthName(m, QLocale::ShortFormat).left(1));
      else if (format == "MMM")
          table->setText(row, col, QLocale().monthName(m, QLocale::ShortFormat));
      else if (format == "MMMM")
          table->setText(row, col, QLocale().monthName(m, QLocale::LongFormat));
    }
    else {
      if (PyType_Ready(item_type) == 0)
        PyErr_Format(PyExc_TypeError, "Can't convert <%s> into a number!", item_type->tp_name);
      return 1;
    }
    break;
  case Table::Day:
    item = PyNumber_Long(item);
    if (item) {
      int day = PyLong_AsLong(item) % 7; // no type checking required since we casted above
      if (!day)
        day = 7;
      QString format = table->columnFormat(col);
      Py_DECREF(item);
      if (format == "d")
          table->setText(row, col, QLocale().dayName(day, QLocale::ShortFormat).left(1));
      else if (format == "ddd")
          table->setText(row, col, QLocale().dayName(day, QLocale::ShortFormat));
      else if (format == "dddd")
          table->setText(row, col, QLocale().dayName(day, QLocale::LongFormat));
    }
    else {
      if (PyType_Ready(item_type) == 0)
        PyErr_Format(PyExc_TypeError, "Can't convert <%s> into a number!", item_type->tp_name);
      return 1;
    }
    break;
  default:
    PyErr_Format(PyExc_ValueError, "Type %d of column %d in table %s not supported!",
                 table->columnType(col), col+1, table->name().toUtf8().constData());
    return 1;
    break;
  } // switch column type
  return 0; // everything ok
}

static int cellDataHelper(Table* table, int row, int col, PyObject** item) {
  switch (table->columnType(col)) {
  case Table::Numeric:
    if (table->text(row, col).isEmpty())
      *item = Py_None;
    else
      *item = PyFloat_FromDouble(table->cell(row, col));
    break;
  case Table::Text:
    *item = PyUnicode_FromString(table->text(row, col).toUtf8().constData());
    break;
  case Table::Date:
    if (table->text(row, col).isEmpty())
      *item = Py_None;
    else {
      if (!PyDateTimeAPI)
        PyDateTime_IMPORT;
      QDateTime dateTime = QDateTime::fromString(table->text(row, col), table->columnFormat(col));
      QDate date = dateTime.date();
      QTime time = dateTime.time();
      *item = PyDateTime_FromDateAndTime(date.year(), date.month(), date.day(),
                                         time.hour(), time.minute(), time.second(), time.msec()*1000);
    }
    break;
  case Table::Time:
    if (table->text(row, col).isEmpty())
      *item = Py_None;
    else {
      if (!PyDateTimeAPI)
        PyDateTime_IMPORT;
      QTime time = QTime::fromString(table->text(row, col), table->columnFormat(col));
      *item = PyTime_FromTime(time.hour(), time.minute(), time.second(), time.msec()*1000);
    }
    break;
  case Table::Month:
    if (table->text(row, col).isEmpty())
      *item = Py_None;
    else {
      QDate date = QDate::fromString(table->text(row, col), table->columnFormat(col));
      *item = PyLong_FromLong(date.month());
    }
    break;
  case Table::Day:
    if (table->text(row, col).isEmpty())
      *item = Py_None;
    else {
      QDate date = QDate::fromString(table->text(row, col), table->columnFormat(col));
      *item = PyLong_FromLong(date.dayOfWeek());
    }
    break;
  default:
    *item = NULL;
    PyErr_Format(PyExc_ValueError, "Type %d of column %d in table %s not supported!",
                 table->columnType(col), col+1, table->name().toUtf8().constData());
    return 1;
    break;
  } // switch column type
  return 0; // everything ok
}
%End

public:
  enum PlotDesignation{None = 0, X = 1, Y = 2, Z = 3, xErr = 4, yErr = 5, Label = 6};
  enum ImportMode{NewColumns, NewRows, Overwrite};
  enum NumericFormat{Default = 0, Decimal = 1, Scientific = 2};

  int numRows();
%Docstring
Returns the number of rows in the table.
%End
  Py_ssize_t __len__();
%MethodCode
  sipRes = sipCpp->numRows();
%End
  int numCols();
%Docstring
Returns the number of columns in the table.
%End
  void setNumRows(int);
%Docstring
Sets the number of rows in the table.
%End
  void setNumCols(int);
%Docstring
Sets the number of columns in the table.
%End

  void deleteRows(int, int);
  void __delitem__(int);
%MethodCode
  int len = sipCpp->numRows();
  if ((a0 = (int)sipConvertFromSequenceIndex(a0, len)) < 0)
    sipIsErr = 1;
  else
    sipCpp->deleteRows(a0 + 1, a0 + 1); // start and stop are equal, counting starts at 1
%End
  void __delitem__(SIP_PYSLICE);
%MethodCode
  Py_ssize_t len, start, stop, step, slicelength;
  len = sipCpp->numRows();
  if (PySlice_Unpack((PyObject *)a0, &start, &stop, &step) < 0)
    sipIsErr = 1;
  else {
    slicelength = (stop - start + step - 1) / step;
    start += 1; // row index starts at 1 (python at 0); the stop is inclusive (python exclusive) so no need to change
    if (step == 1)
      sipCpp->deleteRows(start, stop);
    else {
      for (int i = 0; i < slicelength; i++) {
        sipCpp->deleteRows(start, start);
        start += step - 1; // the "- 1" is needed since the removal also shifts the index
      }
    }
  }
%End

  SIP_PYOBJECT text(SIP_PYOBJECT, int);
%Docstring
text(col, row) -> string
Returns the string value at the specified column and row.
%End
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
  if (sipIsErr == 0) {
		sipRes = PyUnicode_FromString(sipCpp->text(row, col).toUtf8().constData());
		if (!sipRes) {
			sipIsErr = 1;
		}
	}
%End

  void undo();
%Docstring
undo()
Undoes the last operation on this table.
%End
%MethodCode
  if (sipCpp->undoStack())
    sipCpp->undoStack()->undo();
%End

  void redo();
%Docstring
redo()
Redoes the last undone operation on this table.
%End
%MethodCode
  if (sipCpp->undoStack())
    sipCpp->undoStack()->redo();
%End

    void removeCol(SIP_PYOBJECT);
%MethodCode
    sipIsErr=0;
    CHECK_TABLE_COL(a0);
	if (sipIsErr == 0){
        QStringList list;
		list << sipCpp->colName(col);
        sipCpp->deleteColumns(list);
    }
%End

  double cell(SIP_PYOBJECT, int);
%Docstring
cell(col, row) -> float
Returns the numeric value at the specified column and row.
%End
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
	if (sipIsErr == 0){
		if (sipCpp->text(row, col) != "")
			sipRes = sipCpp->cell(row, col);
		else {
			sipRes = NULL;
			PyErr_SetString(PyExc_ValueError, "Empty table cell");
		}
	}
%End
  SIP_PYOBJECT cellData(SIP_PYOBJECT, int);
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
	if (sipIsErr == 0)
    sipIsErr = cellDataHelper(sipCpp, row, col, &sipRes);
%End
  SIP_PYTUPLE rowData(int);
%MethodCode
  PyObject *item;
  sipIsErr = 0;
  CHECK_TABLE_ROW(a0);
	if (sipIsErr == 0){
    int size = sipCpp->numCols();
    sipRes = PyTuple_New(size);
    for(int col = 0; col < size; col++) {
      sipIsErr = cellDataHelper(sipCpp, row, col, &item);
      if (sipIsErr == 0)
        PyTuple_SET_ITEM(sipRes, col, item);
      else {
        Py_DECREF(sipRes);
        sipRes = NULL;
        break;
      }
    } // for each col
	} // if not sip error
%End
  SIP_PYTUPLE __getitem__(int); // this is operator[]
%MethodCode
  Py_ssize_t idx = sipConvertFromSequenceIndex(a0, sipCpp->numRows());
  if (idx < 0)
    sipIsErr = 1;
  else {
    sipRes = PyObject_CallMethod(sipSelf, "rowData", "i", idx + 1); // rowData's index starts at 1
    if (sipRes == NULL)
      sipIsErr = 1;
  }
%End
  SIP_PYTUPLE __getitem__(SIP_PYSLICE); // this is operator[]
%MethodCode
  Py_ssize_t len, start, stop, step, slicelength;
  len = sipCpp->numRows();
  if (PySlice_Unpack((PyObject *)a0, &start, &stop, &step) < 0)
    sipIsErr = 1;
  else {
    slicelength = (stop - start + step - 1) / step;
    PyObject *item;
    sipRes = PyList_New(slicelength);
    for(int i = 0; i < slicelength; i++) {
      item = PyObject_CallMethod(sipSelf, "rowData", "i", start + 1); // rowData's index starts at 1
      start += step;
      if (item)
        PyList_SET_ITEM(sipRes, i, item);
      else {
        Py_DECREF(sipRes);
        sipRes = NULL;
        sipIsErr = 1;
        break;
      }
    } // for
  }
%End
  SIP_PYLIST colData(SIP_PYOBJECT);
%MethodCode
  PyObject *item;
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
	if (sipIsErr == 0){
    int size = sipCpp->numRows();
    sipRes = PyList_New(size);
    for(int row = 0; row < size; row++) {
      sipIsErr = cellDataHelper(sipCpp, row, col, &item);
      if (sipIsErr == 0)
        PyList_SET_ITEM(sipRes, row, item);
      else {
        Py_DECREF(sipRes);
        sipRes = NULL;
        break;
      }
    } // for
	} // if not sip error
%End
  void setRowData(int, SIP_PYTUPLE);
%MethodCode
  PyObject *item;
  sipIsErr = 0;
  CHECK_TABLE_ROW(a0);
  int n = PyTuple_Size(a1);
  int size = sipCpp->numCols();
  if (n != size) {
    sipIsErr = 1;
    PyErr_SetString(PyExc_ValueError, "The argument length must match the table's column count!");
  }
	if (sipIsErr == 0){
    for(int col = 0; col < size; col++) {
      item = PyTuple_GET_ITEM(a1, col);
      sipIsErr = setCellDataHelper(sipCpp, row, col, item);
      if (sipIsErr)
        break;
    } // for each col
	} // if not sip error
%End
  void __setitem__(int, SIP_PYTUPLE);
%MethodCode
  Py_ssize_t idx = sipConvertFromSequenceIndex(a0, sipCpp->numRows());
  if (idx < 0)
    sipIsErr = 1;
  else {
    PyObject* methodName = PyUnicode_FromString("setRowData");
    PyObject* rowNumber = PyLong_FromLong(idx + 1); // setRowData's index starts at 1
    PyObject* ret = PyObject_CallMethodObjArgs(sipSelf, methodName, rowNumber, a1, NULL);
    if (ret == NULL)
      sipIsErr = 1;
    else
      Py_DECREF(ret);
    Py_DECREF(rowNumber);
    Py_DECREF(methodName);
  }
%End
  int appendRowData(SIP_PYTUPLE);
%MethodCode
  sipRes = sipCpp->numRows() + 1;
  sipCpp->resizeRows(sipRes);
  PyObject* methodName = PyUnicode_FromString("setRowData");
  PyObject* rowNumber = PyLong_FromLong(sipRes);
  PyObject* ret = PyObject_CallMethodObjArgs(sipSelf, methodName, rowNumber, a0, NULL);
  if (ret == NULL)
    sipIsErr = 1;
  else
    Py_DECREF(ret);
  Py_DECREF(rowNumber);
  Py_DECREF(methodName);
%End
  void setColData(SIP_PYOBJECT, SIP_PYOBJECT, int=0); // a2 can be used as an offset
%MethodCode
  PyObject *item;
  PyObject *iterator = PyObject_GetIter(a1);
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  int tableLength = sipCpp->numRows();
  if (iterator == NULL) {
    sipIsErr = 1;
    PyErr_SetString(PyExc_TypeError, "Object needs to be iterable");
  }
  else {
    while (a2 < 0) { // a negative offset should skip the first values in the iterator
      if (item = PyIter_Next(iterator))
        Py_DECREF(item); // not needed any more
      else {
        sipIsErr = 1;
        if (! PyErr_Occurred())
          PyErr_SetString(PyExc_StopIteration, "Iterator ended before any row could be filled!");
      }
      a2++;
    } //while
  }
	if (sipIsErr == 0){
    for(int row = a2; row < tableLength; row++) {
      if (item = PyIter_Next(iterator)) {
        sipIsErr = setCellDataHelper(sipCpp, row, col, item);
        Py_DECREF(item);
      }
      else {
        sipIsErr = 1;
        if (! PyErr_Occurred())
          PyErr_SetString(PyExc_StopIteration, "Iterator ended before all rows were filled!");
      }
      if (sipIsErr)
        break;
    } // for each row
	} // if not sip error
  if (iterator)
    Py_DECREF(iterator);
%End
  void setText(SIP_PYOBJECT, int, const QString&);
%Docstring
setText(col, row, value)
Sets the string value at the specified column and row.
%End
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
	if (sipIsErr == 0)
		sipCpp->setText(row, col, *a2);
%End
  void setCell(SIP_PYOBJECT, int, double);
%Docstring
setCell(col, row, value)
Sets the numeric value at the specified column and row.
%End
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
	if (sipIsErr == 0)
		sipCpp->setCell(row, col, a2);
%End
  void setCellData(SIP_PYOBJECT, int, SIP_PYOBJECT);
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
	if (sipIsErr == 0)
		sipIsErr = setCellDataHelper(sipCpp, row, col, a2);
%End
  SIP_PYOBJECT colName(int);
%Docstring
colName(col) -> string
Returns the name of the specified column.
%End
%MethodCode
   if (a0 < 1 || a0 > sipCpp->numCols()) {\
		sipIsErr = 1;\
		PyErr_SetString(PyExc_ValueError, "Invalid column argument");\
	} else
		sipRes = PyUnicode_FromString(sipCpp->colLabel(a0-1).toUtf8().constData());
%End
  SIP_PYTUPLE colNames();
%MethodCode
    QStringList l = sipCpp->colNames();
    sipRes = PyTuple_New(l.count());
    if (sipRes) {
      for (int i=0; i<l.count(); i++)
        PyTuple_SET_ITEM(sipRes, i, PyUnicode_FromString(l.at(i).toUtf8().constData()));
    } else
      sipIsErr = 1;
%End
  void setColName(SIP_PYOBJECT, const QString&, bool=false);
%Docstring
setColName(col, name)
Renames the specified column.
%End
%MethodCode
	sipIsErr = 0;
   CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setColName(col, *a1, a2);
%End
  void setHeader(QStringList header)/PyName=setColNames/;
  void notifyChanges();

  void importASCII(const QString&, const QString&="\t", int=0, bool=false,
  					bool=true, bool=false, bool=false, const QString&="#", bool=false,
					ImportMode = Overwrite, const QLocale& = QLocale(), int = 0, int = -1, const QList<int>& = QList<int>(), const QStringList& = QStringList());
%Docstring
importASCII(file, sep="\t", ignoreLines=0, renameCols=True)
Imports data from an ASCII file.
%End
  bool exportASCII(const QString&, const QString&="\t", bool=false, bool=false, bool=false);
%Docstring
exportASCII(file, sep="\t")
Exports the table data to an ASCII file.
%End

  void setDecimalSeparators(int,bool=true);
%MethodCode
	QLocale locale;
    switch (a0){
        case 0:
            locale = QLocale::system();
        break;
        case 1:
            locale = QLocale::c();
        break;
        case 2:
            locale = QLocale(QLocale::German);
        break;
        case 3:
            locale = QLocale(QLocale::French);
        break;
    }
 	if (a1==true)
		locale.setNumberOptions(QLocale::OmitGroupSeparator);

    sipCpp->updateDecimalSeparators(locale);
%End

  void normalizeCol(SIP_PYOBJECT) /PyName=normalize/;
%Docstring
normalize(col)
Normalizes the specified column data.
%End
%MethodCode
	sipIsErr = 0;
	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->normalizeCol(col);
%End
  void normalize();

  void sortColumn(SIP_PYOBJECT, int order = 0);
%Docstring
sortColumn(col, order=0)
Sorts the specified column. 0 = Ascending, 1 = Descending.
%End
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->sortColumn(col, a1);
%End
  void sort(int type = 0, int order = 0, const QString& leadCol = QString());
  void sortColumns(SIP_PYTUPLE, int=0, int=0, const QString&=QString());
%MethodCode
  QStringList l;
  int n = PyTuple_Size(a0);
  for (int i=0; i<n; i++) {
    PyObject *str = PyObject_Str(PyTuple_GET_ITEM(a0,i));
    if (str) {
      l << PyUnicode_AsUTF8(str);
		Py_DECREF(str);
	 } else {
      sipIsErr = 1;
      break;
    }
  }
  sipCpp->sortColumns(l, a1, a2, *a3);
%End

void setRandomValues(SIP_PYOBJECT, int startRow = 1, int endRow = -1);
%MethodCode
sipIsErr = 0;
CHECK_TABLE_COL(a0);
if (sipIsErr == 0)
	sipCpp->setRandomValues(col, a1 - 1, a2 - 1);
%End

void setNormalRandomValues(SIP_PYOBJECT, int startRow = 1, int endRow = -1, double sd = 1.0);
%MethodCode
sipIsErr = 0;
CHECK_TABLE_COL(a0);
if (sipIsErr == 0)
	sipCpp->setNormalRandomValues(col, a1 - 1, a2 - 1, a3);
%End

	void setCommand(SIP_PYOBJECT, const QString&);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setCommand(col, *a1);
%End
  bool calculate(SIP_PYOBJECT col, int startRow = 1, int endRow = -1, bool forceMuParser = false, bool notifyChanges = true)/PyName=recalculate/;
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  if (a2 == -1)
    a2 = sipCpp->numRows();
  if (sipIsErr == 0)
    sipCpp->calculate(col, a1 - 1, a2 - 1, a3, a4); // row indes starts at 0
%End

	QString comment(SIP_PYOBJECT);
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  if (sipIsErr == 0)
    return PyUnicode_FromString(sipCpp->comment(col).toUtf8().constData());
%End

	void setComment(SIP_PYOBJECT, const QString&);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setColComment(col, *a1);
%End

	void showComments(bool on = true);

    void setReadOnlyColumn(SIP_PYOBJECT, bool = true);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setReadOnlyColumn(col, a1);
%End

	int columnRole(SIP_PYOBJECT);
%MethodCode
	sipIsErr = 0;
	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		return Py_BuildValue("h", sipCpp->colPlotDesignation(col));
%End

    void setColumnRole(SIP_PYOBJECT, PlotDesignation);
%Docstring
setColumnRole(col, role)
Defines the plot role of the specified column.
Roles: Table.X, Table.Y, Table.Z, Table.xErr, Table.yErr, Table.Label, Table.None.
%End
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0){
		sipCpp->setColPlotDesignation(col, a1);
		sipCpp->setHeaderColType();
	}
%End

	void setColTextFormat(SIP_PYOBJECT);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setTextFormat(col);
%End

	void setColNumericFormat(SIP_PYOBJECT, int, int,  bool = true);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setColNumericFormat(a1, a2, col, a3);
%End

	void setColDateFormat(SIP_PYOBJECT, const QString&,  bool = true);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setDateFormat(*a1, col, a2);
%End

	void setColTimeFormat(SIP_PYOBJECT, const QString&,  bool = true);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setTimeFormat(*a1, col, a2);
%End

	void setColMonthFormat(SIP_PYOBJECT, const QString&,  bool = true);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setMonthFormat(*a1, col, a2);
%End

	void setColDayFormat(SIP_PYOBJECT, const QString&,  bool = true);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setDayFormat(*a1, col, a2);
%End

	void setColumnWidth(SIP_PYOBJECT, int);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setColumnWidth(col, a1);
%End

	void showAllColumns();
	void hideColumn(SIP_PYOBJECT, bool = true);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->hideColumn(col, a1);
%End

	bool isRowSelected(int, bool = false);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_ROW(a0);
	if (sipIsErr == 0)
		return Py_BuildValue("b",sipCpp->isRowSelected(row, a1));

%End

	bool isColumnSelected(SIP_PYOBJECT, bool = false)/PyName=isColSelected/;
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		return Py_BuildValue("b",sipCpp->isColumnSelected(col, a1));
%End

    int firstSelectedColumn();
	int numSelectedRows();

	void setSelectedCol(SIP_PYOBJECT);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->setSelectedCol(col);
%End

	int selectedColumn();

	void addCol(PlotDesignation pd = Y)/PyName=addColumn/;
  int addRow();
%MethodCode
  sipRes = sipCpp->numRows() + 1;
  sipCpp->resizeRows(sipRes);
%End
	void insertColumns(SIP_PYOBJECT, int);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);
	if (sipIsErr == 0)
		sipCpp->insertCols(col, a1);
%End

	void swapColumns(SIP_PYOBJECT, SIP_PYOBJECT);
%MethodCode
  	sipIsErr = 0;
  	CHECK_TABLE_COL(a0);

  	int col2;
    if (PyLong_Check(a1)) {
      col2 = (int)PyLong_AsLong(a1) - 1;
		if (col2 < 0 || col2 >= sipCpp->numCols()) {
			sipIsErr = 1;
			PyErr_Format(PyExc_ValueError, "There's no column %d in table %s!", col2+1, sipCpp->name().toUtf8().constData());
		}
	 } else {
      PyObject *tmp = PyObject_Str(a1);
		if (!tmp) {
			sipIsErr = 1;
			PyErr_Format(PyExc_TypeError, "Column argument must be either int or string.");
		} else {
			    col2 = sipCpp->colNames().indexOf(PyUnicode_AsUTF8(tmp));
			if (col2 < 0) {
				sipIsErr = 1;
				PyErr_Format(PyExc_ValueError, "There's no column named %s in table %s!", PyUnicode_AsUTF8(tmp),sipCpp->name().toUtf8().constData());
				Py_DECREF(tmp);
			}
		}
	 }

	if (sipIsErr == 0)
		sipCpp->swapColumns(col, col2);
%End

void scrollToCell(SIP_PYOBJECT, int);
%MethodCode
  sipIsErr = 0;
  CHECK_TABLE_COL(a0);
  CHECK_TABLE_ROW(a1);
  if (sipIsErr == 0)
    sipCpp->table()->ensureCellVisible(row, col); // for some reason these indices seem to start at 1
%End


};

class QwtLinearColorMap
{
private:
    QwtLinearColorMap(const QwtLinearColorMap &);











%TypeHeaderCode
#include <qwt_color_map.h>
%End
public:

  enum Format{RGB, Indexed};
  enum Mode {FixedColors, ScaledColors};
  QwtLinearColorMap(const QColor &, const QColor &, QwtLinearColorMap::Format=QwtColorMap::RGB);

  void addColorStop (double, const QColor &);
  void setMode (Mode);

  void  setColorInterval (const QColor &, const QColor &);
  QColor color1() const;
  QColor color2() const;


};

class LinearColorMap : QwtLinearColorMap
{
%TypeHeaderCode
#include <LinearColorMap.h>
%End
public:

  LinearColorMap();
  LinearColorMap(const QColor &, const QColor &);
  //! Set the intensity range
  void setIntensityRange(double vmin, double vmax);
  //! Get the lower range limit
  double lowerBound();
  //! Get the upper range limit
  double upperBound();


};

class Matrix: MdiSubWindow
{
%Docstring
The Matrix class handles 2D matrices of numeric data.
Note: Row and column indices start at 1.
%End
private:
    Matrix(const Matrix &);











%TypeHeaderCode
#include "Matrix.h"
#define CHECK_MATRIX_COL(arg)\
	int col = arg-1;\
   if (col < 0 || col >= sipCpp->numCols()) {\
		sipIsErr = 1;\
		PyErr_Format(PyExc_ValueError, "There's no column %d in matrix %s!", col+1, sipCpp->name().toUtf8().constData());\
	}
#define CHECK_MATRIX_ROW(arg)\
   int row = arg-1;\
	if (row < 0 || row >= sipCpp->numRows()) {\
		sipIsErr = 1;\
		PyErr_Format(PyExc_ValueError, "There's no row %d in matrix %s!", row+1, sipCpp->name().toUtf8().constData());\
	}
%End
public:
  Matrix(ScriptingEnv *env, int r, int c, const QString& label, ApplicationWindow* parent, const QString& name = QString(), Qt::WindowFlags f = Qt::Widget);
  Matrix(ScriptingEnv *env, const QImage& image, const QString& label, ApplicationWindow* parent, const QString& name = QString(), Qt::WindowFlags f = Qt::Widget);
  enum HeaderViewType{ColumnRow, XY};
  enum ViewType{TableView, ImageView};
  enum ImportMode{NewColumns, NewRows, Overwrite};
  enum ResamplingMethod{Bilinear, Bicubic};

  int numRows();
%Docstring
Returns the number of rows in the matrix.
%End
  void setNumRows(int);
%Docstring
Sets the number of rows in the matrix.
%End
  int numCols();
%Docstring
Returns the number of columns in the matrix.
%End
  void setNumCols(int);
%Docstring
Sets the number of columns in the matrix.
%End
  void setDimensions(int rows, int cols);
%Docstring
setDimensions(rows, cols)
Resizes the matrix to the specified number of rows and columns.
%End
  void resample(int rows, int cols, const ResamplingMethod& method = Matrix::Bilinear);
  void smooth();

  void undo();
%Docstring
undo()
Undoes the last operation on this matrix.
%End
%MethodCode
  if (sipCpp->undoStack())
    sipCpp->undoStack()->undo();
%End

  void redo();
%Docstring
redo()
Redoes the last undone operation on this matrix.
%End
%MethodCode
  if (sipCpp->undoStack())
    sipCpp->undoStack()->redo();
%End

  SIP_PYOBJECT text(int, int);
%Docstring
text(row, col) -> string
Returns the value at the specified row and column as a string.
%End
%MethodCode
	sipIsErr = 0;
	CHECK_MATRIX_ROW(a0);
	CHECK_MATRIX_COL(a1);
	if (sipIsErr == 0)
		sipRes = PyUnicode_FromString(sipCpp->text(row, col).toUtf8().constData());
%End
  double cell(int, int);
%Docstring
cell(row, col) -> float
Returns the numeric value at the specified row and column.
%End
%MethodCode
	sipIsErr = 0;
	CHECK_MATRIX_ROW(a0);
	CHECK_MATRIX_COL(a1);
	if (sipIsErr == 0)
		sipRes = sipCpp->cell(row, col);
%End
  void setText(int, int, const QString&);
%Docstring
setText(row, col, value)
Sets the string value at the specified row and column.
%End
%MethodCode
	sipIsErr = 0;
	CHECK_MATRIX_ROW(a0);
	CHECK_MATRIX_COL(a1);
	if (sipIsErr == 0)
		sipCpp->setText(row, col, *a2);
%End
  void setCell(int, int, double);
%Docstring
setCell(row, col, value)
Sets the numeric value at the specified row and column.
%End
%MethodCode
	sipIsErr = 0;
	CHECK_MATRIX_ROW(a0);
	CHECK_MATRIX_COL(a1);
	if (sipIsErr == 0)
		sipCpp->setCell(row, col, a2);
%End

	double dx();
	double dy();
    double xStart();
	double xEnd();
	double yStart();
	double yEnd();
    void setCoordinates(double xs, double xe, double ys, double ye);

	QString xLabel();

	QString yLabel();

	QString zLabel();

	QString xUnit();

	QString yUnit();

	QString zUnit();

	QString xComment();

	QString yComment();

	QString zComment();

	QString formula();
%Docstring
Returns the formula used to calculate matrix values.
%End
	void setFormula(const QString &);
%Docstring
setFormula(formula)
Sets the formula used to calculate matrix values.
%End


    bool calculate(int startRow = 0, int endRow = -1, int startCol = 0, int endCol = -1, bool muParser = true);
%Docstring
calculate(startRow=0, endRow=-1, startCol=0, endCol=-1, muParser=True)
Calculates matrix values using the current formula.
%End


    void setNumericPrecision(int prec);

    void transpose();
%Docstring
Transposes the matrix.
%End
	void invert();
%Docstring
Inverts the matrix.
%End
	void flipVertically();
%Docstring
Flips the matrix vertically.
%End
	void flipHorizontally();
%Docstring
Flips the matrix horizontally.
%End
	void rotate90(bool = true);
%Docstring
rotate90(clockwise=True)
Rotates the matrix by 90 degrees.
%End
	double determinant();
%Docstring
Returns the determinant of the matrix.
%End
	double integrate();
%Docstring
Returns the integral of the matrix data.
%End

	void setViewType(ViewType, bool renderImage = true);
	void setHeaderViewType(HeaderViewType);

	void setDefaultColorMap();
	void setGrayScale();
	void setRainbowColorMap();
	LinearColorMap *colorMapPointer()/PyName=colorMap/;

	void resetView();

	QImage image();

	void exportRasterImage(const QString&, int = 100, int dpi = 0, int compression = 0);
	void exportToFile(const QString&)  /PyName=export/;
	void exportVector(const QString&, int = 0, bool = true);

	void importASCII(const QString&, const QString&="\t", int=0, bool=false, bool=false,
			const QString&="#", ImportMode = Overwrite, const QLocale& = QLocale(), int = 0, int = -1);
%Docstring
importASCII(file, sep="\t", ignoreLines=0, stripWhiteSpaces=False, simplifyWhiteSpaces=False)
Imports data from an ASCII file into the matrix.
%End
	bool exportASCII(const QString&, const QString&="\t", bool=false);
%Docstring
exportASCII(file, sep="\t", columnNames=False)
Exports matrix data to an ASCII file.
%End


};

class ArrowMarker // : QwtPlotMarker
{









%TypeHeaderCode
#include "ArrowMarker.h"
%End
public:

  ArrowMarker();
private:
  ArrowMarker(const ArrowMarker&);

  void setStartPoint(double, double) /PyName=setStart/;
  void setEndPoint(double, double) /PyName=setEnd/;

  void setStyle(Qt::PenStyle);

  void setWidth(double);
  void drawStartArrow(bool = true);
  void drawEndArrow(bool = true);
  void setHeadLength(int);
  void setHeadAngle(int);
  void fillArrowHead(bool = true);


};

class FrameWidget : QWidget  /PyName=Frame/
{
%TypeHeaderCode
#include "FrameWidget.h"
%End

public:
  FrameWidget(Graph *);
  enum FrameStyle{None = 0, Line = 1, Shadow = 2};
    enum Unit{Inch = 0, Millimeter, Centimeter, Point, Pixel, Scale};

    double xValue();
	double yValue();
	void setOriginCoord(double, double);
	void setOrigin(int x, int y);

	QRectF boundingRect();
	void setCoordinates(double left, double top, double right, double bottom);
    void setSize(int w, int h);
    void setSize(const QSize& newSize);
	void setRect(int x, int y, int w, int h);

	double right();
	double bottom();

	int frameStyle();
	void setFrameStyle(int);

	QPen framePen();

	Qt::PenStyle frameLineStyle();
	void setFrameLineStyle(const Qt::PenStyle&);

	QColor frameColor();

	double frameWidth();
	void setFrameWidth(double);

	QColor backgroundColor();

	QBrush brush();
	void setBrush(const QBrush& );


};

class ImageWidget : FrameWidget /PyName=Image/
{
%TypeHeaderCode
#include "ImageWidget.h"
%End
public:

  ImageWidget(Graph *, const QString&);
  QString fileName();
  bool load(const QString& fn);


};

class LegendWidget : FrameWidget /PyName=Legend/
{
%TypeHeaderCode
#include "LegendWidget.h"
%End
public:
  LegendWidget(Graph *);
  void setAngle(int);
  int angle();

  bool isAutoUpdateEnabled();
  void setAutoUpdate(bool);


};

class RectangleWidget : FrameWidget /PyName=Rectangle/
{
%TypeHeaderCode
#include "RectangleWidget.h"
%End
public:

  RectangleWidget(Graph *);


};

class EllipseWidget : FrameWidget /PyName=Ellipse/
{
%TypeHeaderCode
#include "EllipseWidget.h"
%End
public:

  EllipseWidget(Graph *);


};

class QwtSymbol /PyName = PlotSymbol/
{
private:
  QwtSymbol(const QwtSymbol &);
  QwtSymbol& operator=(const QwtSymbol &);









%TypeHeaderCode
#include <qwt_symbol.h>
%End
	public:
	enum Style
	{
		NoSymbol = -1,
		Ellipse,
		Rect,
		Diamond,
		Triangle,
		DTriangle,
		UTriangle,
		LTriangle,
		RTriangle,
		Cross,
		XCross,
		HLine,
		VLine,
		Star1,
		Star2,
		Hexagon,

	};

	QwtSymbol();
	QwtSymbol(Style st, const QBrush &bd, const QPen &pn, const QSize &s);

	void setSize(const QSize &s);
	void setSize(int a, int b = -1);
	void setBrush(const QBrush& b);
	void setPen(const QPen &p);
	void setStyle (Style s);

	const QBrush& brush() const;
	const QPen& pen() const;
	const QSize& size() const;
	Style style() const;

	
};

class ImageSymbol: QwtSymbol
{
%TypeHeaderCode
#include <ImageSymbol.h>
%End

public:
	ImageSymbol(const QString& fileName);
	ImageSymbol(const QPixmap& pixmap, const QString& fileName = QString());

	QPixmap pixmap();
	QString imagePath();
           
private:
	ImageSymbol(const ImageSymbol&);
	ImageSymbol& operator=(const ImageSymbol&);


};

class QwtPlotCurve
{
private:
    QwtPlotCurve(const QwtPlotCurve &);











%TypeHeaderCode
#include <qwt_plot_curve.h>
%End
	public:
		int dataSize() const;
		double x(int i) const;
%MethodCode
  sipRes = sipCpp->sample(a0).x();
%End
		double y(int i) const;
%MethodCode
  sipRes = sipCpp->sample(a0).y();
%End
		double minXValue() const;
		double maxXValue() const;
		double minYValue() const;
		double maxYValue() const;

		double z() const;
		void setZ(double);

		int xAxis() const;
		void setXAxis(int);

		int yAxis () const;
		void setYAxis(int);

		const QPen &pen() const;

		const QBrush &brush() const;

		void setSymbol(QwtSymbol *s /Transfer/);
		const QwtSymbol* symbol() const;

		void setVisible(bool on);
		bool isVisible();

		void setTitle(const QString &title);

	
};

class PlotCurve : QwtPlotCurve
{
private:
    PlotCurve(const PlotCurve &);











%TypeHeaderCode
#include "PlotCurve.h"
%End
public:
	PlotCurve(const QString& name = QString());

	int skipSymbolsCount();
	void setSkipSymbolsCount(int);


};

class FunctionCurve : PlotCurve
{
%TypeHeaderCode
#include "FunctionCurve.h"
%End
public:
	enum FunctionType{Normal = 0, Parametric = 1, Polar = 2};

	FunctionCurve(const QString& name = QString());

	double startRange();
	double endRange();
	void setRange(double, double);

	QStringList formulas();
	void setFormula(const QString& s);
	void setFormulas(const QString&, const QString&);
%MethodCode
		QStringList l;
		l << *a0;
		l << *a1;
		sipCpp->setFormulas(l);
%End

	QString variable();
	void setVariable(const QString& s);

	FunctionType functionType();
	void setFunctionType(const FunctionType& t);

	//! Returns the number of parameters in your function formula
	int parametersCount();
	//! Returns the name of the parameter of rang index
	QString parameterName(int index);
	//! Returns the value of the parameter of rang index
	double parameterValue(int index);
	//! Returns the value of the name parameter
	double parameterValue(const QString& name);

	bool loadData(int points = 0, bool xLog10Scale = false);


};

class DataCurve : PlotCurve
{
%TypeHeaderCode
#include "PlotCurve.h"
%End
public:
	DataCurve(Table *t, const QString& xColName, const QString& name, int startRow = 0, int endRow = -1);

	bool hasLabels();
	QString labelsColumnName();
	void setLabelsColumnName(const QString& name);

    int labelsAlignment();
    void setLabelsAlignment(int flags);

    int labelsXOffset();
    int labelsYOffset();
    void setLabelsOffset(int x, int y);

    double labelsRotation();
    void setLabelsRotation(double angle);

    QFont labelsFont();
    void setLabelsFont(const QFont& font);

    QColor labelsColor();
    void setLabelsColor(const QColor& c);

    bool labelsWhiteOut();
    void setLabelsWhiteOut(bool whiteOut = true);

	Table* table();
	int tableRow(int);
	int startRow();
	int endRow();
	void setRowRange(int startRow, int endRow);

	bool isFullRange();
	void setFullRange();

	bool updateData(Table *t, const QString& colName);
	void loadData();

	//! The list of attached error bars.
	QList<ErrorBarsCurve *> errorBarsList();
	//! Remove a single error bars curve from the list of attached error bars.
	void removeErrorBars(ErrorBarsCurve *);
	//! Clears the list of attached error bars.
	void clearErrorBars();

	void clearLabels();


};

class ErrorBarsCurve : DataCurve
{
%TypeHeaderCode
#include "ErrorBarsCurve.h"
%End
public:
	enum Orientation{Horizontal = 0, Vertical = 1};

	ErrorBarsCurve(int orientation, Table *t, const QString& name);
	ErrorBarsCurve(Table *t, const QString& name);

	double errorValue(int i);

	double capLength();
	void setCapLength(double t);

	double width();
	void setWidth(double w);

	QColor color();
	void setColor(const QColor& c);

	int direction();
	void setDirection(int);

	bool xErrors();
	void setXErrors(bool);

	bool throughSymbol();
	void drawThroughSymbol(bool);

	bool plusSide();
	void drawPlusSide(bool);

	bool minusSide();
	void drawMinusSide(bool);

	//! Returns the master curve to which this error bars curve is attached.
	DataCurve* masterCurve();
	void setMasterCurve(DataCurve *);

	//! Causes the master curve to delete this curve from its managed error bars list.
	void detachFromMasterCurve();

	void loadData();


};

class PieCurve : DataCurve
{
%TypeHeaderCode
#include "PieCurve.h"
%End
public:
	PieCurve(Table *t, const QString& name, int startRow, int endRow);
	void clone(PieCurve* c);

	double viewAngle();
	void setViewAngle(double);

	double thickness();
	void setThickness(double);

	double horizontalOffset();
	void setHorizontalOffset(double);

	bool counterClockwise();
	void setCounterClockwise(bool);

	double startAzimuth();
	void setStartAzimuth(double);

	double labelsEdgeDistance();
	void setLabelsEdgeDistance(double);

	bool labelsAutoFormat();
	void setLabelsAutoFormat(bool);

	bool labelsValuesFormat();
	void setLabelValuesFormat(bool);

	bool labelsPercentagesFormat();
	void setLabelPercentagesFormat(bool);

	bool labelCategories();
	void setLabelCategories(bool);

	bool fixedLabelsPosition();
	void setFixedLabelsPosition(bool);

	QColor color(int i) const;

	int radius();
	void setRadius(int);

	Qt::BrushStyle pattern();
	void setBrushStyle(const Qt::BrushStyle&);

	void setFirstColor(int);
	int firstColor();

	void loadData();
	void clearLabels();

	
};

class VectorCurve : DataCurve
{
%TypeHeaderCode
#include "VectorCurve.h"
%End
public:
	enum VectorStyle{XYXY, XYAM};
	enum Position{Tail, Middle, Head};

	VectorCurve(VectorStyle style, Table *t, const QString& xColName, const char *name,
				const QString& endCol1, const QString& endCol2, int startRow, int endRow);

private:
	VectorCurve(const VectorCurve&);
	VectorCurve& operator=(const VectorCurve&);

public:

	QString vectorEndXAColName();
	QString vectorEndYMColName();
	void setVectorEnd(const QString& xColName, const QString& yColName);

	double width();
	void setWidth(double);

	QColor color();

	int headLength();
	void setHeadLength(int);

	int headAngle();
	void setHeadAngle(int);

	bool filledArrowHead();
	void fillArrowHead(bool);

	int position();
	void setPosition(int);

	int vectorStyle();
	void setVectorStyle(int);

	void loadData();

	QPen vectorPen();

};

class QwtHistogram : DataCurve /PyName=Histogram/
{
%TypeHeaderCode
#include "QwtHistogram.h"
%End
public:
	QwtHistogram(Table *t, const QString& name, int startRow = 0, int endRow = -1);
	QwtHistogram(Matrix *m);

	void setBinning(double size, double begin, double end);
	void setAutoBinning(bool autoBin = true);
	double begin();
	double end();
	double binSize();

	void loadData();

	double mean();
	double standardDeviation();
	double minimum();
	double maximum();

	Matrix* matrix();


};

class BoxCurve : DataCurve
{
%TypeHeaderCode
#include "BoxCurve.h"
%End
public:
	enum BoxStyle{NoBox, Rect, Diamond, WindBox, Notch};
	enum Range{None, SD, SE, r25_75, r10_90, r5_95, r1_99, MinMax, UserDef};

	BoxCurve(Table *t, const QString& name, int startRow = 0, int endRow = -1);

	QwtSymbol::Style minStyle();
	void setMinStyle(QwtSymbol::Style s);

	QwtSymbol::Style maxStyle();
	void setMaxStyle(QwtSymbol::Style s);

	void setMeanStyle(QwtSymbol::Style s);
	QwtSymbol::Style meanStyle();

	void setP99Style(QwtSymbol::Style s);
	QwtSymbol::Style p99Style();

	void setP1Style(QwtSymbol::Style s);
	QwtSymbol::Style p1Style();

	int boxStyle();
	void setBoxStyle(int);

	int boxWidth();
	void setBoxWidth(int);

	double boxRange();
	int boxRangeType();
	void setBoxRange(int type, double coeff);

	double whiskersRange();
	int whiskersRangeType();
	void setWhiskersRange(int type, double coeff = 0.0);

	void loadData();

	QString statistics();
	double median();
	double quantile(double);


};

class QwtPlotSpectrogram
{
private:
  QwtPlotSpectrogram(const QwtPlotSpectrogram &);
  QwtPlotSpectrogram& operator=(const QwtPlotSpectrogram &);









%TypeHeaderCode
#include <qwt_plot_spectrogram.h>
%End
public:
	enum DisplayMode {
		ImageMode = 1,
		ContourMode = 2
	};

	QwtPlotSpectrogram (const QString &title = QString());
	void setDefaultContourPen (const QPen &);
	void setDisplayMode (DisplayMode mode, bool on = true);


};

class Spectrogram : QwtPlotSpectrogram
{
%TypeHeaderCode
#include "Spectrogram.h"
%End
public:

	Spectrogram(Graph *graph, Matrix *m);

private:
	Spectrogram(const Spectrogram&);
	Spectrogram& operator=(const Spectrogram&);

public:
	Matrix * matrix();
	bool setMatrix(Matrix *, bool = false);

	int levels();
	void setLevelsNumber(int);
	void setContourLevels (SIP_PYTUPLE);
%MethodCode
  QList<double> l;
  int n = PyTuple_Size(a0);
  for (int i=0; i<n; i++) {
    PyObject *element = PyTuple_GET_ITEM(a0,i);
    if (element) {
		l << PyFloat_AS_DOUBLE(element);
		Py_DECREF(element);
	 } else {
		sipIsErr = 1;
		break;
    }
  }
  sipCpp->setContourLevels(l);
%End

	bool hasColorScale();
	int colorScaleAxis();
	void showColorScale(int axis, bool on = true);

	int colorBarWidth();
	void setColorBarWidth(int width);

	void setGrayScale();
	void setDefaultColorMap();

	LinearColorMap *colorMapPointer()/PyName=colorMap/;
	void setCustomColorMap(const LinearColorMap& map);

	bool hasLabels();
	void showContourLineLabels(bool show = true);

	QFont labelsFont();
	void setLabelsFont(const QFont& font);

	QColor labelsColor();
	void setLabelsColor(const QColor& c);

	bool labelsWhiteOut();
	void setLabelsWhiteOut(bool whiteOut = true);

	double labelsXOffset();
    double labelsYOffset();
    void setLabelsOffset(double x, double y);

	double labelsRotation();
    void setLabelsRotation(double angle);

    bool useMatrixFormula();
    bool setUseMatrixFormula(bool on = true);

    void setColorMapPen(bool on = true);
    void setContourLinePen(int, const QPen &);
	void updateData();


};

class Grid // : QwtPlotGrid
{
%TypeHeaderCode
#include "Grid.h"
%End
public:

	Grid();

private:
	Grid(const Grid&);
	Grid& operator=(const Grid&);

public:

	//! obsolete, use enableXMaj() instead
	void enableX(bool = true) /PyName=enableXMax/;
	void enableXMaj(bool on = true);
	bool xMajEnabled();
	void enableXMin(bool = true);
	bool xMinEnabled();

	//! obsolete, use enableYMaj() instead
	void enableY(bool = true) /PyName=enableYMax/;
	void enableYMaj(bool on = true);
	bool yMajEnabled();
	void enableYMin(bool = true);
	bool yMinEnabled();

	bool xZeroLineEnabled();
	void enableZeroLineX(bool = true);
	bool yZeroLineEnabled();
	void enableZeroLineY(bool = true);
	const QPen& xZeroLinePen();
	void setXZeroLinePen(const QPen &p);
	const QPen& yZeroLinePen();
	void setYZeroLinePen(const QPen &p);

	void setMajPenX(const QPen &p);
	const QPen& majPenX();

	void setMinPenX(const QPen &p);
	const QPen& minPenX();

	void setMajPenY(const QPen &p);
	const QPen& majPenY();

	void setMinPenY(const QPen &p);
	const QPen& minPenY();


};

class QwtScaleDiv
{
%TypeHeaderCode
#include <qwt_scale_div.h>
%End
	public:
		double range();
		double lowerBound();
		double upperBound();

	
};

class QwtPlot : QFrame
{
%TypeHeaderCode
#include "qwt_plot.h"
%End


};

class Graph : QwtPlot /PyName=Layer/
{
%Docstring
The Layer class (aliased to Graph) represents a single plot layer.
%End









%TypeHeaderCode
#include "Graph.h"
#include "LegendWidget.h"
#include "MultiLayer.h"
%End
public:
  enum Scale{Linear, Log10, Ln, Log2, Reciprocal, Probability, Logit};
  enum Axis{Left, Right, Bottom, Top};
  enum Ticks{NoTicks = 0, Out = 1, InOut = 2, In = 3};
  enum CurveType{Line, Scatter, LineSymbols, VerticalBars, Area, Pie, VerticalDropLines,
			Spline, HorizontalSteps, Histogram, HorizontalBars, VectXYXY, ErrorBars,
			Box, VectXYAM, VerticalSteps, ColorMap, GrayScale, Contour, Function, ImagePlot,
			StackBar, StackColumn};

  bool isPiePlot();
  SIP_PYOBJECT pieLegendText() /PyName=pieLegend/;
%MethodCode
  sipRes = PyUnicode_FromString(sipCpp->pieLegendText().toUtf8().constData());
%End

  void undoSetScale(int axis, double start, double end, double step = 0.0, int majorTicks = 5, int minorTicks = 5, int type = 0, bool inverted = false);
  void undoSetAxisTitle(int axis, const QString &text);
  void undoSetAxisTitleColor(int axis, const QColor &color);
  void undoSetCanvasBackground(const QColor &color);
  void undoShowGrid(int axis, bool on = true, bool minor = false);
  void undoSetCurvePen(int curveIndex, const QPen &pen);
  void undoSetCurveBrush(int curveIndex, const QBrush &brush);
  bool isGridEnabled(int axis);
  void showGrid(int axis, bool on, bool minor = false);
  QBrush canvasBackground();
  QString axisTitleString(int axis);

  void undo();
%Docstring
undo()
Undoes the last operation on this plot.
%End
%MethodCode
  if (sipCpp->multiLayer() && sipCpp->multiLayer()->undoStack())
    sipCpp->multiLayer()->undoStack()->undo();
%End

  void redo();
%Docstring
redo()
Redoes the last undone operation on this plot.
%End
%MethodCode
  if (sipCpp->multiLayer() && sipCpp->multiLayer()->undoStack())
    sipCpp->multiLayer()->undoStack()->redo();
%End

  DataCurve* insertCurve(Table*, const QString&, int style = 1, int startRow = 0, int endRow = -1);
  DataCurve* insertCurve(Table*, const QString&, const QString&, int style = 1, int startRow = 0, int endRow = -1);
  bool addCurves(Table*, SIP_PYTUPLE, int=0, double=1, int=3, int=0, int=-1);
%MethodCode
  QStringList l;
  int n = PyTuple_Size(a1);
  for (int i=0; i<n; i++) {
    PyObject *str = PyObject_Str(PyTuple_GET_ITEM(a1,i));
    if (str) {
      l << PyUnicode_AsUTF8(str);
		Py_DECREF(str);
	 } else {
      sipIsErr = 1;
      break;
    }
  }
  sipRes = sipCpp->addCurves(a0, l, a2, a3, a4, a5, a6);
%End

  bool addCurve(Table*, const QString&, int=0, double=1, int=3, int=0, int=-1);
%Docstring
addCurve(table, columnName, style=0) -> bool
Adds a curve to the layer from the specified table column.
%End
%MethodCode
	if (a0 == 0) {
	   sipIsErr = 1;
	   PyErr_Format(PyExc_ValueError, "Invalid table in addCurve().");
	} else {
	  QStringList l;
	  l << *a1;
	  sipRes = sipCpp->addCurves(a0, l, a2, a3, a4, a5, a6);
	}
%End
  void removeCurve(int);
%Docstring
removeCurve(index)
Removes the curve at the specified index.
%End

  void removeCurve(QwtPlotCurve*);
  void deleteFitCurves();
  int curveCount() /PyName=numCurves/;
%Docstring
numCurves() -> int
Returns the number of curves in the layer.
%End
  PlotCurve* curve(int index);
  PlotCurve* curve(const QString &title);
  QString curveTitle(int);
  DataCurve* dataCurve(int index);
  DataCurve *selectedDataCurve();
  int selectionActivePoint();
  int selectionInactivePoint();
  bool rangeSelectorsEnabled();

  void plotBox(Table *, const QStringList& names, int startRow = 0, int endRow = -1);
  BoxCurve * boxCurve(int index);

  void changeCurveIndex(int fromIndex, int toIndex);
  void reverseCurveOrder();

  void setCurveLineColor(int, int);
  void setCurveLineColor(int, QColor);
  void setCurveLineStyle(int, Qt::PenStyle);
  void setCurveLineWidth(int, double);

  void showMissingDataGap(bool on = true, bool update = true);
  bool isMissingDataGapEnabled();

  void setGrayScale();
  void setIndexedColors();

	FunctionCurve * functionCurve(int index);
	FunctionCurve* addFunction(const QString&, double, double, int=100);
%MethodCode
  QStringList l;
  l << *a0;
  sipRes = sipCpp->addFunction(l, a1, a2, a3);
%End
	FunctionCurve* addPolarFunction(const QString&, const QString&, double, double, int=100, const QString & = "t");
%MethodCode
  QStringList l;
  l << *a0;
  l << *a1;
  sipRes = sipCpp->addFunction(l, a2, a3, a4, *a5, 2);
%End
	FunctionCurve* addParametricFunction(const QString&, const QString&, double, double, int=100, const QString & = "m");
%MethodCode
  QStringList l;
  l << *a0;
  l << *a1;
  sipRes = sipCpp->addFunction(l, a2, a3, a4, *a5, 1);
%End

  ErrorBarsCurve* addErrorBars(const QString&, Table *, const QString&,
		  int type = 1, double width = 1, double cap = 8, const QColor& color = QColor(Qt::black),
		  bool through = true, bool minus = true, bool plus = true);
  ErrorBarsCurve* addErrorBars(DataCurve *, Table *, const QString&,
		  int type = 1, double width = 1, double cap = 8, const QColor& color = QColor(Qt::black),
		  bool through = true, bool minus = true, bool plus = true);

  QwtHistogram* histogram(int);
  QwtHistogram* addHistogram(Table*, const QString& colName, int startRow = 0, int endRow = -1);
  QwtHistogram* addHistogram(Matrix*);
  Spectrogram* plotSpectrogram(Matrix *m, CurveType type);
  Spectrogram* spectrogram(Matrix *m);

  PieCurve* plotPie(Table*,const QString&, int startRow = 0, int endRow = -1);
  VectorCurve* plotVectors(Table*, const QStringList& colList, int style, int startRow = 0, int endRow = -1);

  ArrowMarker* addArrow(ArrowMarker*);
  void remove(ArrowMarker*);
  QList<ArrowMarker *> arrowsList();
  int numArrows();

  ImageWidget* addImage(ImageWidget*);


  void remove(ImageWidget*);

  void setTitle(const QString& t);
  void setTitleFont(const QFont &fnt);
  void setTitleColor(const QColor &c);
  void setTitleAlignment(int align);
  void removeTitle();

  LegendWidget* newLegend(const QString& = QString());

  LegendWidget* legend();
  void removeLegend();

  LegendWidget* addText(LegendWidget*);
  void remove(LegendWidget*);

  LegendWidget* addTimeStamp();

  FrameWidget* add(FrameWidget*, bool = true);
  void remove(FrameWidget*);

  void setXAxisTitle(const QString& text) /PyName=setXTitle/;
%Docstring
setXTitle(text)
Sets the title of the X axis.
%End
  void setYAxisTitle(const QString& text) /PyName=setYTitle/;
%Docstring
setYTitle(text)
Sets the title of the Y axis.
%End

  void enableAxis(int axis, bool on = true);
  void setAxisColor(int axis, const QColor& color);
  void setAxisFont(int axis, const QFont &fnt);

  void setAxisTitle(int axis, const QString& text);
  void setAxisTitleFont(int axis,const QFont &fnt);
  void setAxisTitleColor(int axis, const QColor& c);
  void setAxisTitleAlignment(int axis, int align);

  int axisTitleDistance(int axis);
  void setAxisTitleDistance(int axis, int dist);

  void setAxisTicksLength(int axis, int majTicksType, int minTicksType, int minLength, int majLength);
  void setAxisLabelRotation(int axis, int rotation);

  void setAxesLinewidth(int width);
  void drawAxesBackbones(bool yes);
  void setTicksLength(int minLength, int majLength);
  int minorTickLength() const;
  int majorTickLength() const;
  void setMajorTicksType(int axis, int type);
  void setMinorTicksType(int axis, int type);

  void enableAxisLabels(int axis, bool on);
  void setAxisLabelsColor(int axis, const QColor& color);
  void setLabelsNumericFormat(int axis, int format, int = 6, const QString& = QString()) /PyName=setAxisNumericFormat/;

  QwtScaleDiv axisScaleDiv (int axisId);
  void setScale(int axis, double start, double end, double step = 0.0,
                int majorTicks = 5, int minorTicks = 5, int type = 0, bool inverted = false,
				double left_break = -DBL_MAX, double right_break = DBL_MAX, int pos = 50,
				double stepBeforeBreak = 0.0, double stepAfterBreak = 0.0, int minTicksBeforeBreak = 4,
				int minTicksAfterBreak = 4, bool log10AfterBreak = false, int breakWidth = 4, bool breakDecoration = true);
  void setAutoScale();

  bool hasSynchronizedScaleDivisions();
  void setSynchronizedScaleDivisions(bool);

  void setAxisMargin(int, int);
  void setFrame(int width = 1, const QColor& color = QColor(Qt::black));
  void setCanvasFrame(int width = 1, const QColor& color =  QColor(Qt::black));
  void setBackgroundColor(const QColor& color);
  void setCanvasBackground(const QColor& color) /PyName=setCanvasColor/;

  QPixmap backgroundPixmap();
  QString canvasBackgroundFileName();
  void setCanvasBackgroundImage (const QString & = QString(), bool = true);

  void setCanvasGeometry(int x, int y, int w, int h);
  void setCanvasGeometry(const QRect &canvasRect);
  void setCanvasSize(int w, int h);
  void setCanvasSize(const QSize &size);

  Grid* grid();
  void showGrid(int);
  void showGrid();
  void setGridOnTop(bool on = true, bool update = true);
  bool hasGridOnTop();

  void replot();
%Docstring
Redraws the layer.
%End

  void exportTeX(const QString& fname, bool color = true, bool escapeStrings = true, bool fontSizes = true,
				const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0)/PyName=exportTex/;
  void exportImage(const QString& fileName, int quality = -1, bool transparent = false, int dpi = 0,
		const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0, int compression = 0);
  void exportVector(const QString&, int = 0, bool = true,
				const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
  void exportToFile(const QString& fileName) /PyName=export/;
%Docstring
export(filename)
Exports the layer to an image or vector file.
%End

  void enableAutoscaling(bool = true);
  void setAutoscaleFonts(bool = true);
  void setAntialiasing(bool on = true, bool update = true);

    void setCurveAxes(int,int,int);
%MethodCode
	sipIsErr = 0;
	if ((a1 > 3) || (a1 < 2) || (a2 > 1) || (a2 < 0))
    {
         sipIsErr=1;
         PyErr_Format(PyExc_ValueError, "Invalid axis attachment!");\
    }
	if (a0>=sipCpp->curveCount())
	{
         sipIsErr=1;
         PyErr_Format(PyExc_ValueError, "Invalid curve index! is %d, should be less than %d",a0,sipCpp->curveCount());\
	}
	if (sipIsErr == 0)
	{
        QwtPlotItem* pItem;
		pItem = sipCpp->plotItem(a0);
		pItem->setAxes(a1, a2);
		sipCpp->setAutoScale();
	}
%End

  void setWaterfallSideLines(bool = true);

  void setWaterfallOffset(double x, double y, bool = true);


};

class MultiLayer : MdiSubWindow /PyName=Graph/
{
%Docstring
The Graph class (aliased to MultiLayer) represents a window containing one or more plot layers.
%End









%TypeHeaderCode
#include "MultiLayer.h"
%End
public:
  MultiLayer (ApplicationWindow* parent = 0, int layers = 1, int rows = 1, int cols = 1, const QString& label = "", const char* name=0, Qt::WindowFlags f = Qt::Widget);

  enum HorAlignement{HCenter, Left, Right};
  enum VertAlignement{VCenter, Top, Bottom};
  enum AlignPolicy{AlignLayers = 0, AlignCanvases};

  Graph *activeLayer();
%Docstring
Returns the currently active layer.
%End

  void undo();
%Docstring
undo()
Undoes the last operation on this multilayer graph.
%End
%MethodCode
  if (sipCpp->undoStack())
    sipCpp->undoStack()->undo();
%End

  void redo();
%Docstring
redo()
Redoes the last undone operation on this multilayer graph.
%End
%MethodCode
  if (sipCpp->undoStack())
    sipCpp->undoStack()->redo();
%End

  void setActiveLayer(Graph*);
%Docstring
Sets the active layer.
%End
  int numLayers();
%Docstring
Returns the number of layers in the graph window.
%End
  void setNumLayers(int n);
%Docstring
Sets the number of layers in the graph window.
%End
  Graph* layer(int num);
%Docstring
layer(index) -> Layer
Returns the layer at the specified index.
%End
  QList<Graph *> layersList();
  Graph* addLayer(int = 0, int = 0, int = 0, int = 0, bool = false);
%Docstring
addLayer() -> Layer
Adds a new layer to the graph window.
%End

  bool removeLayer(Graph *g);
%Docstring
removeLayer(layer)
Removes the specified layer.
%End
  bool removeActiveLayer();
%Docstring
Removes the active layer.
%End

  void setCols(int);
  void setRows(int);
  void setSpacing (int, int);
  void setMargins (int, int, int, int);
  void setLayerCanvasSize (int, int);
  void setAlignement (int, int);
  void arrangeLayers(bool fit = true, bool userSize = false);
%Docstring
arrangeLayers()
Automatically arranges the layers.
%End
  bool swapLayers(int, int);

  void setScaleLayersOnResize(bool = true);
  void setEqualSizedLayers();

  void setAlignPolicy(const AlignPolicy& policy);
  void setCommonLayerAxes(bool verticalAxis = true, bool horizontalAxis = true);
  void linkXLayerAxes(bool = true);

  void exportToFile(const QString& fileName) /PyName=export/;
%Docstring
export(filename)
Exports the graph window to an image or vector file.
%End
  void exportImage(const QString& fileName, int quality = -1, bool transparent = false,
		int dpi = 0, const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel,
		double fontsFactor = 1.0, int compression = 0);
  void exportVector(const QString&, int = 0, bool = true,
		const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
  void exportTeX(const QString& fname, bool color = true, bool escapeStrings = true, bool fontSizes = true,
		const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0)/PyName=exportTex/;

  void reverseWaterfallOrder();


};

class Note: MdiSubWindow
{
private:
    Note(const Note &);











%TypeHeaderCode
#include "Note.h"
%End
public:
  Note(ScriptingEnv *env, const QString& label, ApplicationWindow* parent, const QString& name = QString(), Qt::WindowFlags f = Qt::Widget);
  bool autoexec() const;
  void setAutoexec(bool = true);

  QString text();
  void setText(const QString &s);

  void exportPDF(const QString& fileName);
  QString exportASCII(const QString &file=QString())/PyName=saveAs/;
  QString importASCII(const QString &file=QString());

  void showLineNumbers(bool = true);

  void setFont(const QFont& f);
  void setTabStopDistance(double length);

  int tabs();
  void addTab();
  void removeTab(int = -1);
  void renameTab(int, const QString&);

  int indexOf(ScriptEdit*);
  ScriptEdit* editor(int index);
  ScriptEdit* currentEditor();


};

class Graph3D: MdiSubWindow
{
%Docstring
The Plot3D class (aliased to Graph3D) represents a 3D plot window.
%End
%TypeHeaderCode
#include "Graph3D.h"
%End
public:
  Graph3D (const QString& label, ApplicationWindow* parent, const char* name=0, Qt::WindowFlags f = Qt::Widget);
	enum PlotType{Scatter = 0, Trajectory = 1, Bars = 2, Ribbon = 3};
	enum AxisNumericFormat{Default = 0, Decimal = 1, Scientific = 2, Engineering = 3};

    void exportToFile(const QString& fileName) /PyName=export/;
    void exportVector(const QString& fileName, int = 0, int = 1,
		const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0);
	void exportImage(const QString& fileName, int quality = -1, bool transparent = false, int dpi = 0,
		const QSizeF& customSize = QSizeF(), int unit = FrameWidget::Pixel, double fontsFactor = 1.0, int compression = 0);

    void setRotation(double xVal, double yVal, double zVal);
%Docstring
setRotation(x, y, z)
Sets the rotation of the 3D plot.
%End
	void setScale(double xVal, double yVal, double zVal);
%Docstring
setScale(x, y, z)
Sets the scale factor for each axis.
%End
	void setShift(double xVal, double yVal, double zVal);
%Docstring
setShift(x, y, z)
Sets the shift of the 3D plot.
%End
    void setZoom(double val);
%Docstring
setZoom(value)
Sets the zoom factor.
%End
    void setOrthogonal(bool = true);
    void setMeshLineWidth(int);
%Docstring
setMeshLineWidth(width)
Sets the line width for the 3D mesh.
%End
  void setMeshColor(const QColor&);
%Docstring
setMeshColor(color)
Sets the color of the wireframe mesh.
%End




    void setLabelsDistance(int);


    void setDataColors(const QColor&, const QColor&);
	QString colorMapFile();
    void setDataColorMap(const QString& fileName);
	LinearColorMap *colorMapPointer()/PyName=colorMap/;

    void changeTransparency(double) /PyName=setOpacity/;
    void setResolution(int);
    void showColorLegend(bool = true) /PyName=showLegend/;
    void setTitle(const QString&, const QColor& color = QColor(Qt::black), const QFont& font = QFont());
    void setAntialiasing(bool = true);
%Docstring
setAntialiasing(on=True)
Enables or disables antialiasing.
%End
    void setLeftGrid(bool = true);
	void setRightGrid(bool = true);
	void setCeilGrid(bool = true);
	void setFloorGrid(bool = true);
	void setFrontGrid(bool = true);
	void setBackGrid(bool = true);

    void setFramed();
	void setBoxed();
	void setNoAxes();

	void setFloorData() /PyName=showFloorProjection/;
	void setFloorIsolines() /PyName=showFloorIsolines/;
	void setEmptyFloor();

    void setPolygonStyle();
	void setHiddenLineStyle();
	void setWireframeStyle();
	void setFilledMeshStyle();
	void setDotStyle();
	void setBarStyle();
    void setConeStyle();
    void setCrossStyle();

    void setConeOptions(double rad, int quality);
    void setCrossOptions(double rad, double linewidth, bool smooth, bool boxed);
    void setDotOptions(double size, bool smooth);
    void setBarRadius(double rad);
    void setBarLines(bool lines = true);
    void setFilledBars(bool filled = true);

    void animate(bool = true);
%Docstring
animate(on=True)
Enables or disables plot animation.
%End
    void findBestLayout();

    void changeDataColumn(Table* table, const QString& colName, int = 0) /PyName=setData/;
%Docstring
setData(table, colName)
Sets the data source for the 3D plot from a table column.
%End
    void addMatrixData(Matrix* ) /PyName=setMatrix/;
%Docstring
setMatrix(matrix)
Sets the data source for the 3D plot from a matrix.
%End
    void addFunction(const QString&, double, double, double, double, double, double, int = 40, int = 40) /PyName=setFunction/;
%Docstring
setFunction(formula, xl, xr, yl, yr, zl, zr, columns=40, rows=40)
Sets the data source for the 3D plot from a function formula.
%End
    void addParametricSurface(const QString&, const QString&, const QString&, double,
                        double, double, double, int=40, int=40, bool=true, bool=true) /PyName=setParametricSurface/;

    void update();



	void setXAxisTickLength(double majorLength, double minorLength);
    void setYAxisTickLength(double majorLength, double minorLength);
	void setZAxisTickLength(double majorLength, double minorLength);

	void setScales(double xl, double xr, double yl, double yr, double zl, double zr, int axis = -1);

	int axisNumericFormat(int axis);
	int axisNumericPrecision(int axis);
	void setAxisNumericFormat(int axis, int format, int precision);
	void setXAxisNumericFormat(int format, int precision);
%MethodCode
	sipCpp->setAxisNumericFormat(0, a0, a1);
%End

 void setYAxisNumericFormat(int format, int precision);
%MethodCode
	sipCpp->setAxisNumericFormat(1, a0, a1);
%End

 void setZAxisNumericFormat(int format, int precision);
%MethodCode
	sipCpp->setAxisNumericFormat(2, a0, a1);
%End


};

class ApplicationWindow: QMainWindow
{
%Docstring
The main application instance. Accessible via ``qti.app``.
%End
private:
    ApplicationWindow(const ApplicationWindow &);











%TypeHeaderCode
#include "ApplicationWindow.h"
%End
%ConvertToSubClassCode
// we have to do this to override casting in qt/qobject.sip
 return sipFindType(sipCpp->metaObject()->className());
%End

public:
  void open(const QString&, bool = false, bool = true);
%Docstring
open(filename, factoryVersion=False, newProject=True)
Opens a project file.
%End
  void saveProject();
%Docstring
saveProject()
Saves the current project.
%End
%MethodCode
  sipCpp->saveProject();
%End
  void savedProject();
  void closeProject();
%Docstring
closeProject()
Closes the current project.
%End
  void quit();
%Docstring
quit()
Exits the application.
%End
%MethodCode
  qApp->quit();
%End

  void undo();
%Docstring
undo()
Undoes the last operation in the active window.
%End
%MethodCode
  sipCpp->undo();
%End

  void redo();
%Docstring
redo()
Redoes the last undone operation in the active window.
%End
%MethodCode
  sipCpp->redo();
%End

  ApplicationWindow();
  enum MatrixToTableConversion{Direct, XYZ, YXZ};

  Table* newTable();
%Docstring
Creates a new table.
%End
  Table* currentTable();
%Docstring
Returns the currently active table.
%End
  Table* newTable(const QString&, int=30, int=2);
%Docstring
newTable(name, rows=30, cols=2) -> Table
Creates a new table with the specified name and dimensions.
%End
%MethodCode
  sipRes = sipCpp->newTable(*a0, a1, a2);
%End

  Matrix* newMatrix();
  Matrix* currentMatrix();
  Matrix* newMatrix(const QString&, int=32, int=32);
  MultiLayer *plot(const QString&) /PyName=graph/;
%MethodCode
  sipRes = sipCpp->current_folder->graph(*a0, false);
  if(!sipRes)
    sipRes = sipCpp->projectFolder()->graph(*a0, true);
%End
  MultiLayer* currentPlot() /PyName=currentGraph/;

  MultiLayer* newGraph(const QString& = "Graph1", int = 1, int = 1, int = 1);
%MethodCode
	if (a1 == 1)
		sipRes = sipCpp->newGraph(*a0);
	else
		sipRes = sipCpp->multilayerPlot(*a0, a1, a2, a3);
%End

  Note* newNote(const QString& = QString());
  Note* currentNote();
  MultiLayer *plot(Table*, SIP_PYTUPLE, int=0);
%MethodCode
	if (a0 == 0) {
	   sipIsErr = 1;
	   PyErr_Format(PyExc_ValueError, "Invalid table in argument to plot().");
	} else {
	  QStringList l;
	  int n = PyTuple_Size(a1);
	  for (int i=0; i<n; i++) {
		PyObject *str = PyObject_Str(PyTuple_GET_ITEM(a1,i));
		if (str) {
		  l << PyUnicode_AsUTF8(str);
			Py_DECREF(str);
		 } else {
		  sipIsErr = 1;
		  break;
		}
	  }
	  sipRes = sipCpp->multilayerPlot(a0, l, a2);
	}
%End

	MultiLayer *plot(Table*, const QString&, int=0);
%MethodCode
	if (a0 == 0){
		sipIsErr = 1;
		PyErr_Format(PyExc_ValueError, "Invalid table in argument to plot().");
	} else
		sipRes = sipCpp->multilayerPlot(a0, QStringList(*a1), a2);
%End

	MultiLayer *plot(Table*, int, int=0);
%MethodCode
	if (a0 == 0){
		sipIsErr = 1;
		PyErr_Format(PyExc_ValueError, "Invalid table in argument to plot().");
	} else
		sipRes = sipCpp->multilayerPlot(a0, QStringList(a0->colName(a1 - 1)), a2);
%End

	MultiLayer *waterfallPlot(Table*, SIP_PYTUPLE);
%MethodCode
	if (a0 == 0) {
	   sipIsErr = 1;
	   PyErr_Format(PyExc_ValueError, "Invalid table in argument to waterfallPlot().");
	} else {
	  QStringList l;
	  int n = PyTuple_Size(a1);
	  for (int i=0; i<n; i++) {
		PyObject *str = PyObject_Str(PyTuple_GET_ITEM(a1,i));
		if (str) {
		  l << PyUnicode_AsUTF8(str);
			Py_DECREF(str);
		 } else {
		  sipIsErr = 1;
		  break;
		}
	  }
	  sipRes = sipCpp->waterfallPlot(a0, l);
	}
%End

	Matrix* importImage(const QString&, bool);
	MultiLayer* plotSpectrogram(Matrix*, Graph::CurveType = Graph::ColorMap) /PyName=plot/;
	MultiLayer* plotImageProfiles(Matrix*);
	MultiLayer* plot(const QString&, double, double, int = 100);
%MethodCode
  sipRes = sipCpp->newFunctionPlot(QStringList() << *a0, a1, a2, a3);
%End

    Graph3D* newPlot3D(const QString& = QString());
    Graph3D* plotXYZ(Table* table, const QString&, int = 0) /PyName=plot3D/;
	Graph3D* plot3DMatrix(Matrix *, int style = 5) /PyName=plot3D/;
    Graph3D* plotSurface(const QString&, double xl, double xr, double yl, double yr,
			double zl, double zr, int=40, int=40) /PyName=plot3D/;
    Graph3D* plotParametricSurface(const QString&, const QString&, const QString&,
            double, double, double, double, int=40, int=40, bool=true, bool=true) /PyName=plot3D/;

  QList<MdiSubWindow*> windowsList() /PyName=windows/;
%Docstring
windows() -> list
Returns a list of all windows in the project.
%End
  MdiSubWindow* activeWindow();
%Docstring
activeWindow() -> Window
Returns the currently active window.
%End

  // folders
  Folder *activeFolder() ;
%MethodCode
  sipRes = sipCpp->current_folder;
%End

  Folder* appendProject(const QString& file_name, Folder* parentFolder = 0);
  void saveFolder(Folder *folder, const QString& fn, bool=false);
  Folder* projectFolder() /PyName=rootFolder/;

  Folder* addFolder(QString name, Folder* parent = 0);
  bool deleteFolder(Folder *);
  bool changeFolder(Folder *, bool force = false);
  bool copyFolder(Folder *src, Folder *dest);

  void saveAsTemplate(MdiSubWindow*, const QString&);

  void setWindowName(MdiSubWindow *, const QString &);
  void setPreferences(Graph*);

  void saveProjectAs(const QString& fileName = QString(), bool = false);
%Docstring
saveProjectAs(filename)
Saves the current project to a new file.
%End
  MdiSubWindow* clone(MdiSubWindow*);
%Docstring
clone(window) -> MdiSubWindow
Clones the specified window object.
%End

  Matrix* tableToMatrix(Table* t);
  Matrix* tableToMatrixRegularXYZ(Table* t, const QString& colName);
  Table* matrixToTable(Matrix* m, MatrixToTableConversion = Direct);

  bool writeFitResultsToLog();
  void setWriteFitResultsToLog(bool on = true);
%Docstring
setWriteFitResultsToLog(on=True)
Enables or disables automatic logging of fit results.
%End

  QTextEdit *resultsLog();
  void displayInfo(const QString& text);
  QLineEdit *infoLineEdit();
  QString stemPlot(Table *t, const QString& colName, int power = 1001, int startRow = 0, int endRow = -1);

  QMdiArea* workspace();
  Table* importOdfSpreadsheet(const QString& = QString(), int = -1);
  Table* importExcel(const QString& = QString(), int = -1);
  Table* importWaveFile();


};

class Fit : Filter
{
%Docstring
The Fit class provides access to the data fitting API.
%End
%TypeHeaderCode
#include "Fit.h"
%End
public:
  enum Algorithm{ScaledLevenbergMarquardt, UnscaledLevenbergMarquardt, NelderMeadSimplex};
  enum WeightingMethod{NoWeighting, Instrumental, Statistical, Dataset, Direct};
  Fit(ApplicationWindow * app /TransferThis/, Graph*=0, const char*=0);
  ~Fit();

  virtual void fit();
%Docstring
Runs the fit.
%End
  virtual bool run();
%Docstring
Runs the fit. Returns True on success.
%End

  bool setWeightingData(WeightingMethod, const QString&=QString());
  bool setDataFromTable(Table *, const QString&, const QString&, int = 1, int = -1, bool = false);
  void setInterval(double from, double to);

  QString formula();
  QString resultFormula();
  int numParameters();
  QStringList parameterNames();
%Docstring
Returns a list of parameter names.
%End

  void setInitialGuess(int, double) /PyName=setInitialValue/;
%Docstring
setInitialValue(index, value)
Sets the initial guess for the specified parameter.
%End
  void setInitialGuesses(...) /PyName=setInitialValues/;
%Docstring
setInitialValues(val1, val2, ...)
Sets the initial guesses for all parameters.
%End
%MethodCode
int n = PyTuple_GET_SIZE(a0);
double *values = new double[n];
for (int i=0; i<n; i++) {
  PyObject *item = PyTuple_GET_ITEM(a0, i);
  if (PyNumber_Check(item)) {
    item=PyNumber_Float(item);
    if (!item) {
      sipIsErr=1;
      break;
    }
    values[i] = PyFloat_AS_DOUBLE(item);
    Py_DECREF(item);
  } else
    values[i] = 0;
}
sipCpp->setInitialGuesses(values);
delete values;
%End

  virtual void guessInitialValues();
  void setParameterRange(int, double, double);
%Docstring
setParameterRange(index, min, max)
Sets the allowed range for the specified parameter.
%End

  void setAlgorithm(Algorithm);
  void setOutputPrecision(int);
  void generateFunction(bool, int=100);

  void showLegend();
  virtual QString legendInfo();

  void scaleErrors(bool yes = true);

  SIP_PYTUPLE results();
%Docstring
Returns the fitted parameter values.
%End
%MethodCode
double *results = sipCpp->results();
int size=sipCpp->numParameters();
sipRes = PyTuple_New(size);
if(sipRes)
{
  for(int i=0; i<size; i++)
    PyTuple_SET_ITEM(sipRes, i, PyFloat_FromDouble(results[i]));
} else
  sipIsErr = 1;
%End

  SIP_PYTUPLE errors();
%Docstring
Returns the estimated errors for the fitted parameters.
%End
%MethodCode
double *errors = sipCpp->errors();
int size=sipCpp->numParameters();
sipRes = PyTuple_New(size);
if(sipRes)
{
  for(int i=0; i<size; i++)
    PyTuple_SET_ITEM(sipRes, i, PyFloat_FromDouble(errors[i]));
} else
  sipIsErr = 1;
%End

	SIP_PYTUPLE residuals();
%MethodCode
double *residuals = sipCpp->residuals();
int size = sipCpp->dataSize();
sipRes = PyTuple_New(size);
if(sipRes)
{
  for(int i=0; i<size; i++)
    PyTuple_SET_ITEM(sipRes, i, PyFloat_FromDouble(residuals[i]));
} else
  sipIsErr = 1;
%End

  double chiSquare();
%Docstring
Returns the chi-square value of the fit.
%End
  double rSquare();
%Docstring
Returns the R-square value of the fit.
%End
  double adjustedRSquare();
  double rss();
  double rmse();

  void showPredictionLimits(double);
  void showConfidenceLimits(double);
  double lcl(int, double);
  double ucl(int, double);


};

%ModuleCode
ApplicationWindow *sipqti_app()
{
  int iserr = 0;
  PyObject *me = PyImport_ImportModule("qti");
  PyObject *mydict = PyModule_GetDict(me);
  PyObject *pyapp = PyDict_GetItemString(mydict,"app");
  Py_DECREF(me);
  if (sipCanConvertToType(pyapp, sipType_ApplicationWindow, SIP_NOT_NONE))
    return (ApplicationWindow*) sipConvertToType(pyapp, sipType_ApplicationWindow, NULL, SIP_NOT_NONE, NULL, &iserr);
  else
    return NULL;
}
%End
%ModuleHeaderCode
class ApplicationWindow;
ApplicationWindow *sipqti_app();
#define SIPQTI_APP(sipcppexpr)\
ApplicationWindow *app = sipqti_app();\
if (app) sipCpp = sipcppexpr;\
else sipCpp = NULL;
%End

class ExponentialFit : Fit
{
%TypeHeaderCode
#include "ExponentialFit.h"
%End
public:
  ExponentialFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1, bool = false);
  ExponentialFit(ApplicationWindow * app /TransferThis/, PlotCurve *, bool=false);
  ExponentialFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double, bool=false);
  ExponentialFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, bool=false);
  ExponentialFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double, bool=false);
};

class TwoExpFit : Fit
{
%TypeHeaderCode
#include "ExponentialFit.h"
%End
public:
  TwoExpFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  TwoExpFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  TwoExpFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  TwoExpFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  TwoExpFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);
};

class ThreeExpFit : Fit
{
%TypeHeaderCode
#include "ExponentialFit.h"
%End
public:
  ThreeExpFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  ThreeExpFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  ThreeExpFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  ThreeExpFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  ThreeExpFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);
};

class SigmoidalFit : Fit
{
%TypeHeaderCode
#include "SigmoidalFit.h"
%End
public:
  SigmoidalFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  SigmoidalFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  SigmoidalFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  SigmoidalFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  SigmoidalFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);

  void guessInitialValues();
};

class LogisticFit : Fit
{
%TypeHeaderCode
#include "LogisticFit.h"
%End
public:
  LogisticFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  LogisticFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  LogisticFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  LogisticFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  LogisticFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);

  void guessInitialValues();
};

class GaussAmpFit : Fit
{
%TypeHeaderCode
#include "MultiPeakFit.h"
%End
public:
  GaussAmpFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  GaussAmpFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  GaussAmpFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  GaussAmpFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  GaussAmpFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);

  void guessInitialValues();
};

class NonLinearFit : Fit
{
%TypeHeaderCode
#include "NonLinearFit.h"
%End
public:
  NonLinearFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  NonLinearFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  NonLinearFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  NonLinearFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  NonLinearFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);
  void setParameters(...);
%MethodCode
  QStringList l;
  const char *item;
  for (int i=0; i<PyTuple_GET_SIZE(a0); i++)
    if (item = PyUnicode_AsUTF8(PyTuple_GET_ITEM(a0, i)))
      l << item;
    else
      sipIsErr = 1;

  sipCpp->setParametersList(l);
%End
  // TODO: make it accept Python callables
  bool setFormula(const QString&, bool = true);


};

class PluginFit : Fit
{
%TypeHeaderCode
#include "PluginFit.h"
%End
public:
  PluginFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  PluginFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  PluginFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  PluginFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  PluginFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);

};

class MultiPeakFit : Fit
{
%TypeHeaderCode
#include "MultiPeakFit.h"
%End
public:
  enum PeakProfile{Gauss, Lorentz};
  MultiPeakFit(ApplicationWindow * app /TransferThis/, PlotCurve *, PeakProfile=Gauss, int=1);
  MultiPeakFit(ApplicationWindow * app /TransferThis/, Graph *, PeakProfile=Gauss, int=1);

  int peaks() /PyName=numPeaks/;
  void setNumPeaks(int);

  void enablePeakCurves(bool);
  void setPeakCurvesColor(int);

  static QString generateFormula(int, PeakProfile);
  static QStringList generateParameterList(int);

  void guessInitialValues();
};

class LorentzFit : MultiPeakFit
{
%TypeHeaderCode
#include "MultiPeakFit.h"
%End
public:
  LorentzFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  LorentzFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  LorentzFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  LorentzFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  LorentzFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, int, int);
};

class GaussFit : MultiPeakFit
{
%TypeHeaderCode
#include "MultiPeakFit.h"
%End
public:
  GaussFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  GaussFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  GaussFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  GaussFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  GaussFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);
};

class PolynomialFit : Fit
{
%TypeHeaderCode
#include "PolynomialFit.h"
%End
public:
  PolynomialFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1, int = 2, bool = false);
  PolynomialFit(ApplicationWindow * app /TransferThis/, PlotCurve *, int=2, bool=false);
  PolynomialFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double, int=2, bool=false);
  PolynomialFit(ApplicationWindow * app /TransferThis/, Graph *, QString&, int=2, bool=false);
  PolynomialFit(ApplicationWindow * app /TransferThis/, Graph *, QString&, double, double, int=2, bool=false);

  virtual QString legendInfo();
  void fit();

  static QString generateFormula(int);
  static QStringList generateParameterList(int);
};

class LinearFit : Fit
{
%TypeHeaderCode
#include "PolynomialFit.h"
%End
public:
  LinearFit(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  LinearFit(ApplicationWindow * app /TransferThis/, PlotCurve *);
  LinearFit(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  LinearFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  LinearFit(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);
  void fit();
};

class Filter : QObject
{
%TypeHeaderCode
#include "Filter.h"
%End
public:
  Filter(ApplicationWindow * app /TransferThis/, Table *t, const QString& name = QString());
  Filter(ApplicationWindow * app /TransferThis/, Graph *g, const QString& name = QString());
  Filter(ApplicationWindow * app /TransferThis/, Matrix *m, const QString& name = QString());
  Filter(ApplicationWindow * app /TransferThis/, PlotCurve *c);
  ~Filter();

  void setOutputPoints(int);
  void setTolerance(double);
  void setMaximumIterations(int);

  void setColor(int);


  bool setDataFromCurve(PlotCurve *c);
  bool setDataFromCurve(PlotCurve *c, double from, double to);
  bool setDataFromCurve(const QString&, Graph*=0);
  bool setDataFromCurve(const QString& curveTitle, double from, double to, Graph *g = 0);
  virtual bool setDataFromTable(Table *t, const QString& xColName, const QString& yColName, int from = 1, int to = -1, bool = false);

  Table *resultTable();

  virtual void enableGraphicsDisplay(bool on = true, Graph *g = 0);
  virtual bool run();
};

class Differentiation : Filter
{
%TypeHeaderCode
#include "Differentiation.h"
%End
public:
  Differentiation(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);
  Differentiation(ApplicationWindow * app /TransferThis/, PlotCurve *);
  Differentiation(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  Differentiation(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  Differentiation(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);
  bool run();
};

class Integration : Filter
{
%TypeHeaderCode
#include "Integration.h"
%End
public:
  Integration(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1, bool = false);
  Integration(ApplicationWindow * app /TransferThis/, PlotCurve *);
  Integration(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);
  Integration(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  Integration(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);

  void enableGraphicsDisplay(bool on = true, Graph *g = 0);
  bool run();

  double area();
};

class Interpolation : Filter
{
%TypeHeaderCode
#include "Interpolation.h"
%End
public:
  enum InterpolationMethod{Linear, Cubic, Akima};
// 
  Interpolation(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1, int = 0);
  Interpolation(ApplicationWindow * app /TransferThis/, PlotCurve *, int=0);
  Interpolation(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double, int=0);
  Interpolation(ApplicationWindow * app /TransferThis/, Graph *, const QString&, int=0);
  Interpolation(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double, int=0);

  void setMethod(int n);
  bool run();
};

class SmoothFilter : Filter
{
%TypeHeaderCode
#include "SmoothFilter.h"
%End
public:
  enum SmoothMethod{SavitzkyGolay = 1, FFT = 2, Average = 3, Lowess = 4};
// 
  SmoothFilter(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1, int = 3);
  SmoothFilter(ApplicationWindow * app /TransferThis/, PlotCurve *, int = 3);
  SmoothFilter(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double, int = 3);
  SmoothFilter(ApplicationWindow * app /TransferThis/, Graph *, const QString&, int=3);
  SmoothFilter(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double, int=3);

  void setMethod(int);
  void setSmoothPoints(int, int = 0);
  void setPolynomOrder(int);
  void setLowessParameter(double f, int iterations);
  bool run();
};

class FFTFilter : Filter
{
%TypeHeaderCode
#include "FFTFilter.h"
%End
public:
  enum FilterType{LowPass = 1, HighPass = 2, BandPass = 3, BandBlock = 4};
// 
  FFTFilter(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1, int = 1);
  FFTFilter(ApplicationWindow * app /TransferThis/, PlotCurve *, int = 1);
  FFTFilter(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double, int = 1);
  FFTFilter(ApplicationWindow * app /TransferThis/, Graph *, const QString&, int=1);
  FFTFilter(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double, int=1);

  void setFilterType(int);
  void setCutoff(double);
  void setBand(double, double);
  void enableOffset(bool=true);

  bool run();
};

class FFT : Filter
{
%TypeHeaderCode
#include "FFT.h"
%End
public:
  FFT(ApplicationWindow * app /TransferThis/, Matrix *, Matrix * = NULL, bool = false, bool = true, bool = false, bool = true);
  FFT(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString& = QString(), int = 1, int = -1);
  FFT(ApplicationWindow * app /TransferThis/, Graph *, const QString&);
  FFT(ApplicationWindow * app /TransferThis/, Graph *, const QString&, double, double);
  FFT(ApplicationWindow * app /TransferThis/, PlotCurve *);
  FFT(ApplicationWindow * app /TransferThis/, PlotCurve *, double, double);

  void setInverseFFT(bool=true);
  void setSampling(double);
  void normalizeAmplitudes(bool=true);
  void shiftFrequencies(bool=true);

  Matrix *amplitudesMatrix();
  Matrix *realOutputMatrix();
  Matrix *imaginaryOutputMatrix();

  bool run();
};

class Correlation : Filter
{
%TypeHeaderCode
#include "Correlation.h"
%End
public:
  Correlation(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&, int = 1, int = -1);

  bool setDataFromTable(Table *, const QString&, const QString&, int = 1, int = -1);
  bool run();
};

class Convolution : Filter
{
%TypeHeaderCode
#include "Convolution.h"
%End
public:
  Convolution(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&);

  void setDataFromTable(Table *, const QString&, const QString&);
  bool run();
};

class Deconvolution : Filter
{
%TypeHeaderCode
#include "Convolution.h"
%End
public:
  Deconvolution(ApplicationWindow * app /TransferThis/, Table *, const QString&, const QString&);

  void setDataFromTable(Table *, const QString&, const QString&);
  bool run();
};

class Statistics : QObject
{
%TypeHeaderCode
#include <Statistics.h>
%End
public:
  Statistics(ApplicationWindow * app /TransferThis/, const QString& = QString());
  ~Statistics();

  void showResultsLog(bool = true);

  //! Returns the size of the input data set
  unsigned int dataSize();
  //! Returns the degrees of freedom
  virtual int dof();

  double mean();
  double variance();
  double standardDeviation();
  double standardError();

  QString logInfo(bool header = true);
  virtual bool run();
};

class StatisticTest : Statistics
{
%TypeHeaderCode
#include <StatisticTest.h>
%End
public:
  StatisticTest(ApplicationWindow * app /TransferThis/, double = 0.0, double = 0.05, const QString& = QString());
  enum Tail{Left = 0, Right = 1, Both = 2};

  void setTestValue(double);
  void setSignificanceLevel(double);
  void showDescriptiveStatistics(bool show = true);

  Table* resultTable(const QString& = QString());
  virtual QString logInfo();
  virtual double statistic();
  virtual bool run();
};

class tTest : StatisticTest
{
%TypeHeaderCode
#include <tTest.h>
%End
public:
  tTest(ApplicationWindow * app /TransferThis/, double, double, const QString& = QString(), const QString& = QString(), bool = false);

  bool setSample2(const QString&, bool = false);

  virtual QString logInfo();
  double t();
  double statistic();
  double pValue();
  int dof();
  double power(double alpha, int size = 0);
  //! Lower Confidence Limit
  double lcl(double);
  //! Upper Confidence Limit
  double ucl(double);
};

class ChiSquareTest : StatisticTest
{
%TypeHeaderCode
#include <ChiSquareTest.h>
%End
public:
  ChiSquareTest(ApplicationWindow * app /TransferThis/, double testValue, double level, const QString& = QString());

  virtual QString logInfo();
  double chiSquare();
  double statistic();
  double pValue();
};

class ShapiroWilkTest : StatisticTest
{
%TypeHeaderCode
#include <ShapiroWilkTest.h>
%End
public:
  ShapiroWilkTest(ApplicationWindow * app /TransferThis/, const QString& = QString());

  virtual QString logInfo();
  double w();
  double statistic();
  double pValue();
};

class Anova : StatisticTest
{
%TypeHeaderCode
#include <Anova.h>
%End
public:
  Anova(ApplicationWindow * app /TransferThis/, bool twoWay = false, double level = 0.05);

  bool run();
  bool addSample(const QString& colName, int aLevel = 1, int bLevel = 1);
  void setAnovaTwoWayModel(int);

  virtual QString logInfo();
  double fStat();
  double statistic();
  double pValue();
  //! Sum of squares
  double ssm();
  double sse();
  double sst();
  //! Mean square value
  double mse();
  //! F statistic for factor A
  double fStatA();
  //! F statistic for factor B
  double fStatB();
  //! F statistic for the interaction
  double fStatAB();
  //! P value for factor A
  double pValueA();
  //! P value for factor B
  double pValueB();
  //! P value for the interaction
  double pValueAB();
  //! Sum of squares for factor A
  double ssa();
  //! Sum of squares for factor B
  double ssb();
  //! Sum of squares for the interaction
  double ssab();
  //! Mean square value for factor A
  double msa();
  //! Mean square value for factor B
  double msb();
  //! Mean square value for the interaction
  double msab();
};

// used for output redirection
class PythonScripting
{
%TypeHeaderCode
#include "PythonScripting.h"
%End
public:
  PythonScripting(ApplicationWindow *parent);
  void write(const QString&);
  void flush();

private:
  PythonScripting(const PythonScripting &);
  PythonScripting& operator=(const PythonScripting &);
};
class PythonScript
{
%TypeHeaderCode
#include "PythonScript.h"
%End
public:
  PythonScript(PythonScripting *env, const QString &code, QObject *context=0, const QString &name="<input>");
  void write(const QString&);
  void flush();

private:
  PythonScript(const PythonScript &);
  PythonScript& operator=(const PythonScript &);
};

class Folder : QObject
{
%TypeHeaderCode
#include "Folder.h"
%End
public:
  Folder(Folder *parent, const QString &name);
  QList<MdiSubWindow*> windowsList() /PyName=windows/;
// TODO: implement signal Folder::nameChanged and make it update the project explorer; adjust renaming from GUI accordingly
//  void setFolderName(const QString&) /PyName=setName/;
  QString objectName() /PyName=name/;
  QString path();

  QList<Folder*> folders();
  Folder *findSubfolder(const QString&, bool=true, bool=false)  /PyName=folder/;
  MdiSubWindow* findWindow(const QString&, bool=true, bool=true, bool=false, bool=true);

  MdiSubWindow *window(const QString &name, const char *cls="MdiSubWindow", bool recursive=false);
  Table *table(const QString &name, bool recursive=false);
  Matrix *matrix(const QString &name, bool recursive=false);
  MultiLayer *graph(const QString &name, bool recursive=false);
  Note *note(const QString &name, bool recursive=false);
  Graph3D *plot3D(const QString &name, bool recursive=false);

  Folder* rootFolder();

  QString logInfo();

  void clearLogInfo();


};
