############################################################################
#                                                                          #
# File                 : qtiplotrc.py                                     #
# Project              : QtiPlot                                           #
# Description          : default configuration file of QtiPlot' Python     #
#                        environment                                       #
# Copyright            : (C) 2006-2007 Knut Franke (knut.franke*gmx.de)    #
#                        (C) 2006-2010 Ion Vasilief (ion_vasilief*yahoo.fr)#
#                        (replace * with @ in the email address)           #
#                                                                          #
############################################################################
#                                                                          #
#  This program is free software; you can redistribute it and/or modify    #
#  it under the terms of the GNU General Public License as published by    #
#  the Free Software Foundation; either version 2 of the License, or       #
#  (at your option) any later version.                                     #
#                                                                          #
#  This program is distributed in the hope that it will be useful,         #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of          #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
#  GNU General Public License for more details.                            #
#                                                                          #
#   You should have received a copy of the GNU General Public License      #
#   along with this program; if not, write to the Free Software            #
#   Foundation, Inc., 51 Franklin Street, Fifth Floor,                     #
#   Boston, MA  02110-1301  USA                                            #
#                                                                          #
############################################################################

import __main__

def import_to_global(modname, attrs=None, math=False):
	"""
		import_to_global(modname, (a,b,c,...), math): like "from modname import a,b,c,...",
		but imports to global namespace (__main__).
		If math==True, also registers functions with QtiPlot's math function list.
	"""
	import sys
	import os
	sys.path.append(os.path.dirname(__file__))
	mod = __import__(modname)
	for submod in modname.split(".")[1:]:
		mod = getattr(mod, submod)
	if attrs==None: attrs=dir(mod)
	for name in attrs:
		f = getattr(mod, name)
		setattr(__main__, name, f)
		# make functions available in QtiPlot's math function list
		if math and callable(f): qti.mathFunctions[name] = f

# Import standard math functions and constants into global namespace.
import_to_global("math", None, True)

# Import selected parts of scipy.special (if available) into global namespace.
# See www.scipy.org for information on SciPy and how to get it.
have_scipy = False
try:
	special_functions = [
		# Airy Functions
		"airy", "airye", "ai_zeros", "bi_zeros",
		# Elliptic Functions and Integrals
		"ellipj", "ellipk", "ellipkinc", "ellipe", "ellipeinc",
		# Bessel Functions
		"jn", "jv", "jve",
		"yn", "yv", "yve",
		"kn", "kv", "kve",
		"iv", "ive",
		"hankel1", "hankel1e", "hankel2", "hankel2e",
		"lmbda",
		"jnjnp_zeros", "jnyn_zeros",
		"jn_zeros", "jnp_zeros",
		"yn_zeros", "ynp_zeros",
		"y0_zeros", "y1_zeros", "y1p_zeros",
		"j0", "j1", "y0", "y1", "i0", "i0e", "i1", "i1e", "k0", "k0e", "k1", "k1e",
		# Integrals of Bessel Functions
		"itj0y0", "it2j0y0", "iti0k0", "it2i0k0", "besselpoly",
		# Derivatives of Bessel Functions
		"jvp", "yvp", "kvp", "ivp", "h1vp", "h2vp",
		# Spherical Bessel Functions
		"sph_jn", "sph_yn", "sph_jnyn", "sph_in", "sph_kn", "sph_inkn",
		# Ricatti-Bessel Functions
		"riccati_jn", "riccati_yn",
		# Struve Functions
		"struve", "modstruve", "itstruve0", "it2struve0", "itmodstruve0",
		# Gamma and Related Functions
		"gamma", "gammaln", "gammainc", "gammaincinv", "gammaincc", "gammainccinv",
		"beta", "betaln", "betainc", "betaincinv",
		"psi", "rgamma", "polygamma",
		# Error Function and Fresnel Integrals
		"erf", "erfc", "erfinv", "erfcinv", "erf_zeros",
		"fresnel", "fresnel_zeros", "fresnelc_zeros", "fresnels_zeros", "modfresnelp", "modfresnelm",
		# Legendre Functions
		"lpn", "lqn", "lpmn", "lqmn", "lpmv", "sph_harm",
		# Orthogonal polynomials
		"legendre", "sh_legendre",
		"chebyt", "chebyu", "chebyc", "chebys", "sh_chebyt", "sh_chebyu",
		"jacobi", "sh_jacobi",
		"laguerre", "genlaguerre",
		"hermite", "hermitenorm",
		"gegenbauer",
		# HyperGeometric Functions
		"hyp2f1",
		"hyp1f1", "hyperu", "hyp0f1",
		"hyp2f0", "hyp1f2", "hyp3f0",
		# Parabolic Cylinder Functions
		"pbdv", "pbvv", "pbwa", "pbdv_seq", "pbvv_seq", "pbdn_seq",
		# Mathieu and related Functions (and derivatives)
		"mathieu_a", "mathieu_b", "mathieu_even_coef", "mathieu_odd_coef",
		"mathieu_cem", "mathieu_sem", "mathieu_modcem1", "mathieu_modcem2", "mathieu_modsem1", "mathieu_modsem2",
		# Spheroidal Wave Functions
		"pro_ang1", "pro_rad1", "pro_rad2",
		"obl_ang1", "obl_rad1", "obl_rad2",
		"pro_cv", "obl_cv", "pro_cv_seq", "obl_cv_seq",
		"pro_ang1_cv", "pro_rad1_cv", "pro_rad2_cv",
		"obl_ang1_cv", "obl_rad1_cv", "obl_rad2_cv",
		# Kelvin Functions
		"kelvin", "kelvin_zeros",
		"ber", "bei", "berp", "beip", "ker", "kei", "kerp", "keip",
		"ber_zeros", "bei_zeros", "berp_zeros", "beip_zeros", "ker_zeros", "kei_zeros", "kerp_zeros", "keip_zeros",
		# Other Special Functions
		"expn", "exp1", "expi",
		"wofz", "dawsn",
		"shichi", "sici", "spence",
		"zeta", "zetac",
		# Convenience Functions
		"cbrt", "exp10", "exp2",
		"radian", "cosdg", "sindg", "tandg", "cotdg",
		"log1p", "expm1", "cosm1",
		"round",
	]
	import_to_global("scipy.special", special_functions, True)
	have_scipy = True
	print("Loaded %d special functions from scipy.special." % len(special_functions))
