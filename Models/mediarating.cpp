#include "Models/models.h"
#include "media.h"
#include <boost/math/special_functions.hpp>


int MediaRating::ratingFor(const Media& media, Wt::Dbo::Transaction& transaction)
{
  double ratingsCount = transaction.session().query<double>("select count(id) from media_rating WHERE media_id = ?").bind(media.uid());
  if(ratingsCount == 0) return -1;
  double ratings = transaction.session().query<double>("select sum(rating) from media_rating WHERE media_id = ?").bind(media.uid()).resultValue();
  return boost::math::round(ratings / ratingsCount);
}
