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




#ifndef SETTING_H
#define SETTING_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>

#include <string>
#include <mutex>
#include <boost/lexical_cast.hpp>

class Setting
{
  class Session : public Wt::Dbo::Session {
  public: 
    Session();
    static std::shared_ptr<std::unique_lock<std::mutex>> writeLock();
  private:
    std::unique_ptr<Wt::Dbo::SqlConnection> connection;
  };
public:
    Setting();
    ~Setting();
    enum KeyName {
      QuitPassword,
      DatabaseType,
      PostgreSQL_Hostname,
      PostgreSQL_Port,
      PostgreSQL_Database,
      PostgreSQL_Application,
      PostgreSQL_Username,
      PostgreSQL_Password,
      GoogleBrowserDeveloperKey,
      DatabaseVersion,
      MediaDirectories,
      ThreadPoolThreads,
      AuthEmailName,
      AuthEmailAddress,
      PandoricaSetup,
      PandoricaMode,
      AuthenticationMode,
      EmailVerificationMandatory,
    };

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, _key, "key");
        Wt::Dbo::field(a, _value, "value");
    }


    
    template<class Type>
    static void write(KeyName key, const Type &value) {
      write_values(key, {boost::lexical_cast<std::string>(value)} );
    }
    
    template<typename Type, class Cont>
    static void write(KeyName key, const Cont &values) {
      std::vector<std::string> string_values;
      std::transform(std::begin(values), std::end(values), std::back_inserter(string_values), [=](const Type &t){ return boost::lexical_cast<std::string>(t); });
      write_values(key, string_values);
    }
    
    template<class Type>
    static Type value(KeyName key, const Type &defaultValue = Type{} ) {
      auto _values = values<Type>(key);
      if(_values.empty())
        return defaultValue;
      return _values[0];
    }
    
    template<class Type>
    static std::vector<Type> values(KeyName key) {
      auto _values = get_values(key);
      std::vector<Type> collection;
      std::transform(_values.begin(), _values.end(), std::back_inserter(collection), [=](const std::string &s) { return boost::lexical_cast<Type>(s); });
      return collection;
    }


    
private:
    std::string _key;
    std::string _value;
    static std::string keyName(KeyName key);
    
    static std::vector<std::string> get_values(KeyName key);
    static void write_values(KeyName key, const std::vector<std::string> &values);
    static std::map<KeyName, std::vector<std::string>> values_cache;
};


#endif // SETTING_H
