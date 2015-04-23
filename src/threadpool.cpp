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

ThreadPool::Private::Private(ThreadPool* q) : /* work(ioService), */ q(q)
{
  string threads = boost::lexical_cast<string>(ioService.threadCount());
  if(WServer::instance()->readConfigurationProperty("threadpool_threads_count", threads)) {
    ioService.setThreadCount(boost::lexical_cast<int>(threads));
  }
  ioService.start();
}

ThreadPool::Private::~Private()
{
  ioService.stop();
}

std::shared_ptr<ThreadPool> ThreadPool::instance()
{
  static shared_ptr<ThreadPool> threadpool(new ThreadPool);
  return threadpool;
}


std::shared_ptr<std::unique_lock<std::mutex>> ThreadPool::lock(const std::string &name) {
  static std::mutex locks_mutex;
  std::unique_lock<std::mutex> lock_locks_mutex(locks_mutex);
  static map<string,shared_ptr<mutex>> locks;
  if(locks.count(name) == 0) {
    locks[name] = make_shared<mutex>();
  }
  return make_shared<unique_lock<mutex>>(*locks[name]);
}


ThreadPool::ThreadPool()
    : d(this)
{
}

ThreadPool::~ThreadPool()
{
  d->ioService.stop();
}

void ThreadPool::post(ThreadPool::Function f)
{
  d->ioService.post(f);
}

