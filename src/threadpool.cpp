/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "threadpool.h"
#include "threadpool_p.h"
#include <utils/d_ptr_implementation.h>
#include <Wt/WServer>

using namespace std;
using namespace Wt;

ThreadPool::Private::Private(int max, ThreadPool* q) : max(max), q(q)
{
}
ThreadPool::Private::~Private()
{
}

std::shared_ptr<ThreadPool> ThreadPool::instance(int max)
{
  static shared_ptr<ThreadPool> threadpool(new ThreadPool(max));
  return threadpool;
}


ThreadPool::ThreadPool(int max)
    : d(max, this)
{
  for(int i=0; i<max; i++) {
    boost::async([=]{
      Function function;
      while(WServer::instance()->isRunning()) {
        {
          WServer::instance()->log("notice") << "[thread-" << i << "]: checking for queue";
          unique_lock<mutex> lock(d->mutex);
          if(d->threads_queue.empty()) {
            WServer::instance()->log("notice") << "[thread-" << i << "]: queue empty, sleeping";
            boost::this_thread::sleep_for(boost::chrono::seconds(1));
            continue;
          }
          function = d->threads_queue.front();
          d->threads_queue.pop();
        }
        WServer::instance()->log("notice") << "[thread-" << i << "]: job found, running...";
        function();
        WServer::instance()->log("notice") << "[thread-" << i << "]: job found, done";
      }
    });
  }
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::post(ThreadPool::Function f)
{
  unique_lock<mutex> lock(d->mutex);
  d->threads_queue.push(f);
}

