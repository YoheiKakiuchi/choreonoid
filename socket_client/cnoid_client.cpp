#include <QApplication>
#include <QtConcurrent>
#include "ClientSocket.h"

int main(int argc, char * argv[])
{
  // TODO start choreonoid process...

  //
  QApplication a(argc, argv);

  cnoidClientSocket client;
  readlineAdaptor adaptor;

  if (!client.isValid()) {
    return 1;
  }
  client.connect(&adaptor, &readlineAdaptor::sendRequest,
                 &client, &cnoidClientSocket::sendMsg);

  QFuture<void> future = QtConcurrent::run(&adaptor, &readlineAdaptor::readlineProc);

  client.connect(&client, &cnoidClientSocket::disconnected,
                 [=, &future]() {
                   std::cerr << "moge moge" << std::endl;
                   future.cancel();
                 });
  client.connect(&client, &cnoidClientSocket::disconnected,
                 &adaptor, &readlineAdaptor::setTerminate);

  return a.exec();
}
