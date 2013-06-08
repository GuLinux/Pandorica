#include <Wt/Dbo/Dbo>
#include "session.h"
#include "Models/models.h"
#include <media.h>

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

void User::rate(Wt::Dbo::ptr< User > userPtr, const Media& media, int rating, Wt::Dbo::Transaction& transaction) {
  MediaRatingPtr previousRating = transaction.session().find<MediaRating>()
    .where("user_id = ?").bind(userPtr.id())
    .where("media_id = ?").bind(media.uid());
  if(!previousRating) {
    transaction.session().add(new MediaRating{userPtr, media.uid(), rating});
    return;
  }
  previousRating.modify()->setRating(rating);
}

