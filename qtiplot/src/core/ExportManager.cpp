/***************************************************************************
    File                 : ExportManager.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2004 - 2012 by Ion Vasilief,
                           (C) 2006 - june 2007 Tilman Hoener zu Siederdissen, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Plot, table, and document export and print manager

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                   *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                            *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                            *
 *                                                                         *
 ***************************************************************************/

#include "ExportManager.h"
#include "ApplicationWindow.h"
#include "Table.h"
#include "Matrix.h"
#include "Graph.h"
#include "MultiLayer.h"
#include "Note.h"
#include "Graph3D.h"
#include "PolarGraph.h"
#include "ExportDialog.h"
#include "ImageExportDialog.h"
#include "ImportExportPlugin.h"
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QSvgGenerator>
#include <QTextDocumentWriter>
#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDateTime>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QImageWriter>

ExportManager::ExportManager(ApplicationWindow *app)
    : QObject(app), d_app(app)
{
}

ExportManager::~ExportManager()
{
}

void ExportManager::exportMatrix(const QString& exportFilter)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_extended_export_dialog = d_app->d_extended_export_dialog;
	auto &d_image_export_filter = d_app->d_image_export_filter;
	auto &imagesDirPath = d_app->imagesDirPath;

	Matrix* m = (Matrix*)d_app->activeWindow(ApplicationWindow::MatrixWindow);
	if (!m)
		return;

	ImageExportDialog *ied = new ImageExportDialog(m, d_app, d_extended_export_dialog);
	ied->setDirectory(imagesDirPath);
	ied->selectFile(m->objectName());
	if (exportFilter.isEmpty())
    	ied->selectFilter(d_image_export_filter);
	else
		ied->selectFilter(exportFilter);

	if ( ied->exec() != QDialog::Accepted )
		return;
	imagesDirPath = ied->directory().path();

	QString selected_filter = ied->selectedNameFilter().remove("*");
	QString file_name = ied->selectedFiles()[0];
	if(!file_name.endsWith(selected_filter, Qt::CaseInsensitive))
		file_name.append(selected_filter);

	if (selected_filter.contains(".eps") || selected_filter.contains(".pdf") || selected_filter.contains(".ps"))
		m->exportVector(file_name, ied->vectorResolution(), ied->color());
	else if (selected_filter.contains(".svg"))
		m->exportSVG(file_name);
	else if (selected_filter.contains(".emf"))
		m->exportEMF(file_name);
	else if (selected_filter.contains(".odf"))
		m->exportRasterImage(file_name, ied->quality(), ied->bitmapResolution());
	else {
		QList<QByteArray> list = QImageWriter::supportedImageFormats();
		for (int i = 0; i < list.count(); i++){
			if (selected_filter.contains("." + list[i].toLower()))
				m->exportRasterImage(file_name, ied->quality(), ied->bitmapResolution(), ied->compression());
		}
	}
}

void ExportManager::exportExcel()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	ImportExportPlugin *ep = d_app->exportPlugin("xls");
	if (!ep)
		return;

	ExportDialog *ed = d_app->showExportASCIIDialog();
	if (ed){
		ed->setWindowTitle(d_app->tr("Export Excel"));
		ed->setNameFilters(QStringList() << "*.xls");
		ed->updateAdvancedOptions(".xls");
	}
}

void ExportManager::exportOds()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	ImportExportPlugin *ep = d_app->exportPlugin("ods");
	if (!ep)
		return;

	ExportDialog *ed = d_app->showExportASCIIDialog();
	if (ed){
		ed->setWindowTitle(d_app->tr("Export Open Document Spreadsheet"));
		ed->setNameFilters(QStringList() << "*.ods");
		ed->updateAdvancedOptions(".ods");
	}
}

