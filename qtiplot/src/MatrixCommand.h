/***************************************************************************
    File                 : MatrixCommand.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief,
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Matrix undo/redo commands

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
#ifndef MATRIX_COMMAND_H
#define MATRIX_COMMAND_H

#include "Matrix.h"
#include "MatrixModel.h"
#include <QUndoCommand>

//! Matrix commands used by the undo/redo framework
class MatrixCommand: public QUndoCommand
{
public:
    MatrixCommand(MatrixModel *modelBefore, MatrixModel *modelAfter, const QString & text);

    virtual void redo();
    virtual void undo();

private:
    MatrixModel *d_model_before;
    MatrixModel *d_model_after;
};

class MatrixEditCellCommand: public QUndoCommand
{
public:
    MatrixEditCellCommand(MatrixModel *model, const QModelIndex & index, double valBefore,
                            double valAfter, const QString & text);
    virtual void redo();
    virtual void undo();

private:
    MatrixModel *d_model;
    QModelIndex d_index;
    double d_val_before;
    double d_val_after;
};

class MatrixSetFormulaCommand: public QUndoCommand
{
public:
    MatrixSetFormulaCommand(Matrix *m, const QString& oldFormula, const QString& newFormula, const QString & text);    
    virtual void redo();
    virtual void undo();

private:
    Matrix *d_matrix;
    QString d_old_formula;
    QString d_new_formula;
};

class MatrixSetViewCommand: public QUndoCommand
{
public:
    MatrixSetViewCommand(Matrix *m, Matrix::ViewType oldView, Matrix::ViewType newView, const QString & text);
    virtual void redo();
    virtual void undo();

private:
    Matrix *d_matrix;
    Matrix::ViewType d_old_view;
    Matrix::ViewType d_new_view;
};

class MatrixSetHeaderViewCommand: public QUndoCommand
{
public:
    MatrixSetHeaderViewCommand(Matrix *m, Matrix::HeaderViewType oldView,
                            Matrix::HeaderViewType newView, const QString & text);
    virtual void redo();
    virtual void undo();

private:
    Matrix *d_matrix;
    Matrix::HeaderViewType d_old_view;
    Matrix::HeaderViewType d_new_view;
};

class MatrixSetColWidthCommand: public QUndoCommand
{
public:
    MatrixSetColWidthCommand(Matrix *m, int oldWidth, int newWidth, const QString & text);
    virtual void redo();
    virtual void undo();

private:
    Matrix *d_matrix;
    int d_old_width;
    int d_new_width;
};

class MatrixSetPrecisionCommand: public QUndoCommand
{
public:
    MatrixSetPrecisionCommand(Matrix *m, const QChar& oldFormat, const QChar& newFormat,
                    int oldPrec, int newPrec, const QString & text);
    virtual void redo();
    virtual void undo();

private:
    Matrix *d_matrix;
    QChar d_old_format;
    QChar d_new_format;
    int d_old_prec;
    int d_new_prec;
};

class MatrixSetCoordinatesCommand: public QUndoCommand
{
public:
    MatrixSetCoordinatesCommand(Matrix *, double, double, double, double,
                    double, double, double, double, const QString &);
    virtual void redo();
    virtual void undo();

private:
    Matrix *d_matrix;
    double d_old_xs, d_old_xe, d_old_ys, d_old_ye;
    double d_new_xs, d_new_xe, d_new_ys, d_new_ye;
};

class MatrixSetColorMapCommand: public QUndoCommand
{
public:
    MatrixSetColorMapCommand(Matrix *m, Matrix::ColorMapType d_map_type_before, 
					const QwtLinearColorMap& d_map_before, Matrix::ColorMapType d_map_type_after, 
					const QwtLinearColorMap& d_map_after, const QString& text);
    virtual void redo();
    virtual void undo();

private:
    Matrix *d_matrix;
    Matrix::ColorMapType d_map_type_before, d_map_type_after;
    QwtLinearColorMap d_map_before, d_map_after;
};
#endif
