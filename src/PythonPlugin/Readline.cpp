
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "Readline.h"

using namespace cnoid;

readlineAdaptor::readlineAdaptor(QObject *parent)
    : QObject(parent), terminate(false)  {

}

bool readlineAdaptor::startThread() {
    rl_future = QtConcurrent::run(this, &readlineAdaptor::readlineProc);
    return rl_future.isStarted();
}

void readlineAdaptor::readlineProc() {
    char* comm;
    while ((comm = readline("")) != nullptr && !terminate) {
        if (strlen(comm) > 0) {
            add_history(comm);
            {
                QString qmsg(comm);
                Q_EMIT sendRequest(qmsg);
            }
        } else {
            // just input return
            {
                QString qmsg("\n");
                Q_EMIT sendRequest(qmsg);
            }
        }
        // free buffer
        free(comm);
    }
}

void readlineAdaptor::setTerminate() {
    terminate = true;
    printf("term\n");
    if(rl_future.isStarted()) {
        printf(">can\n");
        rl_future.cancel();
        printf("can>\n");
    }
}
