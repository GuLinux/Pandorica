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


#include "Models/models.h"
#include "media/media.h"
#include <boost/math/special_functions.hpp>


Ratings MediaRating::ratingFor(const Media& media, Wt::Dbo::Transaction& transaction)
{
  long ratingsCount = transaction.session().query<long>("select count(id) from media_rating WHERE media_id = ?").bind(media.uid());
  if(ratingsCount == 0) return {0, -1};
  double ratings = transaction.session().query<double>("select sum(rating) from media_rating WHERE media_id = ?").bind(media.uid()).resultValue();
  return {ratingsCount, (int) (boost::math::round(ratings / (double) ratingsCount)) };
}
