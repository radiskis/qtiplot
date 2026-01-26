#include "PolarSettingsDialog.h"
#include <PolarGraph.h>
#include <ColorButton.h>
#include <qwt_polar_plot.h>
#include <qwt_polar_grid.h>
#include <qwt_scale_div.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>

#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <qwt_symbol.h>
#include <QComboBox>
#include <QSpinBox>

PolarSettingsDialog::PolarSettingsDialog(PolarGraph *g, QWidget *parent)
    : QDialog(parent), d_graph(g)
{
    setSizeGripEnabled(true);
    setWindowTitle(tr("QtiPlot - Polar Plot Settings"));
    
    initTabs();
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    QPushButton *btnApply = new QPushButton(tr("&Apply"));
    connect(btnApply, &QPushButton::clicked, this, &PolarSettingsDialog::apply);
    btnLayout->addWidget(btnApply);

    QPushButton *btnOk = new QPushButton(tr("&OK"));
    connect(btnOk, &QPushButton::clicked, this, &PolarSettingsDialog::accept);
    btnLayout->addWidget(btnOk);

    QPushButton *btnCancel = new QPushButton(tr("&Cancel"));
    connect(btnCancel, &QPushButton::clicked, this, &PolarSettingsDialog::reject);
    btnLayout->addWidget(btnCancel);
    
    layout->addLayout(btnLayout);
    
    apply();
}

void PolarSettingsDialog::initTabs()
{
    tabs = new QTabWidget();
    tabs->addTab(initScaleTab(), tr("Scale"));
    tabs->addTab(initGridTab(), tr("Grid"));
    tabs->addTab(initFormatTab(), tr("Format"));
}

QWidget* PolarSettingsDialog::initFormatTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    QHBoxLayout *curveLayout = new QHBoxLayout();
    curveLayout->addWidget(new QLabel(tr("Curve:")));
    boxCurve = new QComboBox();
    curveLayout->addWidget(boxCurve);
    layout->addLayout(curveLayout);

    QList<QwtPolarCurve*> curves = d_graph->curves();
    for (QwtPolarCurve *c : curves){
        boxCurve->addItem(c->title().text());
    }
    connect(boxCurve, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PolarSettingsDialog::setActiveCurve);

    // Line Group
    QGroupBox *lineBox = new QGroupBox(tr("Line"));
    QGridLayout *lineLayout = new QGridLayout(lineBox);
    
    lineColor = new ColorButton();
    lineLayout->addWidget(new QLabel(tr("Color:")), 0, 0);
    lineLayout->addWidget(lineColor, 0, 1);
    
    lineWidth = new QSpinBox();
    lineWidth->setRange(0, 100);
    lineLayout->addWidget(new QLabel(tr("Width:")), 1, 0);
    lineLayout->addWidget(lineWidth, 1, 1);
    
    lineStyle = new QComboBox();
    lineStyle->addItem("Solid", (int)Qt::SolidLine);
    lineStyle->addItem("Dash", (int)Qt::DashLine);
    lineStyle->addItem("Dot", (int)Qt::DotLine);
    lineStyle->addItem("DashDot", (int)Qt::DashDotLine);
    lineStyle->addItem("DashDotDot", (int)Qt::DashDotDotLine);
    lineLayout->addWidget(new QLabel(tr("Style:")), 2, 0);
    lineLayout->addWidget(lineStyle, 2, 1);
    
    layout->addWidget(lineBox);
    
    // Symbol Group
    QGroupBox *symbolBox = new QGroupBox(tr("Symbol"));
    QGridLayout *symbolLayout = new QGridLayout(symbolBox);
    
    symbolStyle = new QComboBox();
    symbolStyle->addItem("None", QwtSymbol::NoSymbol);
    symbolStyle->addItem("Ellipse", QwtSymbol::Ellipse);
    symbolStyle->addItem("Rect", QwtSymbol::Rect);
    symbolStyle->addItem("Diamond", QwtSymbol::Diamond);
    symbolStyle->addItem("Triangle", QwtSymbol::Triangle);
    symbolLayout->addWidget(new QLabel(tr("Style:")), 0, 0);
    symbolLayout->addWidget(symbolStyle, 0, 1);
    
    symbolSize = new QSpinBox();
    symbolSize->setRange(0, 100);
    symbolLayout->addWidget(new QLabel(tr("Size:")), 1, 0);
    symbolLayout->addWidget(symbolSize, 1, 1);
    
    symbolColor = new ColorButton(); 
    symbolLayout->addWidget(new QLabel(tr("Color:")), 2, 0);
    symbolLayout->addWidget(symbolColor, 2, 1);
    
    symbolPenColor = new ColorButton(); 
    symbolLayout->addWidget(new QLabel(tr("Edge Color:")), 3, 0);
    symbolLayout->addWidget(symbolPenColor, 3, 1);

    symbolPenWidth = new QSpinBox(); 
    symbolPenWidth->setRange(0,100);
    symbolLayout->addWidget(new QLabel(tr("Edge Width:")), 4, 0);
    symbolLayout->addWidget(symbolPenWidth, 4, 1);
    
    layout->addWidget(symbolBox);
    layout->addStretch();
    
    if (boxCurve->count() > 0) setActiveCurve(0);

    return tab;
}


