#ifndef GROUP_H
#define GROUP_H

#include <Wt/Auth/User>
#include <Wt/Dbo/Types>
#include <Wt/WGlobal>

class User;
namespace dbo = Wt::Dbo;

class Group
{
public:
  template<class Action>
  void persist(Action& a) {
    dbo::hasMany(a, users, dbo::ManyToMany, "groups_users");
    dbo::field(a, _groupName, "group_name");
    dbo::field(a, _isAdmin, "is_admin");
  }
  inline std::string groupName() const { return _groupName; }
  inline void setGroupName(const std::string &groupName) { _groupName = groupName; }
  inline bool isAdmin() const { return _isAdmin; }
  
  dbo::collection< dbo::ptr<User> > users;
  
private:
  std::string _groupName;
  bool _isAdmin;
};

#endif // GROUP_H
