#!/usr/bin/python

############################################################################
#                                                                          #
# File                 : python-sipcmd.py                                  #
# Project              : QtiPlot                                           #
# Description          : Auto-detect SIP/PyQt location; dump commandline   #
# Copyright            : (C) 2007-2010 Knut Franke (knut.franke*gmx.de)    #
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

import sys
import os

# Since we don't have pyqtconfig anymore, we need to find sip executable.
# User has sip 6.x installed via pip.
# The executable should be sip-module or just sip if in path.
# For now, let's assume 'sip-module' is available as it was seen in Scripts.
sip_bin = "sip-module"
# We also need the include directory for PyQt/Qt if we were using it, 
# but for now let's just output a basic sip command.
print(sip_bin)
