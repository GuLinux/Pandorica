#ifndef GROUP_H
#define GROUP_H

#include <Wt/Auth/User>
#include <Wt/Dbo/Types>
#include <Wt/WGlobal>

class User;
class Group;
namespace dbo = Wt::Dbo;

class GroupPath {
public:
  GroupPath() = default;
  GroupPath(std::string path) : _path(path) {}
  template<class Action>
  void persist(Action& a) {
    dbo::field(a, _path, "path");
    dbo::belongsTo(a, _group, "group");
  }
  inline std::string path() const { return _path; }
private:
  std::string _path;
  dbo::ptr<Group> _group;
};

class Group
{
public:
  Group() = default;
  Group(std::string groupName, bool isAdmin = false);
  template<class Action>
  void persist(Action& a) {
    dbo::hasMany(a, users, dbo::ManyToMany, "groups_users");
    dbo::field(a, _groupName, "group_name");
    dbo::field(a, _isAdmin, "is_admin");
    dbo::hasMany(a, groupPaths, dbo::ManyToOne, "group");
  }
  inline std::string groupName() const { return _groupName; }
  inline void setGroupName(const std::string &groupName) { _groupName = groupName; }
  inline bool isAdmin() const { return _isAdmin; }
  
  dbo::collection< dbo::ptr<User> > users;
  dbo::collection< dbo::ptr<GroupPath> > groupPaths;
private:
  std::string _groupName;
  bool _isAdmin;
};

typedef dbo::ptr<Group> GroupPtr;

#endif // GROUP_H
