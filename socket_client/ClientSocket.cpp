#include "ClientSocket.h"

//// for readline
static readlineAdaptor *singleton;
static bool do_terminate;
static int check_state() {
    if (do_terminate) {
        rl_done = 1;
    }
    return 0;
}

char **character_name_completion(const char *, int, int);
char *character_name_generator(const char *, int);

char **
character_name_completion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    //printf("-> text(%d, %d):%s//%s\n", start, end, text, rl_line_buffer);

    singleton->queryComp(text);

    char ** ret = rl_completion_matches(text, character_name_generator);

    if (ret != NULL) {
        int i = 0;
        while(ret[i] != NULL) {
            //printf("?? match(%d):%s\n", i, ret[i]);
            i++;
        }
    }
    return ret;
}

char *
character_name_generator(const char *text, int state)
{
    //printf("--> (%d):%s=>", state, text);
    static int list_index, len;
    const char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    std::vector<std::string> &tabResults = singleton->getResults();
    while (list_index < tabResults.size()) {
        name = tabResults[list_index++].c_str();
        if (strncmp(name, text, len) == 0) {
            //printf("%s\n", name);
            return strdup(name);
        }
    }
    return NULL;
}

void readlineAdaptor::readlineProc()
{
    rl_catch_signals = 0;
    rl_clear_signals();
    rl_event_hook = check_state;
    do_terminate = false;

    singleton = this;
    // comp
    rl_attempted_completion_function = character_name_completion;
    char* comm;
    while ((comm = readline("")) != nullptr && !terminate) {
        if (strlen(comm) > 0) {
            add_history(comm);
            printf("[%s]\n", comm);
            QString qmsg(comm);
            emit sendComRequest(qmsg);
          } else {
            // just input return
            printf("[\n]\n");
            QString qmsg("\n");
            emit sendComRequest(qmsg);
        }
        // free buffer
        free(comm);
    }
}

void readlineAdaptor::queryComp(const char *str) {
  std::cerr << "query0" << std::endl;
    QString qmsg(str);
    QEventLoop loop1;
    connect(this, &readlineAdaptor::gotResults, &loop1, &QEventLoop::quit,
            Qt::DirectConnection);
  std::cerr << "query1" << std::endl;
    Q_EMIT sendTabRequest(qmsg);
  std::cerr << "query2" << std::endl;
    loop1.exec();
  std::cerr << "query3" << std::endl;
}

void readlineAdaptor::setQueryResults(const QByteArray &results) {
  std::cerr << "slot: setResults" << std::endl;
  QString qstr(results);
  std::vector<std::string> res_lst;
  QStringList qres_lst = qstr.split(";");
  for(auto q = qres_lst.begin(); q != qres_lst.end(); ++q) {
    res_lst.push_back(q->toStdString());
  }
  setResults(res_lst);
  Q_EMIT gotResults();
}

cnoidClientSocket::cnoidClientSocket(QObject * parent) : QLocalSocket(parent)
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

void cnoidClientSocket::startThread()
{
    readlineAdaptor *adaptor = new readlineAdaptor();
    this->rl_adaptor = adaptor;
    this->connect(adaptor, &readlineAdaptor::sendComRequest,
                  this, &cnoidClientSocket::sendMsg, Qt::BlockingQueuedConnection);
    this->connect(adaptor, &readlineAdaptor::sendTabRequest,
                  this, &cnoidClientSocket::sendQuery, Qt::BlockingQueuedConnection);
    this->connect(this, &cnoidClientSocket::setResults,
                  adaptor, &readlineAdaptor::setQueryResults,
                  Qt::QueuedConnection); //Qt::BlockingQueuedConnection);

    QFuture<void> future = QtConcurrent::run(adaptor, &readlineAdaptor::readlineProc);

    this->connect(this, &cnoidClientSocket::disconnected,
                  [=, &future]() {
                     std::cerr << "disconnected" << std::endl;
                     future.cancel();
                  });
    this->connect(this, &cnoidClientSocket::disconnected,
                  adaptor, &readlineAdaptor::setTerminate);
}

void cnoidClientSocket::sendMsg(const QString &msg)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_1);

    out << (qint64)0;
    out << msg.toUtf8();

    out.device()->seek(0);
    out << (qint64)1;

    this->write(block);
    this->flush();
}

void cnoidClientSocket::sendQuery(const QString &msg)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_1);

    out << (qint64)0;
    out << msg.toUtf8();

    out.device()->seek(0);
    out << (qint64)2; // query = 2

    this->write(block);
    this->flush();
}

void cnoidClientSocket::recvMsg()
{
    std::cerr << ">recvMsg" << std::endl;
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_1);

    while(!in.atEnd()) {
        qint64 channel;
        in >> channel;
        QByteArray data;
        in >> data;

        if (channel == 1) {
            // switch channel
            std::cout << std::string( data.data() );
            std::flush(std::cout);
        } else if (channel == 2) {
            std::cerr << "signal: setResults" << std::endl;
            Q_EMIT setResults(data);
        }
    }
    std::cerr << "recvMsg>" << std::endl;
}
