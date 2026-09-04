#include "Logger.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>
#include <iostream>

Q_LOGGING_CATEGORY(lcIo, "qtiplot.io")
Q_LOGGING_CATEGORY(lcFit, "qtiplot.fit")
Q_LOGGING_CATEGORY(lcScript, "qtiplot.script")
Q_LOGGING_CATEGORY(lcPlot, "qtiplot.plot")

namespace {
    QFile *s_logFile = nullptr;
    QMutex s_logMutex;

    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        QMutexLocker locker(&s_logMutex);

        QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        const char *levelStr = "DEBUG";
        switch (type) {
            case QtDebugMsg:    levelStr = "DEBUG"; break;
            case QtInfoMsg:     levelStr = "INFO "; break;
            case QtWarningMsg:  levelStr = "WARN "; break;
            case QtCriticalMsg: levelStr = "CRIT "; break;
            case QtFatalMsg:    levelStr = "FATAL"; break;
        }

        QString formatted = QString("[%1] [%2] [%3] %4\n")
            .arg(timeStr, levelStr, context.category ? context.category : "default", msg);

        // Always tee to standard error/console
        std::cerr << formatted.toLocal8Bit().constData();

        if (s_logFile && s_logFile->isOpen()) {
            QTextStream stream(s_logFile);
            stream.setEncoding(QStringConverter::Utf8);
            stream << formatted;
            stream.flush();
        }
    }
}

namespace Logger {

QString logDirPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty())
        base = QDir::tempPath() + "/qtiplot";
    return base + "/logs";
}

QString logFilePath()
{
    return logDirPath() + "/qtiplot.log";
}

void initLogging()
{
    QString dirPath = logDirPath();
    QDir().mkpath(dirPath);

    QString filePath = logFilePath();

    // Rotate log if larger than 5 MB
    QFileInfo fi(filePath);
    if (fi.exists() && fi.size() > 5 * 1024 * 1024) {
        QString oldPath = filePath + ".1";
        QFile::remove(oldPath);
        QFile::rename(filePath, oldPath);
    }

    s_logFile = new QFile(filePath);
    if (s_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qInstallMessageHandler(messageHandler);
    } else {
        qInstallMessageHandler(messageHandler);
    }
}

} // namespace Logger
