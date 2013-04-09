#include "user.h"
#include "group.h"
#include <Wt/Dbo/Dbo>
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"
#include "mediaattachment.h"

bool User::isAdmin() const
{
  for(auto group: groups) {
    if(group->isAdmin()) return true;
  }
  return false;
}
