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



#include "setting.h"
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/WServer>
#include <boost/filesystem.hpp>
#include "settings.h"
using namespace std;
using namespace Wt;

Setting::Session::Session()
{
  connection.reset(new Dbo::backend::Sqlite3(Settings::sqlite3DatabasePath("Pandorica_Settings.sqlite")));
  connection->setProperty("show-queries", "false");
  connection->executeSql("PRAGMA journal_mode=WAL;");
  setConnection(*connection);
  mapClass<Setting>("settings");
  try {
    createTables();
  } catch(std::exception &e) {
    WServer::instance()->log("warning") << "error creating new database: " << e.what();
  }
}

shared_ptr<unique_lock<mutex>> Setting::Session::writeLock()
{
  static mutex write_mutex;
  return make_shared<unique_lock<mutex>>(write_mutex);
}



Setting::Setting()
{

}

Setting::~Setting()
{

}
