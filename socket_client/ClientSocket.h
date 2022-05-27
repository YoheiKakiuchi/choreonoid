#ifndef CLIENTSOCk_H
#define CLIENTSOCK_H

#include <QApplication>
#include <QLocalSocket>
#include <QDataStream>

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
      printf("r\n");
      if (strlen(comm) > 0) {
        add_history(comm);

        printf("[%s]\n", comm);
        this->sendMsg(comm);
      }
      // free buffer
      free(comm);
    }
  }

private:
  bool terminate;

public slots:
  void sendMsg(const char *msg)
  {
    QString qmsg(msg);
    emit sendRequest(qmsg);
  }
public slots:
  void setTerminate() { terminate = true; }

signals:
  void sendRequest(const QString msg);
};

class cnoidClientSocket : public QLocalSocket
{

  Q_OBJECT
public:
    explicit cnoidClientSocket(QObject * parent = 0)
      : QLocalSocket(parent)
    {
      this->abort();
      //this->setReadBufferSize(4096);

      //
      connect(this, &cnoidClientSocket::readyRead, this, &cnoidClientSocket::recvMsg);

      connect(this, static_cast<void (cnoidClientSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
              this, &cnoidClientSocket::handleError);
      //connect(this, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)),
      //        this, SLOT(handleState(QLocalSocket::LocalSocketState)));
      connect(this, &cnoidClientSocket::connected, this, &cnoidClientSocket::handleConnect);
      connect(this, &cnoidClientSocket::disconnected, this, &cnoidClientSocket::handleDisconnect);

      std::cerr << ">socket connection" << std::endl;
      this->connectToServer(".choreonoid_console_server");

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

public slots:
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
  void sendMsg(const QString msg)
  {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_1);

    out << msg.toUtf8();

    this->write(block);
    this->flush();
  }

  void recvMsg()
  {
    //std::cerr << ">recvMsg" << std::endl;
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_1);

    while(!in.atEnd()) {
      //qint64 blockSize;
      //in >> blockSize;
      QByteArray data;
      in >> data;
      std::cout << std::string( data.data() );
      std::flush(std::cout);
    }
    //std::cerr << "recvMsg>" << std::endl;
  }

};
#endif // CLIENTWIDGET_H
