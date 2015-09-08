============================
QtiPlot - Installation Notes
============================
`Visit QtiPlot webpage <http://soft.proindependent.com/qtiplot.html>`_

License
-------
QtiPlot is distributed under the `GNU General Public License <http://www.gnu.org/licenses/gpl.html>`_. Thus it is `"free software" <http://www.fsf.org/licensing/essays/free-sw.html>`_. "Free software" is a matter of liberty, not price. To understand the concept, you should think of "free" as in "free speech", not as in "free beer". 
"Free software" is also often called `Open Source, FOSS, or FLOSS <http://en.wikipedia.org/wiki/Alternative_terms_for_free_software>`_.

General requirements
--------------------l
You need to install the following libraries:

- `Qt (>= 4.5.0) <http://www.trolltech.com/products/qt/index.html>`_,
- `GSL <http://sources.redhat.com/gsl/>`_,
- `muParser (1.32) <http://muparser.sourceforge.net/>`_,
- `zlib <http://www.zlib.net/>`_ and
- `libpng <http://www.libpng.org/pub/png/libpng.html>`_.

You also need to build and install the slightly modified versions of the 
`Qwt (5.2) <http://qwt.sourceforge.net/index.html>`_ and
`QwtPlot3D <http://qwtplot3d.sourceforge.net/>`_ libraries which are shipped 
with QtiPlot in the "3rdparty/qwt" and "3rdparty/qwtplot3d" folders respectively.

For the export of 2D plots to TeX, you need to download and install 
`QTeXEngine <http://soft.proindependent.com/qtexengine/>`_.

If you need to convert random grid XYZ datasets to matrices you must download 
and install
`ALGLIB (2.6) <http://www.alglib.net/>`_.

If you want to perform ANOVA statistic calculations you must download
`TAMUANOVA <http://www.stat.tamu.edu/~aredd/tamuanova/>`_.


If you need Python scripting support, don't forget to download and install 
`Python <http://www.python.org/>`_, 
`SIP <http://www.riverbankcomputing.co.uk/software/sip/download>`_ and 
`PyQt4 <http://www.riverbankcomputing.co.uk/software/pyqt/download>`_.

In order to build QtiPlot you also need *qmake*, which is distributed as a part of Qt.

Linux/Mac OS X
--------------

Binaries from maintenance contracts
-----------------------------------
Ion Vasilief offers supported QtiPlot binaries as `Debian <http://www.debian.org/>`_ packages (for Debian, Ubuntu, ...) as well as for Mac OS X (>= 10.4). 
The packages provide all needed libraries, except Python and Qt (on Linux), which must be installed separately on your system.

QtiPlot Linux binaries free of charge
-------------------------------------
The QtiPlot team offers GNU/Linux binaries free of charge but without any guaranteed support.

To use this service, you need to subscribe to a maintenance contract. 
You can find details about this `QtiPlot download page <http://soft.proindependent.com/download.html>`_.

To install the Debian package, download the file `qtiplot_X.X.X_i386.deb <http://soft.proindependent.com/download.html>`_ and login to your machine as "root" (on Ubuntu, login as a normal user and prepend <code>sudo</code>), then open a terminal and execute the following command:
      ``dpkg -i qtiplot_X.X.X_i386.deb``

To install the binary package on Mac OS X, download the file `qtiplot-X.X.X.pkg.zip <http://soft.proindependent.com/download.html>`_, unzip it, double-click on the "qtiplot-X.X.X.pkg" file and follow the installation instructions. 
Some time after each release, you will find these (including notes how to install them) on the
`QtiPlot download page <http://soft.proindependent.com/download.html>`_ or in the files section at the `QtiPlot project page at berliOS <https://developer.berlios.de/project/showfiles.php?group_id=6626>`_.

Compiling QtiPlot from source
-----------------------------

#. Install all the dependencies listed above either from your package manager or 
   from the sources. If you are using precompiled packages, make sure you also 
   install the corresponding development packages (*-dev or *-devel). If you want
   to display the manual from within QtiPlot (you can also view it with your 
   favorite browser), you also need the Qt Assistant, which sometimes comes in a
   separate package (qt4-dev-tools for Ubuntu edgy).
#. Open a terminal window.
#. Download the .tar.bz2 archive with QtiPlot's sources (or the .zip archive, which has the same contents):
   ``wget http://soft.proindependent.com/src/qtiplot-X.X.X.tar.bz2``
#. Unpack the source archive:
   ``tar -xvjf qtiplot-X.X.X.tar.bz2`` (or ``unzip qtiplot-X.X.X.zip``, respectively)
#. Go to the main directory qtiplot-X.X.X:
   ``cd qtiplot-X.X.X``
#. Create a build.conf file. You can use the provided example for this:
   ``cp build.conf.example build.conf``
#. Open build.conf in your favorite editor. Near the bottom of the same file, 
   either comment out the line ``SCRIPTING_LANGS += Python`` or make 
   sure you have the following additional dependencies installed:
   `Python <http://www.python.org/>`_,
   `SIP <http://www.riverbankcomputing.co.uk/software/sip/download>`_ and 
   `PyQt4 <http://www.riverbankcomputing.co.uk/software/pyqt/download>`_.
#. Build qtiplot:
   ``qmake && make``
#. Install QtiPlot on your system (you need root privileges):
   ``sudo make install``
   or, alternatively,
   ``su -c "make install"``

Windows
-------

Binaries from maintenance contracts
-----------------------------------

Ion Vasilief offers supported QtiPlot Windows binaries. In principle QtiPlot should work on any Windows version from 98 to Vista. But don't expect it to run perfectly on older versions than 2000. For platform specific issues of Qt4 based applications see `doc.trolltech.com <http://www.trolltech.com/developer/notes/platforms/index>`_.

To install:

#. Download the `qtiplot-X.X.X.exe <http://soft.proindependent.com/download.html>`_ file.
#. Open a Windows Explorer, double click on 'qtiplot-X.X.X.exe' and follow the installation instructions.


Compiling QtiPlot from source
-----------------------------
The compilation process requires the same steps as explained above in the Linux/Mac OS X section.

Forums and mailing lists
------------------------
If you want to discuss QtiPlot with other users (or the developers) or if you are insterested in its further development, you can go to the relevant `QtiPlot forum <https://developer.berlios.de/forum/?group_id=6626>`_ or subscribe to the appropriate `QtiPlot mailing list <https://developer.berlios.de/mail/?group_id=6626>`_.
