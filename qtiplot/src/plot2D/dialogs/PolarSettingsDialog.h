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

    PolarGraph *d_graph = nullptr;

    QTabWidget *tabs = nullptr;

    // Scale Tab Widgets
    QLineEdit *radialStart = nullptr, *radialEnd = nullptr, *radialStep = nullptr;
    QCheckBox *radialAutoscale = nullptr;
    QLineEdit *azimuthStart = nullptr, *azimuthEnd = nullptr, *azimuthStep = nullptr;
    QCheckBox *azimuthAutoscale = nullptr;

    // Grid Tab Widgets
    QCheckBox *showRadialMajor = nullptr, *showRadialMinor = nullptr;
    QCheckBox *showAzimuthMajor = nullptr, *showAzimuthMinor = nullptr;
    ColorButton *radialMajorColor = nullptr, *radialMinorColor = nullptr;
    ColorButton *azimuthMajorColor = nullptr, *azimuthMinorColor = nullptr;

    // Format Tab Widgets
    QComboBox *boxCurve = nullptr;
    ColorButton *lineColor = nullptr;
    QSpinBox *lineWidth = nullptr;
    QComboBox *lineStyle = nullptr;
    
    QComboBox *symbolStyle = nullptr;
    QSpinBox *symbolSize = nullptr;
    ColorButton *symbolColor = nullptr;
    ColorButton *symbolPenColor = nullptr;
    QSpinBox *symbolPenWidth = nullptr;

    QPushButton *btnApply = nullptr, *btnOk = nullptr, *btnCancel = nullptr;
};

#endif
