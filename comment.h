#ifndef COMMENT_H_
#define COMMENT_H_

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WGlobal>
#include <Wt/WDateTime>
#include <Wt/Auth/User>


class Comment {
public:
  Comment() {}
  ~Comment() {}
  
private:
  std::string _content;
  std::string _videoId;
  long _lastUpdated;
  Wt::Auth::User _user;
public:
    template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, _content, "content");
    Wt::Dbo::field(a, _videoId, "video_id");
    Wt::Dbo::field(a, _lastUpdated, "last_updated");
  }
};

typedef Wt::Dbo::ptr<Comment> CommentPtr;


#endif // COMMENT_H_
