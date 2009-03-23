##########################################################################
#  File                 : strd_nist_fit.py
#  Project              : QtiPlot
#  --------------------------------------------------------------------
#  Copyright            : (C) 2008 by Ion Vasilief
#  Email 		: ion_vasilief At yahoo.fr
#  Description          : Example of nonlinear curve fitting in QtiPlot. 
#			  Uses data from the Statistical Reference Datasets Project of 
#			  the National Institute of Standards and Technology (NIST).
#
#  Usage		: qtiplot -x strd_nist_fit.py
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
import urllib, re, sys

# Pop-up a file dialog allowing to chose a destination folder:
dirPath = QtGui.QFileDialog.getExistingDirectory(qti.app, "Choose Destination Folder")

saveout = sys.stdout
# create a log file in the destination folder                                    
fsock = open(dirPath + "/" + "results.txt", "w")                             
sys.stdout = fsock   

# on Unix systems you can redirect the output directly to a console by uncommenting the line bellow:
#sys.stdout = sys.__stdout__ 

# make sure that the decimal separator is the dot character 
qti.app.setLocale(QtCore.QLocale.c())

host = "http://www.itl.nist.gov/div898/strd/nls/data/LINKS/DATA/"
url = urllib.urlopen(host)
url_string = url.read()
p = re.compile( '\w{,}.dat">' )
iterator = p.finditer( url_string )
for m in iterator:
	name = (m.group()).replace("\">", "")
	if (name == "Nelson.dat"):
		continue
		
	url = host + name
	print  "\nRetrieving file: " + url 
	path = dirPath + "/" + name
	urllib.urlretrieve( url, path ) # retrieve .dat file to specified location
				
	file = QtCore.QFile(path)	
	if file.open(QtCore.QIODevice.ReadOnly):
		ts = QtCore.QTextStream(file)
		name = name.replace(".dat", "")
		changeFolder(addFolder(name)) #create a new folder and move to it
		formula = ""
		parameters = 0
		initValues = list()
		certifiedValues = list()
		standardDevValues = list()
		xLabel = "X"
		yLabel = "Y"
					
		while (ts.atEnd() == False):
			s = ts.readLine().simplified()
				
			if (s.contains("(y = ")):
				lst = s.split("=")
				yLabel = lst[1].remove(")")
					
			if (s.contains("(x = ")):
				lst = s.split("=")
				xLabel = lst[1].remove(")")
				
			if (s.contains("Model:")):
				s = ts.readLine().simplified()
				lst = s.split(QtCore.QRegExp("\\s"))
				s = lst[0]
				parameters = s.toInt()[0]
				ts.readLine()
				if (name == "Roszman1"):
					ts.readLine() 
					formula = ts.readLine().simplified()
				else:
					formula = (ts.readLine() + ts.readLine() + ts.readLine()).simplified()
				formula.remove("+ e").remove("y =").replace("[", "(").replace("]", ")")
				formula.replace("**", "^").replace("arctan", "atan")
			
			if (s.contains("Starting")):
				ts.readLine()
				ts.readLine()
				for i in range (1, parameters + 1):
					s = ts.readLine().simplified()
					lst = s.split(" = ")
					s = lst[1].simplified()
					lst = s.split(QtCore.QRegExp("\\s"))
					initValues.append(lst[1])
					certifiedValues.append(lst[2])
					standardDevValues.append(lst[3])
				
			if (s.contains("Data: y")):
				row = 0
				t = newTable(name, 300, 2)
				t.setColName(1, "y")
				t.setColumnRole(1, Table.Y)
				t.setColName(2, "x")
				t.setColumnRole(2, Table.X)
				while (ts.atEnd() == False):
					row = row + 1
					s = ts.readLine().simplified()
					lst = s.split(QtCore.QRegExp("\\s"))
					t.setText(1, row, lst[0])
					t.setText(2, row, lst[1])
				
				g = plot(t, t.colName(1), Layer.Scatter).activeLayer()
				g.setTitle("Data set: " + name + ".dat")
				g.setAxisTitle(Layer.Bottom, xLabel)
				g.setAxisTitle(Layer.Left, yLabel)
				
				f = NonLinearFit(g, name + "_" + t.colName(1))
				if (f.setFormula(formula) == False) :
					file.close()
					changeFolder(rootFolder())
					continue
					
				f.scaleErrors()
				for i in range (0, parameters):
					f.setInitialValue(i, initValues[i].toDouble()[0])
				f.fit()
				g.removeLegend()
				f.showLegend()
				print  "QtiPlot Results:\n" + f.legendInfo().toAscii()
				
				print  "\nCertified Values:"
				paramNames = f.parameterNames()
				for i in range (0, parameters):
					print  '%s = %s +/- %s' % (paramNames[i], certifiedValues[i], standardDevValues[i])

				print  "\nDifference with QtiPlot results:"
				results = f.results()
				for i in range (0, parameters):
					diff = fabs(results[i] - certifiedValues[i].toDouble()[0])
					print  'db%d = %6g' % (i+1, diff)
					
				file.close()
				changeFolder(rootFolder())
		
newNote("ResultsLog").importASCII(dirPath + "/" + "results.txt")		
saveProjectAs(dirPath + "/" + "StRD_NIST.qti")	
sys.stdout = saveout                                     
fsock.close()
