#ifndef SESSIONPRIVATE_H
#define SESSIONPRIVATE_H
#include <Wt/Auth/Login>
#include "session.h"

namespace Wt {
  namespace Dbo {
    class SqlConnection;
  }
}

namespace StreamingPrivate {
class SessionPrivate {
public:
    void createConnection();
    Wt::Dbo::SqlConnection *connection;
    UserDatabase *users;
    Wt::Auth::Login login;
};
}

#endif
