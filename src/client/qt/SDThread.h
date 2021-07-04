#ifndef _THREAD_H_
#define _THREAD_H_
#include <QThread>

class SDThread : public QThread
{
  Q_OBJECT

  public:
    SDThread();
    void stop();
    bool isShutDown();

  protected:
    virtual void run() = 0;
    volatile bool _shutdown;
    volatile bool _stopped;

  private:
};


#endif //_THREAD_H_
