/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
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

#include "semaphore.h"
#include "private/semaphore_p.h"
#include "utils/d_ptr_implementation.h"

using namespace std;
Semaphore::Private::Private(Semaphore* q) : q(q)
{
}

Semaphore::Semaphore(int occupied)
  : d(this)
{
  occupy(occupied);
}

void Semaphore::occupy(int number)
{
  unique_lock<mutex> l(d->m);
  d->locks = number;
}

void Semaphore::release(uint howmany)
{
  unique_lock<mutex> l(d->m);
  d->locks -= howmany;
  d->cv.notify_one();
//   d->cv.notify_all();
}

void Semaphore::wait()
{
  unique_lock<mutex> l(d->m);
  d->cv.wait(l, [this]{
    return d->locks <= 0;
  });
}

Semaphore::~Semaphore()
{
}

