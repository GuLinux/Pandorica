#include "Models/models.h"
#include "media.h"
#include <boost/math/special_functions.hpp>


Ratings MediaRating::ratingFor(const Media& media, Wt::Dbo::Transaction& transaction)
{
  long ratingsCount = transaction.session().query<long>("select count(id) from media_rating WHERE media_id = ?").bind(media.uid());
  if(ratingsCount == 0) return {0, -1};
  double ratings = transaction.session().query<double>("select sum(rating) from media_rating WHERE media_id = ?").bind(media.uid()).resultValue();
  return {ratingsCount, boost::math::round(ratings / (double) ratingsCount)};
}
