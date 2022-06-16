#ifndef CNOID_READLINE_H
#define CNOID_READLINE_H

#include <QObject>
#include <QtConcurrent>
#include <QLocalServer>
#include <QLocalSocket>

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
        void queryComp(const char *str);
        void put(const QString &message);
        void put_socket(const QString &message, int channel = 1);

        void setResults(const std::vector<std::string> &ret) {
            _results = ret;
        }
        std::vector<std::string> &getResults() {
            return _results;
        }

    Q_SIGNALS:
        void sendComRequest(const QString &msg);
        void sendTabRequest(const QString &msg);

    private:
        QFuture<void> rl_future;
        QString prompt;
        std::vector<std::string> _results;
    // socket
    public:
        QLocalServer *qserver;
        QLocalSocket *qsocket;

    };
}

#endif // CNOID_READLINE_H
