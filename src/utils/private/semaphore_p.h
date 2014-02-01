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

#ifndef SEMAPHORE_P_H
#define SEMAPHORE_P_H
#include "semaphore.h"
#include <condition_variable>

class Semaphore::Private
{
public:
    Private(Semaphore* q);
    std::mutex m;
    std::mutex m_i;
    std::condition_variable cv;
    int32_t locks;
private:
    class Semaphore* const q;
};
#endif // SEMAPHORE_P_H
