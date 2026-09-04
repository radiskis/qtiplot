#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#include <QString>
#include <QStringList>

namespace CrashHandler {
    void initCrashHandler();
    QString recoveryDirPath();
    QStringList findRecoveryFiles();
    void cleanSessionRecoveryFiles();
    void setSessionRecoveryFile(const QString &filePath);
    QString sessionRecoveryFile();
}

#endif // CRASH_HANDLER_H
