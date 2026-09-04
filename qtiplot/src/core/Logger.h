#ifndef LOGGER_H
#define LOGGER_H

#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(lcIo)
Q_DECLARE_LOGGING_CATEGORY(lcFit)
Q_DECLARE_LOGGING_CATEGORY(lcScript)
Q_DECLARE_LOGGING_CATEGORY(lcPlot)

namespace Logger {
    void initLogging();
    QString logFilePath();
    QString logDirPath();
}

#endif // LOGGER_H