void ExportManager::exportGraph(const QString& exportFilter)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_extended_export_dialog = d_app->d_extended_export_dialog;
	auto &d_image_export_filter = d_app->d_image_export_filter;
	auto &imagesDirPath = d_app->imagesDirPath;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	MultiLayer *plot2D = qobject_cast<MultiLayer *>(w);
	Graph3D *plot3D = qobject_cast<Graph3D *>(w);
	PolarGraph *plotPolar = qobject_cast<PolarGraph *>(w);
	if(plot2D && plot2D->isEmpty()){
		QMessageBox::critical(d_app, d_app->tr("QtiPlot - Export Error"),
					d_app->tr("<h4>There are no plot layers available in d_app window!</h4>"));
		return;
	}

	if (!plot2D && !plot3D && !plotPolar)
		return;

	ImageExportDialog *ied = new ImageExportDialog(w, d_app, d_extended_export_dialog);
	ied->setDirectory(imagesDirPath);
	ied->selectFile(w->objectName());
    if (exportFilter.isEmpty())
    	ied->selectFilter(d_image_export_filter);
	else
		ied->selectFilter(exportFilter);

	if ( ied->exec() != QDialog::Accepted )
		return;
	imagesDirPath = ied->directory().path();

	QString selected_filter = ied->selectedNameFilter().remove("*");
	QString file_name = ied->selectedFiles()[0];
	if(!file_name.endsWith(selected_filter, Qt::CaseInsensitive))
		file_name.append(selected_filter);

    if (plot3D && selected_filter.contains(".pgf")){
        plot3D->exportVector(file_name, ied->textExportMode(), ied->sortMode());
        return;
    }

    if (plot2D && selected_filter.contains(".emf")){
		plot2D->exportEMF(file_name, ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		return;
    }
    
    if (plotPolar && selected_filter.contains(".emf")){
        // Polar plots don't support EMF yet, but we could add it if there's a plugin
        return;
    }

#ifdef TEX_OUTPUT
	if (plot2D && selected_filter.contains(".tex")){
		plot2D->exportTeX(file_name, ied->color(), ied->escapeStrings(), ied->exportFontSizes(), ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		return;
	}
#endif

	if (selected_filter.contains(".eps") || selected_filter.contains(".pdf") ||
		selected_filter.contains(".ps") || selected_filter.contains(".svg")) {
		if (plot3D)
			plot3D->exportVector(file_name, ied->textExportMode(), ied->sortMode(),
					ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		else if (plot2D){
			if (selected_filter.contains(".svg"))
				plot2D->exportSVG(file_name, ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
			else
				plot2D->exportVector(file_name, ied->vectorResolution(), ied->color(),
						ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		} else if (plotPolar){
            if (selected_filter.contains(".svg"))
                plotPolar->exportSVG(file_name, ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
            else
                plotPolar->exportVector(file_name, ied->vectorResolution(), ied->color(),
                        ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		}
	} else if (selected_filter.contains(".odf")){
		if (plot2D)
			plot2D->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
					ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		else if (plot3D)
			plot3D->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
					ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		else if (plotPolar)
			plotPolar->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
					ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());

	} else {
		QList<QByteArray> list = QImageWriter::supportedImageFormats();
		for (int i = 0; i < list.count(); i++){
			if (selected_filter.contains("." + (list[i]).toLower())){
				if (plot2D)
					plot2D->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
							ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor(), ied->compression());
				else if (plot3D){
					plot3D->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
						ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor(), ied->compression());
				} else if (plotPolar){
					plotPolar->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
						ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor(), ied->compression());
				}
			}
		}
	}
}

void ExportManager::exportLayer()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_extended_export_dialog = d_app->d_extended_export_dialog;
	auto &d_image_export_filter = d_app->d_image_export_filter;
	auto &imagesDirPath = d_app->imagesDirPath;

	MdiSubWindow *w = d_app->activeWindow(ApplicationWindow::MultiLayerWindow);
	if (!w)
		return;

	Graph* g = ((MultiLayer*)w)->activeLayer();
	if (!g)
		return;

	ImageExportDialog *ied = new ImageExportDialog(w, d_app, d_extended_export_dialog, g);
	ied->setDirectory(imagesDirPath);
	ied->selectFile(w->objectName());
	ied->selectFilter(d_image_export_filter);
	if ( ied->exec() != QDialog::Accepted )
		return;
	imagesDirPath = ied->directory().path();

	QString file_name = ied->selectedFiles()[0];
	QString selected_filter = ied->selectedNameFilter().remove("*");
	if(!file_name.endsWith(selected_filter, Qt::CaseInsensitive))
		file_name.append(selected_filter);

	if (selected_filter.contains(".eps") || selected_filter.contains(".pdf") || selected_filter.contains(".ps"))
		g->exportVector(file_name, ied->vectorResolution(), ied->color(),
			ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
	else if (selected_filter.contains(".svg"))
		g->exportSVG(file_name, ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
	else if (selected_filter.contains(".emf"))
		g->exportEMF(file_name, ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
#ifdef TEX_OUTPUT
	else if (selected_filter.contains(".tex"))
		g->exportTeX(file_name, ied->color(), ied->escapeStrings(), ied->exportFontSizes(), ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
#endif
    else if (selected_filter.contains(".odf"))
		g->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
						ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
    else {
		QList<QByteArray> list = QImageWriter::supportedImageFormats();
		for (int i = 0; i < list.count(); i++){
			if (selected_filter.contains("." + (list[i]).toLower()))
				g->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
							ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor(), ied->compression());
		}
	}
}

void ExportManager::exportPresentationODF()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_extended_export_dialog = d_app->d_extended_export_dialog;
	auto &imagesDirPath = d_app->imagesDirPath;

	ImageExportDialog *ied = new ImageExportDialog(nullptr, d_app, d_extended_export_dialog);
	ied->setDirectory(imagesDirPath);
	ied->setNameFilter("*.odf");

	if ( ied->exec() != QDialog::Accepted )
		return;
	imagesDirPath = ied->directory().path();

	QString selected_filter = ied->selectedNameFilter().remove("*");
	QString file_name = ied->selectedFiles()[0];
	if(!file_name.endsWith(selected_filter, Qt::CaseInsensitive))
		file_name.append(selected_filter);

	QDialog *previewDlg = new QDialog(d_app);
	previewDlg->setSizeGripEnabled(true);
	previewDlg->setWindowTitle(d_app->tr("QtiPlot") + " - " + d_app->tr("Presentation Preview"));
	previewDlg->resize(QSize(600, 400));

	QHBoxLayout *bl = new QHBoxLayout();
	bl->addStretch();
	QPushButton *okBtn = new QPushButton(d_app->tr("&Save"));
	connect(okBtn, &QPushButton::clicked, previewDlg, &QDialog::accept);
	bl->addWidget(okBtn);

	QPushButton *cancelBtn = new QPushButton(d_app->tr("&Cancel"));
	connect(cancelBtn, &QPushButton::clicked, previewDlg, &QDialog::reject);
	bl->addWidget(cancelBtn);
	bl->addStretch();

	QVBoxLayout *vl = new QVBoxLayout(previewDlg);
	QTextEdit *te = new QTextEdit();
	vl->addWidget(te);
	vl->addLayout(bl);

	QTextDocument *document = te->document();

	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		if (qobject_cast<MultiLayer*>(w)){
			MultiLayer *plot2D = qobject_cast<MultiLayer*>(w);
			if (!plot2D->isEmpty())
				plot2D->exportImage(document, ied->quality(), ied->transparency(), ied->bitmapResolution(),
						ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		} else if (qobject_cast<Graph3D*>(w))
			((Graph3D *)w)->exportImage(document, ied->quality(), ied->transparency(), ied->bitmapResolution(),
						ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
	}

	if (previewDlg->exec() == QDialog::Accepted){
		QTextDocumentWriter writer(file_name);
		writer.write(document);
	}
}

void ExportManager::exportAllGraphs()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_confirm_overwrite = d_app->d_confirm_overwrite;
	auto &d_extended_export_dialog = d_app->d_extended_export_dialog;
	auto &d_image_export_filter = d_app->d_image_export_filter;
	auto &format = d_app->format;
	auto &imagesDirPath = d_app->imagesDirPath;
	auto &right = d_app->right;

	ImageExportDialog *ied = new ImageExportDialog(nullptr, d_app, d_extended_export_dialog);
	ied->setWindowTitle(d_app->tr("Choose a directory to export the graphs to"));
	QStringList tmp = ied->nameFilters();
	ied->setFileMode(QFileDialog::Directory);
	ied->setNameFilters(tmp);
	ied->setLabelText(QFileDialog::FileType, d_app->tr("Output format:"));
	ied->setLabelText(QFileDialog::FileName, d_app->tr("Directory:"));
	ied->setDirectory(imagesDirPath);
    ied->selectFilter(d_image_export_filter);

	if ( ied->exec() != QDialog::Accepted )
		return;
	imagesDirPath = ied->directory().path();
	if (ied->selectedFiles().isEmpty())
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QString output_dir = ied->selectedFiles()[0];
	QString file_suffix = ied->selectedNameFilter();
	file_suffix = file_suffix.toLower();
	file_suffix.remove("*");

	bool confirm_overwrite = d_confirm_overwrite;
	MultiLayer *plot2D;
	Graph3D *plot3D;

	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("MultiLayer")) {
			plot3D = 0;
			plot2D = (MultiLayer *)w;
			if (plot2D->isEmpty()) {
				QApplication::restoreOverrideCursor();
				QMessageBox::warning(d_app, d_app->tr("QtiPlot - Warning"),
						d_app->tr("There are no plot layers available in window <b>%1</b>.<br>"
							"Graph window not exported!").arg(plot2D->objectName()));
				QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
				continue;
			}
		} else if (w->inherits("Graph3D")) {
			plot2D = 0;
			plot3D = (Graph3D *)w;
		} else
			continue;

		QString file_name = output_dir + "/" + w->objectName() + file_suffix;
		QFile f(file_name);
		if (f.exists() && confirm_overwrite) {
			QApplication::restoreOverrideCursor();

			QString msg = d_app->tr("A file called: <p><b>%1</b><p>already exists. ""Do you want to overwrite it?").arg(file_name);
			QMessageBox msgBox(QMessageBox::Question, d_app->tr("QtiPlot - Overwrite file?"), msg,
							  QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::Cancel,
							  (ApplicationWindow *)d_app);
 			msgBox.exec();
			switch(msgBox.standardButton(msgBox.clickedButton())){
				case QMessageBox::Yes:
					QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
				break;
				case QMessageBox::YesToAll:
					confirm_overwrite = false;
				break;
				case QMessageBox::No:
					confirm_overwrite = true;
					continue;
				break;
				case QMessageBox::Cancel:
					return;
				break;
				default:
					break;
			}
		}
		if ( !f.open( QIODevice::WriteOnly ) ) {
			QApplication::restoreOverrideCursor();
			QMessageBox::critical(d_app, d_app->tr("QtiPlot - Export error"),
					d_app->tr("Could not write to file: <br><h4>%1</h4><p>"
						"Please verify that you have the right to write to d_app location!").arg(file_name));
			return;
		}
		f.close();

	if (plot2D && file_suffix.contains(".emf")){
		plot2D->exportEMF(file_name, ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		return;
	}

#ifdef TEX_OUTPUT
	if (plot2D && file_suffix.contains(".tex")){
		plot2D->exportTeX(file_name, ied->color(), ied->escapeStrings(), ied->exportFontSizes(), ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
		return;
	}
#endif

		if (file_suffix.contains(".eps") || file_suffix.contains(".pdf") ||
			file_suffix.contains(".ps") || file_suffix.contains(".svg")) {
			if (plot3D)
				plot3D->exportVector(file_name, ied->textExportMode(), ied->sortMode(),
					ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
			else if (plot2D){
				if (file_suffix.contains(".svg"))
					plot2D->exportSVG(file_name, ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
				else
					plot2D->exportVector(file_name, ied->vectorResolution(), ied->color(),
							ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor());
			}
		} else {
			QList<QByteArray> list = QImageWriter::supportedImageFormats();
			for (int i = 0; i < list.count(); i++){
				if (file_suffix.contains("." + (list[i]).toLower())) {
					if (plot2D)
						plot2D->exportImage(file_name, ied->quality(), ied->transparency(),
						ied->bitmapResolution(), ied->customExportSize(), ied->sizeUnit(),
						ied->scaleFontsFactor(), ied->compression());
					else if (plot3D)
						plot3D->exportImage(file_name, ied->quality(), ied->transparency(), ied->bitmapResolution(),
							ied->customExportSize(), ied->sizeUnit(), ied->scaleFontsFactor(), ied->compression());
				}
			}
		}
	}
	QApplication::restoreOverrideCursor();
}

ExportDialog* ExportManager::showExportASCIIDialog()
{

	if (!d_app) return nullptr;
	ApplicationWindow *app = d_app;

    MdiSubWindow* t = d_app->activeWindow();
    if (!t)
		return 0;
	if (!qobject_cast<Matrix*>(t) && !t->inherits("Table"))
		return 0;

    ExportDialog* ed = new ExportDialog(t, d_app, true);
	ed->open();
	return ed;
}

void ExportManager::exportAllTables(const QString& dir, const QString& filter, const QString& sep, bool colNames, bool colComments, bool expSelection)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_confirm_overwrite = d_app->d_confirm_overwrite;
	auto &workingDir = d_app->workingDir;

	if (dir.isEmpty())
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	workingDir = dir;

	bool confirmOverwrite = d_confirm_overwrite;
	bool success = true;
	QList<MdiSubWindow *> windows = d_app->windowsList();
	for (MdiSubWindow *w : windows){
		if (w->inherits("Table") || w->inherits("Matrix")){
			QString fileName = dir + "/" + w->objectName() + filter;
			QFile f(fileName);
			if (f.exists(fileName) && confirmOverwrite){
				QApplication::restoreOverrideCursor();
				switch(QMessageBox::question(d_app, d_app->tr("QtiPlot - Overwrite file?"),
							d_app->tr("A file called: <p><b>%1</b><p>already exists. "
								"Do you want to overwrite it?").arg(fileName), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::Cancel, QMessageBox::Yes))
				{
					case QMessageBox::Yes:
						if (w->inherits("Table"))
							success = ((Table*)w)->exportASCII(fileName, sep, colNames, colComments, expSelection);
						else if (w->inherits("Matrix"))
							success = ((Matrix*)w)->exportASCII(fileName, sep, expSelection);
						break;

					case QMessageBox::YesToAll:
						confirmOverwrite = false;
						if (w->inherits("Table"))
							success = ((Table*)w)->exportASCII(fileName, sep, colNames, colComments, expSelection);
						else if (w->inherits("Matrix"))
							success = ((Matrix*)w)->exportASCII(fileName, sep, expSelection);
						break;

					case QMessageBox::Cancel:
						return;
						break;
				}
			} else if (w->inherits("Table"))
				success = ((Table*)w)->exportASCII(fileName, sep, colNames, colComments, expSelection);
			  else if (w->inherits("Matrix"))
				success = ((Matrix*)w)->exportASCII(fileName, sep, expSelection);

			if (!success)
				break;
		}
	}
	QApplication::restoreOverrideCursor();
}

void ExportManager::exportPDF()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_confirm_overwrite = d_app->d_confirm_overwrite;
	auto &imagesDirPath = d_app->imagesDirPath;
	auto &right = d_app->right;

	MdiSubWindow *w = d_app->activeWindow();
	if (!w)
		return;

	if(qobject_cast<MultiLayer *>(w) && ((MultiLayer *)w)->isEmpty()){
		QMessageBox::warning(d_app,d_app->tr("QtiPlot - Warning"),
			d_app->tr("<h4>There are no plot layers available in d_app window.</h4>"));
		return;
	}

	if (qobject_cast<MultiLayer *>(w) || qobject_cast<Graph3D *>(w) || qobject_cast<PolarGraph *>(w)){
		d_app->exportGraph("*.pdf");
		return;
	} else if (qobject_cast<Matrix *>(w)){
		d_app->exportMatrix("*.pdf");
		return;
	}

    QString fname = d_app->getFileName(d_app, d_app->tr("Choose a filename to save under"),
					imagesDirPath + "/" + w->objectName(), "*.pdf", 0, true, d_confirm_overwrite);
	if (!fname.isEmpty() ){
		QFileInfo fi(fname);
		QString baseName = fi.fileName();
		if (!baseName.contains("."))
			fname.append(".pdf");

        imagesDirPath = fi.absolutePath();

        QFile f(fname);
        if (!f.open(QIODevice::WriteOnly)){
            QMessageBox::critical(d_app, d_app->tr("QtiPlot - Export error"),
            d_app->tr("Could not write to file: <h4>%1</h4><p>Please verify that you have the right to write to d_app location or that the file is not being used by another application!").arg(fname));
            return;
        }

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        w->exportPDF(fname);
		QApplication::restoreOverrideCursor();
	}
}

void ExportManager::print()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	MdiSubWindow* w = d_app->activeWindow();
	if (!w)
		return;

    if (w->inherits("MultiLayer") && ((MultiLayer *)w)->isEmpty()){
		QMessageBox::warning(d_app,d_app->tr("QtiPlot - Warning"),
				d_app->tr("<h4>There are no plot layers available in d_app window.</h4>"));
		return;
	}
	w->print();
}

void ExportManager::printPreview()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_print_paper_size = d_app->d_print_paper_size;
	auto &d_printer_orientation = d_app->d_printer_orientation;

	MdiSubWindow* w = d_app->activeWindow();
	if (!w)
		return;

	if (w->inherits("MultiLayer") && ((MultiLayer *)w)->isEmpty()){
		QMessageBox::warning(d_app,d_app->tr("QtiPlot - Warning"),
				d_app->tr("<h4>There are no plot layers available in d_app window.</h4>"));
		return;
	}

	QPrinter p;
	p.setPageSize(QPageSize((QPageSize::PageSizeId)d_print_paper_size));
	p.setPageOrientation((QPageLayout::Orientation)d_printer_orientation);

	QPrintPreviewDialog *preview = new QPrintPreviewDialog(&p, d_app, Qt::Window);
	preview->setWindowTitle(d_app->tr("QtiPlot") + " - " + d_app->tr("Print preview of window: ") + w->objectName());
	connect(preview, &QPrintPreviewDialog::paintRequested, w, qOverload<QPrinter*>(&MdiSubWindow::print));
	connect(preview, &QPrintPreviewDialog::paintRequested, d_app, &ApplicationWindow::setPrintPreviewOptions);

	preview->exec();
}

void ExportManager::setPrintPreviewOptions(QPrinter *printer)
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;
	auto &d_print_paper_size = d_app->d_print_paper_size;
	auto &d_printer_orientation = d_app->d_printer_orientation;

	if (!printer)
		return;

	d_print_paper_size = printer->pageLayout().pageSize().id();
	d_printer_orientation = printer->pageLayout().orientation();
}

void ExportManager::printAllPlots()
{

	if (!d_app) return;
	ApplicationWindow *app = d_app;

	QPrinter printer;
	printer.setPageOrientation(QPageLayout::Landscape);
	printer.setColorMode (QPrinter::Color);
	printer.setFullPage(true);

	QPrintDialog dialog(&printer, d_app);
	if (dialog.exec() == QDialog::Accepted){
		QPainter *paint = new QPainter (&printer);

		int plots = 0;
		QList<MdiSubWindow *> windows = d_app->windowsList();
		for (MdiSubWindow *w : windows){
			if (qobject_cast<MultiLayer*>(w))
				plots++;
		}

		printer.setFromTo (0, plots);

		for (MdiSubWindow *w : windows){
			MultiLayer *ml = qobject_cast<MultiLayer*>(w);
			if (ml){
				ml->printAllLayers(paint);
				if (w != windows.last())
					printer.newPage();
			}
		}
		paint->end();
		delete paint;
	}
}
