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
#include <map>
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
//     WServer::instance()->log("warning") << "error creating new database: " << e.what();
  }
}

shared_ptr<unique_lock<mutex>> Setting::Session::writeLock()
{
  static mutex write_mutex;
  return make_shared<unique_lock<mutex>>(write_mutex);
}

string Setting::keyName(Setting::KeyName key)
{
  static map<KeyName,string> keyNames {
    {QuitPassword, "quit-password"},
    {DatabaseType, "database-type"},
    {PostgreSQL_Hostname, "postgresql-hostname"},
    {PostgreSQL_Database, "postgresql-database"},
    {PostgreSQL_Port, "postgresql-port"},
    {PostgreSQL_Application, "postgresql-application"},
    {PostgreSQL_Username, "postgresql-username"},
    {PostgreSQL_Password, "postgresql-password"},
    {GoogleBrowserDeveloperKey, "google-browser-dveloper-key"},
    {DatabaseVersion, "db_version"},
    {MediaDirectories, "media_directories"},
    {EmailVerificationMandatory, "email_verification_mandatory"},
    {ThreadPoolThreads, "threadpool_threads_count"},
    {AuthEmailName, "auth_mail_sender_name"},
    {AuthEmailAddress, "auth_mail_sender_address"},
    {PandoricaSetup, "pandorica_setup"},
    {PandoricaMode, "pandorica_mode"},
    {AuthenticationMode, "authentication_mode"},
  };
  return keyNames[key];
}

Setting::Setting()
{

}

Setting::~Setting()
{

}

#include <boost/algorithm/string.hpp>
map<Setting::KeyName, vector<string>> Setting::values_cache {};

vector< string > Setting::get_values(Setting::KeyName key)
{
  if(values_cache.count(key) == 0) {
    vector<string> _values;
    Session session;
    Wt::Dbo::Transaction t(session);
    auto dboValues = session.find<Setting>().where("\"key\" = ?").bind(keyName(key)).resultList();
    transform(dboValues.begin(), dboValues.end(), back_inserter(_values), [=](const Dbo::ptr<Setting> &s){ return s->_value; });
    values_cache[key] = _values;
  }
  return values_cache[key];
}

void Setting::write_values(Setting::KeyName key, const vector< string >& values)
{
  Session session;
  Wt::Dbo::Transaction t(session);
  auto writeLock = session.writeLock();
  session.execute("DELETE FROM settings WHERE \"key\" = ?").bind(keyName(key));
  for(auto value: values) {
    Setting *setting = new Setting;
    setting->_key = keyName(key);
    setting->_value = value;
    session.add(setting);
  }
  values_cache[key] = values;
}

