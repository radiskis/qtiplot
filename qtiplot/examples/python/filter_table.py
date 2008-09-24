##########################################################################
#  File                 : filter_table.py
#  Project              : QtiPlot
#  --------------------------------------------------------------------
#  Copyright            : (C) 2008 by Luyang Han
#  Email 		: 
#  Description          : Filtering table values. 
#
#  Usage		: Add this plugin to the 'Table' menu using 
#			 'Scripting | Add custom script action...'. 
##########################################################################

##########################################################################
#                                                                        #
#  This program is free software; you can redistribute it and/or modify  #
#  it under the terms of the GNU General Public License as published by  #
#  the Free Software Foundation; either version 2 of the License, or     #
#  (at your option) any later version.                                   #
#                                                                        #
#  This program is distributed in the hope that it will be useful,       #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#  GNU General Public License for more details.                          #
#                                                                        #
#  You should have received a copy of the GNU General Public License     #
#  along with this program; if not, write to the Free Software           #
#  Foundation, Inc., 51 Franklin Street, Fifth Floor,                    #
#  Boston, MA  02110-1301  USA                                           #
#                                                                        #
##########################################################################

def filter_table(t, filter_str):
    num_row = t.numRows()
    delete_list = []
    for i in range(1, num_row+1):
        col = lambda x: t.cell(x, i)
        res = bool(eval(filter_str))
        if not res:
            delete_list.append(i)
    for i, row in enumerate(delete_list):
        row_index = row - i
        t.deleteRows(row_index, row_index)

t = currentTable()
filter_str, isOK = QtGui.QInputDialog.getText(qti.app, "Row filter", "Input the python expression to filter current table.\nUse col(col_name) to access one column. \nIf the expression yields false value, the corresponding row will be deleted.", QtGui.QLineEdit.Normal, 'col("1") > 0')
if isOK:
    try:
        filter_table(t, str(filter_str))
    except Exception, x:
        QtGui.QMessageBox.warning(qti.app, 'Filtering failed!', x.__str__())
