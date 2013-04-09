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
    dbo::hasMany(a, _users, dbo::ManyToMany, "groups_users");
    dbo::field(a, _groupName, "group_name");
  }
private:
  dbo::collection< dbo::ptr<User> > _users;
  std::string _groupName;
};

#endif // GROUP_H
