/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/


#include <Wt/Dbo/Dbo>
#include "session.h"
#include "Models/models.h"
#include "media/media.h"

using namespace std;

bool User::isAdmin(Wt::Dbo::Transaction &transaction) const
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

