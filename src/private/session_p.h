/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/


#ifndef SESSIONPRIVATE_H
#define SESSIONPRIVATE_H
#include <Wt/Auth/Login>
#include "session.h"
#include <mutex>

namespace Wt {
  namespace Dbo {
    class SqlConnection;
  }
}

class Session::Private {
public:
    Private(Session *q) : q(q) {}
    void createConnection();
    std::string psqlConnectionString() const;
    std::unique_ptr<Wt::Dbo::SqlConnection> connection;
    UserDatabase *users = 0;
    Wt::Auth::Login login;
    std::shared_ptr<std::mutex> mutex;
    Session *q;
};

#endif
