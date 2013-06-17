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
#include <boost/lexical_cast.hpp>

class Setting
{
public:
    Setting();
    ~Setting();
  static std::string deployType();
  static std::string deployPath(const std::string &path);
  static std::string secureDownloadPassword();
  static std::string useCache();
  static std::string cacheDirectory();
  static std::string cacheDeployPath();

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, _key, "key");
        Wt::Dbo::field(a, _value, "value");
    }


    
    template<class Type>
    static void write(std::string key, Type value, Wt::Dbo::Transaction &transaction) {
      transaction.session().execute("DELETE FROM settings WHERE \"key\" = ?").bind(key);
      Setting *setting = new Setting;
      setting->_key = key;
      setting->_value = boost::lexical_cast<std::string>(value);
      transaction.session().add(setting);
    }
    
    template<typename Type, class Cont>
    static void write(std::string key, Cont values, Wt::Dbo::Transaction &transaction) {
      transaction.session().execute("DELETE FROM settings WHERE \"key\" = ?").bind(key);
      for(Type value: values) {
        Setting *setting = new Setting;
        setting->_key = key;
        setting->_value = boost::lexical_cast<std::string>(value);
        transaction.session().add(setting);
      }
    }
    
    template<class Type>
    static Type value(std::string key, Wt::Dbo::Transaction &transaction, Type defaultValue = Type{} ) {
      Wt::Dbo::ptr<Setting> setting = transaction.session().find<Setting>().where("\"key\" = ?").bind(key);
      if(!setting)
        return defaultValue;
      return boost::lexical_cast<Type>(setting->_value);
    }
    
    template<class Type>
    static std::vector<Type> values(std::string key, Wt::Dbo::Transaction &transaction) {
      std::vector<Type> collection;
      Wt::Dbo::collection<Wt::Dbo::ptr<Setting>> dboValues = transaction.session().find<Setting>().where("\"key\" = ?").bind(key);
      std::transform(dboValues.begin(), dboValues.end(), std::back_insert_iterator<std::vector<Type>>(collection),
                     [=](Wt::Dbo::ptr<Setting> setting) { return boost::lexical_cast<Type>(setting->_value); });
      return collection;
    }

private:
private:
    std::string _key;
    std::string _value;
};


#endif // SETTING_H
