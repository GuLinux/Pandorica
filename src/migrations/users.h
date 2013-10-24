#ifndef MIGRATIONS_USERS_H
#define MIGRATIONS_USERS_H
#include "Wt-Commons/migratedbo.h"
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
namespace Migrations
{
  namespace Users
  {

    CREATE_MIGRATION( AddInvitationFieldToUsersTable, "2013-10-25 07:59:01",
      {
	m.addColumn<boost::optional<std::string>>("user", "invited_email_address");
      }
    )
// migrations_placeholder
  }
}
#endif

