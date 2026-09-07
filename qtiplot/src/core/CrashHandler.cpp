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
    char s_reportPathBuf[512] = {0};
    char s_recoveryPathBuf[512] = {0};

    void writeCrashReport(const char *reason)
    {
        if (s_reportPathBuf[0] == '\0')
            return;

#if defined(_WIN32) || defined(WIN32)
        HANDLE hFile = CreateFileA(s_reportPathBuf, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            char buf[1024];
            int len = snprintf(buf, sizeof(buf), "QtiPlot Crash Report\nPID: %lu\nReason: %s\nActive Autosave: %s\n",
                               (unsigned long)GetCurrentProcessId(), reason, s_recoveryPathBuf);
            if (len > 0) {
                DWORD written = 0;
                WriteFile(hFile, buf, (DWORD)len, &written, NULL);
            }
            CloseHandle(hFile);
        }
#else
        int fd = ::open(s_reportPathBuf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd >= 0) {
            char buf[1024];
            int len = snprintf(buf, sizeof(buf), "QtiPlot Crash Report\nPID: %d\nReason: %s\nActive Autosave: %s\n",
                               (int)getpid(), reason, s_recoveryPathBuf);
            if (len > 0) {
                ::write(fd, buf, len);
            }
            ::close(fd);
        }
#endif
    }

#if defined(_WIN32) || defined(WIN32)
    LONG WINAPI windowsUnhandledExceptionFilter(EXCEPTION_POINTERS *ep)
    {
        char reason[128];
        snprintf(reason, sizeof(reason), "Unhandled Windows Exception code: 0x%08lx",
                 (unsigned long)ep->ExceptionRecord->ExceptionCode);
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

        writeCrashReport(sigName);
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
    strncpy(s_recoveryPathBuf, filePath.toUtf8().constData(), sizeof(s_recoveryPathBuf) - 1);
    s_recoveryPathBuf[sizeof(s_recoveryPathBuf) - 1] = '\0';
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
        s_recoveryPathBuf[0] = '\0';
    }
}

QStringList findRecoveryFiles()
{
    QDir dir(recoveryDirPath());
    if (!dir.exists())
        return QStringList();

    QStringList filters;
    filters << "*.qti";
    QStringList allFiles = dir.entryList(filters, QDir::Files, QDir::Time);
    QStringList result;

    qint64 currentPid = QCoreApplication::applicationPid();

    for (const QString &fn : allFiles) {
        QFileInfo fi(fn);
        QString baseName = fi.completeBaseName();
        int lastUnderscore = baseName.lastIndexOf('_');
        if (lastUnderscore >= 0) {
            bool ok = false;
            qint64 pid = baseName.mid(lastUnderscore + 1).toLongLong(&ok);
            if (ok) {
                if (pid == currentPid)
                    continue; // Skip our own running instance

#if defined(_WIN32) || defined(WIN32)
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
                if (hProcess != NULL) {
                    DWORD exitCode = 0;
                    if (GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE) {
                        CloseHandle(hProcess);
                        continue; // Process is still running
                    }
                    CloseHandle(hProcess);
                }
#else
                if (kill((pid_t)pid, 0) == 0) {
                    continue; // Process is still running
                }
#endif
            }
        }
        result << fn;
    }
    return result;
}

void initCrashHandler()
{
    QString dir = recoveryDirPath();
    QDir().mkpath(dir);

    qint64 pid = QCoreApplication::applicationPid();
    QString reportPath = QString("%1/crash_%2.txt").arg(dir).arg(pid);
    strncpy(s_reportPathBuf, reportPath.toUtf8().constData(), sizeof(s_reportPathBuf) - 1);
    s_reportPathBuf[sizeof(s_reportPathBuf) - 1] = '\0';

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
