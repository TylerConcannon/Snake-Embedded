#ifndef PELLETTHREAD_H
#define PELLETTHREAD_H

#include <QThread>

class PelletThread : QThread
{
private:
    void run();
public:
    PelletThread();
};

#endif // PELLETTHREAD_H
