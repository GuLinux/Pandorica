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


#ifndef MEDIA_RATING_H
#define MEDIA_RATING_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>

class Media;
class User;

struct Ratings {
  long users;
  int ratingAverage;
};

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
  static Ratings ratingFor(const Media &media, Wt::Dbo::Transaction &transaction);
  void setRating(int newRating) { _rating = newRating; }
private:
  Wt::Dbo::ptr<User> _user;
  std::string _mediaId;
  int _rating;
};

typedef Wt::Dbo::ptr<MediaRating> MediaRatingPtr;

#endif