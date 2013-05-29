#ifndef COMMENT_H_
#define COMMENT_H_

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WGlobal>
#include <Wt/WDateTime>
#include <Wt/Auth/User>

class User;

class Comment {
public:
  Comment() {}
  Comment(std::string mediaId, Wt::Dbo::ptr<User> user, std::string content)
    : _mediaId(mediaId), _user(user), _content(content), _lastUpdated(Wt::WDateTime::currentDateTime().toTime_t()) {}
  ~Comment() {}
  std::string content() const { return _content; }
  std::string mediaId() const { return _mediaId; }
  Wt::WDateTime lastUpdated() const { return Wt::WDateTime::fromTime_t(_lastUpdated); }
  Wt::Dbo::ptr<User> user() const { return _user; }
private:
  std::string _content;
  std::string _mediaId;
  long _lastUpdated;
  Wt::Dbo::ptr<User> _user;
public:
    template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, _content, "content");
    Wt::Dbo::field(a, _mediaId, "media_id");
    Wt::Dbo::field(a, _lastUpdated, "last_updated");
    Wt::Dbo::belongsTo(a, _user, "user");
  }
};

typedef Wt::Dbo::ptr<Comment> CommentPtr;


#endif // COMMENT_H_
