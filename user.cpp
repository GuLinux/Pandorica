#include "user.h"
#include "group.h"
#include <Wt/Dbo/Dbo>
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"
#include "mediaattachment.h"

using namespace std;

bool User::isAdmin() const
{
  for(auto group: groups) {
    if(group->isAdmin()) return true;
  }
  return false;
}

list<string> User::allowedPaths() const
{
  list<string> paths;
  for(auto group: groups) {
    paths.merge(group->allowedPaths());
  }
  return paths;
}
