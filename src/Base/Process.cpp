/**
   @author Shin'ichiro Nakaoka
*/

#include "Process.h"
#if 0
#ifdef Q_OS_UNIX
#include "unistd.h"
#endif
#endif
using namespace cnoid;

Process::Process(QObject* parent)
    : QObject(parent)
{
#if 0
    connect(this, SIGNAL(readyReadStandardOutput()),
            this, SLOT(onReadyReadStandardOutput()));
#endif
}


void Process::onReadyReadStandardOutput()
{
    sigReadyReadStandardOutput_();
}


//void Process::start(const QString& program, const QStringList& arguments, OpenMode mode)
void Process::start(const QString& program, const QStringList& arguments)
{
#if 0
    QProcess::start(program, arguments, mode);
    
#ifdef Q_OS_UNIX
    setpgid(processId(), 0);
#endif
#endif
}