QWidget* PolarSettingsDialog::initScaleTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);

    QGroupBox *radialBox = new QGroupBox(tr("Radial Axis"));
    QFormLayout *radialForm = new QFormLayout();
    radialStart = new QLineEdit();
    radialEnd = new QLineEdit();
    radialStep = new QLineEdit();
    radialAutoscale = new QCheckBox(tr("Autoscale"));
    radialForm->addRow(tr("Start:"), radialStart);
    radialForm->addRow(tr("End:"), radialEnd);
    radialForm->addRow(tr("Step:"), radialStep);
    radialForm->addRow(radialAutoscale);
    radialBox->setLayout(radialForm);

    QGroupBox *azimuthBox = new QGroupBox(tr("Azimuth Axis"));
    QFormLayout *azimuthForm = new QFormLayout();
    azimuthStart = new QLineEdit();
    azimuthEnd = new QLineEdit();
    azimuthStep = new QLineEdit();
    azimuthAutoscale = new QCheckBox(tr("Autoscale"));
    azimuthForm->addRow(tr("Start:"), azimuthStart);
    azimuthForm->addRow(tr("End:"), azimuthEnd);
    azimuthForm->addRow(tr("Step:"), azimuthStep);
    azimuthForm->addRow(azimuthAutoscale);
    azimuthBox->setLayout(azimuthForm);

    layout->addWidget(radialBox);
    layout->addWidget(azimuthBox);
    layout->addStretch();
    return tab;
}

QWidget* PolarSettingsDialog::initGridTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);

    QGroupBox *radialGridBox = new QGroupBox(tr("Radial Grid"));
    QGridLayout *radialGrid = new QGridLayout();
    showRadialMajor = new QCheckBox(tr("Major Grid"));
    showRadialMinor = new QCheckBox(tr("Minor Grid"));
    radialMajorColor = new ColorButton();
    radialMinorColor = new ColorButton();
    radialGrid->addWidget(showRadialMajor, 0, 0);
    radialGrid->addWidget(radialMajorColor, 0, 1);
    radialGrid->addWidget(showRadialMinor, 1, 0);
    radialGrid->addWidget(radialMinorColor, 1, 1);
    radialGridBox->setLayout(radialGrid);

    QGroupBox *azimuthGridBox = new QGroupBox(tr("Azimuth Grid"));
    QGridLayout *azimuthGrid = new QGridLayout();
    showAzimuthMajor = new QCheckBox(tr("Major Grid"));
    showAzimuthMinor = new QCheckBox(tr("Minor Grid"));
    azimuthMajorColor = new ColorButton();
    azimuthMinorColor = new ColorButton();
    azimuthGrid->addWidget(showAzimuthMajor, 0, 0);
    azimuthGrid->addWidget(azimuthMajorColor, 0, 1);
    azimuthGrid->addWidget(showAzimuthMinor, 1, 0);
    azimuthGrid->addWidget(azimuthMinorColor, 1, 1);
    azimuthGridBox->setLayout(azimuthGrid);

    layout->addWidget(radialGridBox);
    layout->addWidget(azimuthGridBox);
    layout->addStretch();
    return tab;
}

void PolarSettingsDialog::updateScale()
{
    QwtPolarPlot *plot = d_graph->plot();
    
    const QwtScaleDiv *radialDiv = plot->scaleDiv(QwtPolar::ScaleRadius);
    if (radialDiv) {
        radialStart->setText(QString::number(radialDiv->lowerBound()));
        radialEnd->setText(QString::number(radialDiv->upperBound()));
    }
    radialAutoscale->setChecked(plot->hasAutoScale(QwtPolar::ScaleRadius)); 

    const QwtScaleDiv *azimuthDiv = plot->scaleDiv(QwtPolar::ScaleAzimuth);
    if (azimuthDiv) {
        azimuthStart->setText(QString::number(azimuthDiv->lowerBound()));
        azimuthEnd->setText(QString::number(azimuthDiv->upperBound()));
    }
}

