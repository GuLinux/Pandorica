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

#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <utils/d_ptr.h>
#include <functional>
#include <mutex>

class ThreadPool
{
public:
  ~ThreadPool();
  static std::shared_ptr<ThreadPool> instance(int max = 10);
  typedef std::function<void()> Function;
  void post(Function f);
  static std::shared_ptr<std::unique_lock<std::mutex>> lock(const std::string &name);
private:
  ThreadPool(int max);
  D_PTR;
};

#endif // THREADPOOL_H
