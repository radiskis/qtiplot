#ifndef POLARSETTINGSDIALOG_H
#define POLARSETTINGSDIALOG_H

#include <QDialog>

class QTabWidget;
class QGroupBox;
class QLineEdit;
class QCheckBox;
class QPushButton;
class PolarGraph;
class ColorButton;

class QComboBox;
class QSpinBox;

class PolarSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    PolarSettingsDialog(PolarGraph *g, QWidget *parent = 0);

private slots:
    void accept();
    void apply();
    void updateScale();
    void updateGrid();
    void setActiveCurve(int);

private:
    void initTabs();
    QWidget* initScaleTab();
    QWidget* initGridTab();
    QWidget* initFormatTab();

    PolarGraph *d_graph;

    QTabWidget *tabs;

    // Scale Tab Widgets
    QLineEdit *radialStart, *radialEnd, *radialStep;
    QCheckBox *radialAutoscale;
    QLineEdit *azimuthStart, *azimuthEnd, *azimuthStep;
    QCheckBox *azimuthAutoscale;

    // Grid Tab Widgets
    QCheckBox *showRadialMajor, *showRadialMinor;
    QCheckBox *showAzimuthMajor, *showAzimuthMinor;
    ColorButton *radialMajorColor, *radialMinorColor;
    ColorButton *azimuthMajorColor, *azimuthMinorColor;

    // Format Tab Widgets
    QComboBox *boxCurve;
    ColorButton *lineColor;
    QSpinBox *lineWidth;
    QComboBox *lineStyle;
    
    QComboBox *symbolStyle;
    QSpinBox *symbolSize;
    ColorButton *symbolColor;
    ColorButton *symbolPenColor;
    QSpinBox *symbolPenWidth;

    QPushButton *btnApply, *btnOk, *btnCancel;
};

#endif
