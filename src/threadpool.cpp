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

ThreadPool::Private::Private(int max, ThreadPool* q) : work(ioService), q(q)
{
  for(int i=0; i<max; i++)
    threadGroup.create_thread(boost::bind(&boost::asio::io_service::run, &ioService));
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

}

ThreadPool::~ThreadPool()
{
  d->ioService.stop();
  d->threadGroup.join_all();
}

void ThreadPool::post(ThreadPool::Function f)
{
  d->ioService.post(f);
}

