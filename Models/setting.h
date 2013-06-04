/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

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


    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, _key, "key");
        Wt::Dbo::field(a, _value, "value");
    }


    
    template<class Type>
    static void write(std::string key, Type value, Wt::Dbo::Transaction &transaction) {
      transaction.session().execute("DELETE FROM settings WHERE key = ?").bind(key);
      Setting *setting = new Setting;
      setting->_key = key;
      setting->_value = boost::lexical_cast<std::string>(value);
      transaction.session().add(setting);
    }
    
    template<typename Type, class Cont>
    static void write(std::string key, Cont values, Wt::Dbo::Transaction &transaction) {
      transaction.session().execute("DELETE FROM settings WHERE key = ?").bind(key);
      for(Type value: values) {
        Setting *setting = new Setting;
        setting->_key = key;
        setting->_value = boost::lexical_cast<std::string>(value);
        transaction.session().add(setting);
      }
    }
    
    template<class Type>
    static Type value(std::string key, Wt::Dbo::Transaction &transaction, Type defaultValue = Type{} ) {
      Wt::Dbo::ptr<Setting> setting = transaction.session().find<Setting>().where("key = ?").bind(key);
      if(!setting)
        return defaultValue;
      return boost::lexical_cast<Type>(setting->_value);
    }
    
    template<class Type>
    static std::vector<Type> values(std::string key, Wt::Dbo::Transaction &transaction) {
      std::vector<Type> collection;
      Wt::Dbo::collection<Wt::Dbo::ptr<Setting>> dboValues = transaction.session().find<Setting>().where("key = ?").bind(key);
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
