#include <QApplication>
#include "ClientSocket.h"

int main(int argc, char * argv[])
{
  // TODO start choreonoid process...
  QApplication a(argc, argv);

  cnoidClientSocket client;

  if (!client.isValid()) {
    return 1;
  }

  client.startThread();

  return a.exec();
}
