#ifndef MEDIA_RATING_H
#define MEDIA_RATING_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>

class Media;
class User;
class MediaRating {

public:
  MediaRating() = default;
  MediaRating(Wt::Dbo::ptr<User> user, std::string mediaId, int rating)
    : _user(user), _mediaId(mediaId), _rating(rating) {}
  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::belongsTo(a, _user, "user");
    Wt::Dbo::field(a, _rating, "rating");
    Wt::Dbo::field(a, _mediaId, "media_id");
  }
  
  Wt::Dbo::ptr<User> user() { return _user; }
  std::string mediaId() const{ return _mediaId; }
  int rating() const { return _rating; }
  static int ratingFor(const Media &media, Wt::Dbo::Transaction &transaction);
  void setRating(int newRating) { _rating = newRating; }
private:
  Wt::Dbo::ptr<User> _user;
  std::string _mediaId;
  int _rating;
};

typedef Wt::Dbo::ptr<MediaRating> MediaRatingPtr;

#endif