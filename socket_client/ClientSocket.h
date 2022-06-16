#ifndef CLIENTSOCk_H
#define CLIENTSOCK_H

#include <QApplication>
#include <QLocalSocket>
#include <QDataStream>
#include <QtConcurrent>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

class readlineAdaptor : public QObject
{
  Q_OBJECT;

public:
  explicit readlineAdaptor()
  {
    terminate = false;
  }
public:
  void readlineProc();
  void queryComp(const char *str);
  void setResults(const std::vector<std::string> &ret) {
    _results = ret;
  }
  std::vector<std::string> &getResults() {
    return _results;
  }

private:
  bool terminate;
  QString prompt;
  std::vector<std::string> _results;

public Q_SLOTS:
  void setTerminate() { terminate = true; }
  void setQueryResults(const QByteArray &results);

Q_SIGNALS:
  void sendComRequest(const QString &msg);
  void sendTabRequest(const QString &msg);
  void gotResults();
};

class cnoidClientSocket : public QLocalSocket
{
    Q_OBJECT;
public:
    explicit cnoidClientSocket(QObject * parent = 0);

    virtual ~cnoidClientSocket() { }

    void startThread();
public:
    readlineAdaptor *rl_adaptor;

public Q_SLOTS:
  void handleConnect()
  {
    std::cerr << "connected" << std::endl;
  }
  void handleDisconnect()
  {
    std::cerr << "disconnected" << std::endl;
    QApplication::quit();
  }
  void handleError(QLocalSocket::LocalSocketError l_error)
  {
    std::cerr << "error: " << l_error << std::endl;
  }
  void handleState(QLocalSocket::LocalSocketState l_state)
  {
    std::cerr << "state: " << l_state << std::endl;
  }

  //void sendMsg(const char* msg)
  void sendMsg(const QString &msg);
  void sendQuery(const QString &msg);
  void recvMsg();

Q_SIGNALS:
  void setResults(const QByteArray &results);

};
#endif // CLIENTWIDGET_H
