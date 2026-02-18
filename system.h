#include <QSysInfo>
#include <QThread>

struct SystemInfo
{
    QString ProductName;
};

struct CPUInfo
{
    QString Architecture;
    int LogicalCores;
};

void query_system_info(SystemInfo* Info)
{
    Info->ProductName = QSysInfo::prettyProductName();
}

void query_cpu_info(CPUInfo *Info)
{
    Info->Architecture = QSysInfo::currentCpuArchitecture();
    Info->LogicalCores = QThread::idealThreadCount();
    // TODO(warren): Get a cpu name. Might need different code
    // for different platforms. QT doesn't seem to have a way to do this nicely.
}

