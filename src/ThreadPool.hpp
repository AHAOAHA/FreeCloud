/*************************************************************************
*File Name: ThreadPool.hpp
*Author: AHAOAHA
*mail: ahaoaha_@outlook.com
*Created Time: 2019年02月19日 星期二 14时42分01秒
 ************************************************************************/
#pragma once
#include "Utils.hpp"
#include <pthread.h>
#include <queue>

class ThreadPool {
  private:
    int _thr_num; //记录线程池中线程数量
    bool _is_stop;  //标记线程池是否停止
    std::queue<HttpTask> _task_queue; //任务队列
    pthread_cond_t _cond; //条件变量
    pthread_mutex_t _lock;  //锁

  private:
    static void* _thr_start(void *arg)
    {
      ThreadPool* tp = (ThreadPool*)arg;
      for(;;)
      {
        tp->LockQueue();
        while(tp->QueueEmpty())
        {
          if(tp->IsStop())
          {
            tp->UnlockQueue();
            pthread_exit(NULL);
          }
          tp->ThreadWait();
        }
        HttpTask tt = tp->PopTask();
        tp->UnlockQueue();
        tt.Run();
      }
    }
    void LockQueue() {
      pthread_mutex_lock(&_lock);
    }
    void UnlockQueue() {
      pthread_mutex_unlock(&_lock);
    }
    void ThreadWait() {
      pthread_cond_wait(&_cond, &_lock);
    }
    bool QueueEmpty() {
      return _task_queue.empty();
    }
    HttpTask PopTask()
    {
      HttpTask tt = _task_queue.front();
      _task_queue.pop();
      return tt;
    }
    void WakeupOne() {
      pthread_cond_signal(&_cond);
    }
    void WakeupAll() {
      pthread_cond_broadcast(&_cond);
    }
    bool IsStop() {
      return _is_stop;
    }

  public:
    ThreadPool(int num = 5):_thr_num(num), _is_stop(false)    {}
    bool ThreadInit()
    {
      pthread_t tid;
      for(int i = 0; i < _thr_num; ++i)
      {
        if(pthread_create(&tid, NULL, _thr_start, this) != 0) {
          cerr << "thread create error" << endl;
          return false;
        }
        if(pthread_detach(tid) != 0) {
          cerr << "thread detach error" << endl;
          return false;
        }
      }

      if(pthread_mutex_init(&_lock, NULL) != 0) {
        cerr << "mutex init error" << endl;
        return false;
      }
      if(pthread_cond_init(&_cond, NULL) != 0) {
        cerr << "cond init error" << endl;
        return false;
      }
      return true;
    }

    void PushTask(HttpTask tt) {
      HttpTask* task = new HttpTask(tt);
      LockQueue();
      _task_queue.push(*task);
      UnlockQueue();
      WakeupOne();
    }

    void StopThreadPool() {
      _is_stop = true;
    }
};
