#ifndef CNOID_READLINE_H
#define CNOID_READLINE_H

#include <QObject>
#include <QtConcurrent>
#include "exportdecl.h"

namespace cnoid {

    class CNOID_EXPORT readlineAdaptor : public QObject
    {
        Q_OBJECT;

    public:
        explicit readlineAdaptor(QObject *parent = nullptr);

    public:
        ~readlineAdaptor() { }

    public:
        bool startThread();
        void readlineProc();
        void setTerminate();
        void setPrompt(const QString &p) { prompt = p; };
        void setResults(const std::vector<std::string> &ret);
        void queryComp(const char *str);

        void put(const QString &message) {
            std::cout <<  message.toStdString();
            std::flush(std::cout);
        }

    Q_SIGNALS:
        void sendComRequest(const QString &msg);
        void sendTabRequest(const QString &msg);

    private:
        QFuture<void> rl_future;
        QString prompt;

        //socket command(standard input/output)
        //socket querry
    };
}

#endif // CNOID_READLINE_H