except(ImportError): pass

# make Qt API available (it gets imported in any case by the qti module)
global QtGui
from PyQt4 import QtGui

global QtCore
from PyQt4 import QtCore

global Qt
from PyQt4.QtCore import Qt

# import QtiPlot's classes to the global namespace (particularly useful for fits)
from qti import *

# import selected methods of ApplicationWindow into the global namespace
appImports = (
	"table", "newTable",
	"matrix", "newMatrix",
	"graph", "newGraph",
	"note", "newNote",
	"newPlot3D",
	"tableToMatrix", "tableToMatrixRegularXYZ", "matrixToTable",
	"openTemplate", "saveAsTemplate",
	"clone", "setWindowName",
	"importImage", "importExcel", "importOdfSpreadsheet", "importWaveFile",
	"setPreferences",
	"plot", "plot3D", "stemPlot", "waterfallPlot", "plotImageProfiles",
	"activeFolder", "rootFolder",
	"addFolder", "deleteFolder", "changeFolder", "copyFolder",
	"saveFolder", "appendProject", "saveProjectAs",
	"currentTable","currentMatrix","currentGraph","currentNote",
	"resultsLog", "workspace", "displayInfo", "infoLineEdit"
	)
for name in appImports:
	setattr(__main__,name,getattr(qti.app,name))

# import utility module
import_to_global("qtiUtil", None, True)

# Provide easy access to SymPy, for symbolic mathematics
try:
	import_to_global("sympy", None, False)
	print "\nsympy module successfully imported: SymPy is a Python library for symbolic mathematics."
	print "If you are new to SymPy, start with the documentation: http://docs.sympy.org/\n"
except(ImportError): pass

# Provide easy access to R, for statistical computing.
# See http://www.r-project.org/ for information on R and how to get it.
# See http://rpy.sourceforge.net/rpy2.html for the Python/R bridge
try:
  from rpy2.robjects import r as R
  have_R = True
  # now create some convenience helper
  import rpy2.rlike.container
  import rpy2.robjects

  def tableToRDataFrame(t):
    "Create a R data frame from this table"
    # create an empty TaggedList and append values later
    tl = rpy2.rlike.container.TaggedList([])
    # cache row indices for later use
    for colName in  t.colNames():
      values = t.colData(colName)
      tl.append(rpy2.robjects.FloatVector(values), tag=colName)
    # build an R dataframe
    return rpy2.robjects.RDataFrame(tl)
  # make it also avilable as a table instance method
  setattr(Table, "toRDataFrame", tableToRDataFrame)

  def newTableFromRDataFrame(df, name = "R result"):
    "create a new table from a R data frame"
    t = newTable(name, df.nrow(), df.ncol())
    t.setColNames(df.colnames())
    for i, coldata in enumerate(df):
      t.setColData(i+1, coldata)
    return t
  # make it also avilable as a function of qti.app
  setattr(app, "newTableFromRDataFrame", newTableFromRDataFrame)

  print "R support successfully set up"
except(ImportError): pass

print "Python scripting engine is ready."
