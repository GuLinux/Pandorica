#include "group.h"
using namespace std;

Group::Group(std::string groupName, bool isAdmin)
: _groupName(groupName), _isAdmin(isAdmin)
{
}


list<string> Group::allowedPaths() const
{
  if(isAdmin())
    return {"/"};
  list<string> paths;
  for(auto path: groupPaths) {
    paths.push_back(path->path());
  }
  return paths;
}
