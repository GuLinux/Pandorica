/***********************************************************************
Copyright (c) 2015 "Marco Gulino <marco.gulino@gmail.com>"

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
#ifndef COLLECTION_ITEM_PROPERTIES_H
#define COLLECTION_ITEM_PROPERTIES_H

#include <Wt/Auth/User>
#include <Wt/Dbo/Types>
#include <Wt/WGlobal>
#include <boost/filesystem.hpp>

class Media;
class MediaRating;
class Group;
class Comment;
namespace dbo = Wt::Dbo;
class CollectionItemProperty;
typedef dbo::ptr<CollectionItemProperty> CollectionItemPropertyPtr;
class CollectionItemProperty {
public:
  CollectionItemProperty() {}
  CollectionItemProperty(const std::string &name, const std::string &item_ref, const std::string &value)
    : _name(name), _item_ref(item_ref), _value(value) {}
  std::string name() const { return _name; }
  std::string item_ref() const { return _item_ref; }
  std::string value() const { return _value; }
  void value(const std::string &value) { _value = value; }
  template<typename Action>
  void persist(Action& a) {
    Wt::Dbo::field(a, _name, "name");
    Wt::Dbo::field(a, _item_ref, "item_ref");
    Wt::Dbo::field(a, _value, "value");
  }
  static CollectionItemPropertyPtr label(const boost::filesystem::path &path, const Wt::Dbo::Transaction &transaction) {
    return transaction.session().find<CollectionItemProperty>().where("name = 'fs-label' AND item_ref = ?").bind(path.string());
  }
  
  static void label(const boost::filesystem::path &path, const std::string &label, const Wt::Dbo::Transaction &transaction) {
    transaction.session().execute("DELETE FROM collection_item_property WHERE item_ref = ?").bind(path.string());
    if(!label.empty())
      transaction.session().add(new CollectionItemProperty("fs-label", path.string(), label));
  }
  
private:
  std::string _name;
  std::string _item_ref;
  std::string _value;
};
#endif