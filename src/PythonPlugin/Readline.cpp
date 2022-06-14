
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "Readline.h"

using namespace cnoid;

readlineAdaptor::readlineAdaptor(QObject *parent)
  : QObject(parent), prompt(">>> ") {

}

bool readlineAdaptor::startThread() {
    rl_future = QtConcurrent::run(this, &readlineAdaptor::readlineProc);
    return rl_future.isStarted();
}

static readlineAdaptor *singleton;
static std::vector<std::string> tabResults;
static bool do_terminate;
static int check_state() {
    if (do_terminate) {
        rl_done = 1;
    }
    return 0;
}

char **character_name_completion(const char *, int, int);
char *character_name_generator(const char *, int);
//rl_completion_maches()

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
    while (list_index < tabResults.size()) {
        name = tabResults[list_index++].c_str();
        if (strncmp(name, text, len) == 0) {
            //printf("%s\n", name);
            return strdup(name);
        }
    }
    return NULL;
}


void readlineAdaptor::readlineProc() {
    // for killing by Ctrl-C
    rl_catch_signals = 0;
    rl_clear_signals();
    rl_event_hook = check_state;
    do_terminate = false;

    singleton = this;
    // comp
    rl_attempted_completion_function = character_name_completion;

    char* comm;
    while ((comm = readline(prompt.toStdString().c_str())) != nullptr && !do_terminate) {
        if (strlen(comm) > 0) {
            add_history(comm);
            {
                QString qmsg(comm);
                Q_EMIT sendComRequest(qmsg);
            }
        } else {
            // just input return
            {
                QString qmsg("\n");
                Q_EMIT sendComRequest(qmsg);
            }
        }
        // free buffer
        free(comm);
    }
}

void readlineAdaptor::setTerminate() {
    do_terminate = true;
}

void readlineAdaptor::setResults(const std::vector<std::string> &ret) {
    tabResults = ret;
}

void readlineAdaptor::queryComp(const char *str) {
    QString qmsg(str);
    Q_EMIT sendTabRequest(qmsg);
}
