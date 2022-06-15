#ifndef CLIENTSOCk_H
#define CLIENTSOCK_H

#include <QApplication>
#include <QLocalSocket>
#include <QDataStream>
#include <QThread>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

class readlineAdaptor : public QObject
{
  Q_OBJECT
public:
  explicit readlineAdaptor()
  {
    terminate = false;
  }
public:
  void readlineProc() {
    char* comm;
    while ((comm = readline("")) != nullptr && !terminate) {
      if (strlen(comm) > 0) {
        add_history(comm);

        printf("[%s]\n", comm);
        this->sendMsg(comm);
      } else {
        // just input return
        printf("[\n]\n");
        this->sendMsg("\n");
      }
      // free buffer
      free(comm);
    }
  }

private:
  bool terminate;

private:
  inline void sendMsg(const char *msg)
  {
    QString qmsg(msg);
    emit sendRequest(qmsg);
  }

public Q_SLOTS:
  void setTerminate() { terminate = true; }

Q_SIGNALS:
  void sendCom(const QString msg);
  void sendQuerry(const QString msg);
};


class cnoidClientSocket : public QLocalSocket
{

  Q_OBJECT

public:
  std::string stored;

public:
    explicit cnoidClientSocket(QObject * parent = 0)
      : QLocalSocket(parent)
    {

    }

  bool connectWith(std::string &name)
  {
      this->abort();
      //this->setReadBufferSize(4096);

      //
      connect(this, &cnoidClientSocket::readyRead, this, &cnoidClientSocket::recvMsg,
              //Qt::BlockingQueuedConnection);
              Qt::DirectConnection);

      connect(this, static_cast<void (cnoidClientSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
              this, &cnoidClientSocket::handleError);
      //connect(this, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)),
      //        this, SLOT(handleState(QLocalSocket::LocalSocketState)));
      connect(this, &cnoidClientSocket::connected, this, &cnoidClientSocket::handleConnect);
      connect(this, &cnoidClientSocket::disconnected, this, &cnoidClientSocket::handleDisconnect);

      std::cerr << ">socket connection" << std::endl;
      this->connectToServer(name);

      std::cerr << "vaild: " << this->isValid() << std::endl;

      if (!this->waitForConnected(10000)) {
        std::cerr << "socket connect failed" << std::endl;
        //QLocalSocket::LocalSocketError er = this->error();
        return;
      }

      ///HelloServerサーバーに接続
      std::cerr << "socket connection>" << std::endl;
    }

    virtual ~cnoidClientSocket()
    {

    }

Q_SIGNALS:
  void finishStored();

public Q_SLOTS:
  void startThread() {
    this->readlineProc();
  }
  void setTerminate() { terminate = true; }

  void hogehoge()
  {
    std::cerr << "class::hogehoge" << std::endl;
  }

  void handleConnect()
  {
    std::cerr << "class::connected" << std::endl;
  }
  void handleDisconnect()
  {
    std::cerr << "class::disconnected" << std::endl;
    terminate = false;
    QApplication::quit();
  }
  void handleError(QLocalSocket::LocalSocketError l_error)
  {
    std::cerr << "class::error: " << l_error << std::endl;
  }
  void handleState(QLocalSocket::LocalSocketState l_state)
  {
    std::cerr << "class::state: " << l_state << std::endl;
  }
  //void sendMsg(const char* msg)
  void sendMsg(const QString msg)
  {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_1);

    out << msg.toUtf8();

    this->write(block);
    this->flush();
  }

  void sendMsgWait(const QString msg)
  {
    QDataStream in(this);
    // check available

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_1);

    out << msg.toUtf8();

    this->write(block);
    this->flush();

    //loop;
    //connect(this, finishStored, &loop, finish);
    //loop.exec();
    // wait received

  }

  void recvMsg()
  {
    std::cerr << ">recvMsg" << std::endl;
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_1);

    while(!in.atEnd()) {
      //qint64 blockSize;
      //in >> blockSize;
      QByteArray data;
      in >> data;
      this->stored = std::string( data.data() );
      std::cout << this->stored;
      std::flush(std::cout);
    }

    emit finishStored();
    std::cerr << "recvMsg>" << std::endl;
  }

};
#endif // CLIENTWIDGET_H
