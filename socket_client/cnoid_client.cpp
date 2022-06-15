#include <QApplication>
#include <QtConcurrent>
#include <QThread>
#include "ClientSocket.h"

int main(int argc, char * argv[])
{
  // TODO start choreonoid process...

  //
  QApplication a(argc, argv);

  cnoidClientSocket client;
  //readlineAdaptor adaptor;

  if (!client.isValid()) {
    return 1;
  }
  //client.connect(&adaptor, &readlineAdaptor::sendRequest,
  //&client, &cnoidClientSocket::sendMsg);

  QFuture<void> future = QtConcurrent::run(&client, &cnoidClientSocket::readlineProc);
#if 0
  QFuture<void> future = QtConcurrent::run([] () {
      cnoidClientSocket client;
      if (!client.isValid()) {
        return 1;
      }

      client.readlineProc();

      return 0;
    });
#endif
  //client.readlineProc();
  //client.connect(&client, &cnoidClientSocket::disconnected,
  //[=, &future]() {
  ///std::cerr << "moge moge" << std::endl;
  //future.cancel();
  //});

  //client.connect(&client, &cnoidClientSocket::disconnected,
  //&adaptor, &readlineAdaptor::setTerminate);

#if 0
  QThread* workerThread = new QThread();

  workerThread->connect(workerThread , &QThread::started,
                        &client, &cnoidClientSocket::startThread);

  //client.moveToThread(workerThread);
  workerThread->start();
#endif

  return a.exec();
}
