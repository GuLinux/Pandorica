#ifndef AUTHORIZEDUSER_H_
#define AUTHORIZEDUSER_H_

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WGlobal>

class AuthorizedUser {
public:
  enum Role { NormalUser, Admin };
  AuthorizedUser(std::string email, Role role = NormalUser)
    : _email(email), _role(role) {}
  AuthorizedUser() {}
  ~AuthorizedUser() {}
  inline std::string email() const { return _email; }
  void setEmail(std::string email) { this->_email = email; }
  inline Role role() const { return _role; }
  void setRole(Role role) { this->_role = role; }
  
private:
  std::string _email;
  Role _role;
public:
    template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, _email, "email");
    Wt::Dbo::field(a, _role, "role");
  }
};

typedef Wt::Dbo::ptr<AuthorizedUser> AuthorizedUserPtr;

#endif // AUTHORIZEDUSER_H_