void PolarSettingsDialog::updateGrid()
{
    QwtPolarGrid *grid = d_graph->grid();
    showRadialMajor->setChecked(grid->isGridVisible(QwtPolar::ScaleRadius));
    showRadialMinor->setChecked(grid->isMinorGridVisible(QwtPolar::ScaleRadius));
    radialMajorColor->setColor(grid->majorGridPen(QwtPolar::ScaleRadius).color());
    radialMinorColor->setColor(grid->minorGridPen(QwtPolar::ScaleRadius).color());

    showAzimuthMajor->setChecked(grid->isGridVisible(QwtPolar::ScaleAzimuth));
    showAzimuthMinor->setChecked(grid->isMinorGridVisible(QwtPolar::ScaleAzimuth));
    azimuthMajorColor->setColor(grid->majorGridPen(QwtPolar::ScaleAzimuth).color());
    azimuthMinorColor->setColor(grid->minorGridPen(QwtPolar::ScaleAzimuth).color());
}

void PolarSettingsDialog::setActiveCurve(int index)
{
    if (index < 0) return;
    QList<QwtPolarCurve*> curves = d_graph->curves();
    if (index >= curves.size()) return;
    
    QwtPolarCurve *c = curves[index];
    lineColor->setColor(c->pen().color());
    lineWidth->setValue(c->pen().width());
    
    int styleIdx = lineStyle->findData((int)c->pen().style());
    if (styleIdx != -1) lineStyle->setCurrentIndex(styleIdx);
    
    const QwtSymbol *s = c->symbol();
    if (s) {
        int symIdx = symbolStyle->findData((int)s->style());
        if (symIdx != -1) symbolStyle->setCurrentIndex(symIdx);
        else symbolStyle->setCurrentIndex(symbolStyle->findData(QwtSymbol::NoSymbol));

        symbolSize->setValue(s->size().width());
        symbolColor->setColor(s->brush().color());
        symbolPenColor->setColor(s->pen().color());
        symbolPenWidth->setValue(s->pen().width());
    } else {
        symbolStyle->setCurrentIndex(symbolStyle->findData(QwtSymbol::NoSymbol));
    }
}

void PolarSettingsDialog::apply()
{
    d_graph->setAxisScale(QwtPolar::ScaleRadius, radialStart->text().toDouble(), radialEnd->text().toDouble(), radialStep->text().toDouble());
    d_graph->setAxisAutoscale(QwtPolar::ScaleRadius, radialAutoscale->isChecked());

    d_graph->setAxisScale(QwtPolar::ScaleAzimuth, azimuthStart->text().toDouble(), azimuthEnd->text().toDouble(), azimuthStep->text().toDouble());

    d_graph->showGrid(QwtPolar::ScaleRadius, showRadialMajor->isChecked());
    d_graph->showMinorGrid(QwtPolar::ScaleRadius, showRadialMinor->isChecked());
    
    QPen p = d_graph->grid()->majorGridPen(QwtPolar::ScaleRadius);
    p.setColor(radialMajorColor->color());
    d_graph->setMajorGridPen(QwtPolar::ScaleRadius, p);
    
    p = d_graph->grid()->minorGridPen(QwtPolar::ScaleRadius);
    p.setColor(radialMinorColor->color());
    d_graph->setMinorGridPen(QwtPolar::ScaleRadius, p);

    d_graph->showGrid(QwtPolar::ScaleAzimuth, showAzimuthMajor->isChecked());
    d_graph->showMinorGrid(QwtPolar::ScaleAzimuth, showAzimuthMinor->isChecked());

    p = d_graph->grid()->majorGridPen(QwtPolar::ScaleAzimuth);
    p.setColor(azimuthMajorColor->color());
    d_graph->setMajorGridPen(QwtPolar::ScaleAzimuth, p);

    p = d_graph->grid()->minorGridPen(QwtPolar::ScaleAzimuth);
    p.setColor(azimuthMinorColor->color());
    d_graph->setMinorGridPen(QwtPolar::ScaleAzimuth, p);
    
    // Apply Curve Settings
    int index = boxCurve->currentIndex();
    if (index >= 0) {
        QList<QwtPolarCurve*> curves = d_graph->curves();
        if (index < curves.size()) {
            QwtPolarCurve *c = curves[index];
            QPen cp = c->pen();
            cp.setColor(lineColor->color());
            cp.setWidth(lineWidth->value());
            cp.setStyle((Qt::PenStyle)lineStyle->itemData(lineStyle->currentIndex()).toInt());
            c->setPen(cp);
            
            int sStyle = symbolStyle->itemData(symbolStyle->currentIndex()).toInt();
            if (sStyle == QwtSymbol::NoSymbol){
                 c->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol));
            } else {
                 QwtSymbol *symb = new QwtSymbol((QwtSymbol::Style)sStyle);
                 symb->setSize(symbolSize->value());
                 symb->setColor(symbolColor->color()); // Brush
                 symb->setPen(QPen(symbolPenColor->color(), symbolPenWidth->value())); // Pen
                 c->setSymbol(symb);
            }
        }
    }

    d_graph->plot()->replot();
}

void PolarSettingsDialog::accept()
{
    apply();
    QDialog::accept();
}
