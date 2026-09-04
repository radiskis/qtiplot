#include "CrashHandler.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>
#include <exception>
#include <csignal>
#include <iostream>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

namespace {
    QString s_currentRecoveryFile;

    void writeCrashReport(const QString &reason)
    {
        QString dir = CrashHandler::recoveryDirPath();
        QDir().mkpath(dir);

        QString timeStamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        qint64 pid = QCoreApplication::applicationPid();
        QString reportPath = QString("%1/crash_%2_%3.txt").arg(dir).arg(pid).arg(timeStamp);

        QFile f(reportPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&f);
            out << "QtiPlot Crash Report\n";
            out << "Timestamp: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
            out << "PID: " << pid << "\n";
            out << "Reason: " << reason << "\n";
            if (!s_currentRecoveryFile.isEmpty()) {
                out << "Active Autosave / Recovery File: " << s_currentRecoveryFile << "\n";
            }
            out.flush();
            f.close();
        }
    }

#if defined(_WIN32) || defined(WIN32)
    LONG WINAPI windowsUnhandledExceptionFilter(EXCEPTION_POINTERS *ep)
    {
        QString reason = QString("Unhandled Windows Exception code: 0x%1")
            .arg((ulong)ep->ExceptionRecord->ExceptionCode, 8, 16, QChar('0'));
        writeCrashReport(reason);
        return EXCEPTION_CONTINUE_SEARCH;
    }
#endif

    void terminateHandler()
    {
        writeCrashReport("std::terminate called (uncaught exception)");
        std::abort();
    }

    void signalHandler(int sig)
    {
        const char *sigName = "Unknown signal";
        if (sig == SIGSEGV) sigName = "SIGSEGV (Segmentation Fault)";
        else if (sig == SIGABRT) sigName = "SIGABRT (Abort)";
        else if (sig == SIGFPE)  sigName = "SIGFPE (Floating Point Exception)";
        else if (sig == SIGILL)  sigName = "SIGILL (Illegal Instruction)";

        writeCrashReport(QString("Received signal: %1").arg(sigName));
        std::signal(sig, SIG_DFL);
        std::raise(sig);
    }
}

namespace CrashHandler {

QString recoveryDirPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty())
        base = QDir::tempPath() + "/qtiplot";
    return base + "/recovery";
}

void setSessionRecoveryFile(const QString &filePath)
{
    s_currentRecoveryFile = filePath;
}

QString sessionRecoveryFile()
{
    return s_currentRecoveryFile;
}

void cleanSessionRecoveryFiles()
{
    if (!s_currentRecoveryFile.isEmpty() && QFile::exists(s_currentRecoveryFile)) {
        QFile::remove(s_currentRecoveryFile);
        s_currentRecoveryFile.clear();
    }
}

QStringList findRecoveryFiles()
{
    QDir dir(recoveryDirPath());
    if (!dir.exists())
        return QStringList();

    QStringList filters;
    filters << "*.qti";
    return dir.entryList(filters, QDir::Files, QDir::Time);
}

void initCrashHandler()
{
#if defined(_WIN32) || defined(WIN32)
    SetUnhandledExceptionFilter(windowsUnhandledExceptionFilter);
#endif
    std::set_terminate(terminateHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGFPE, signalHandler);
    std::signal(SIGILL, signalHandler);
}

} // namespace CrashHandler
