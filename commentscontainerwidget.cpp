#include "commentscontainerwidget.h"
#include "session.h"
#include "comment.h"
#include <Wt/Dbo/QueryModel>
#include <Wt/WApplication>
#include <Wt/WViewWidget>
#include <Wt/WTextArea>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <boost/function.hpp>
#include "Wt-Commons/wt_helpers.h"

#include "sessioninfo.h"
#include "sessiondetails.h"
#include "streamingapp.h"

using namespace Wt;
using namespace boost;
using namespace std;
//                          content, last_updated, username, email
typedef boost::tuple<string, long,         string,    string> CommentTuple;

typedef boost::function<void(string, long)> CommentAddedCallback;

class CommentViewers {
public:
  void addClient(string sessionId, CommentAddedCallback commentAddedCallback) { _viewers[sessionId] = commentAddedCallback; }
  void removeClient(string sessionId) { _viewers.erase(sessionId); }
  void commentAdded(string videoId, long commentId);
  
private:
  map<string,CommentAddedCallback> _viewers;
};

void CommentViewers::commentAdded(string videoId, long int commentId)
{
  for(pair<string,CommentAddedCallback> sessions: _viewers) {
    WServer::instance()->post(sessions.first, boost::bind(sessions.second, videoId, commentId));
  }
}



CommentViewers commentViewers;

class CommentsContainerWidgetPrivate {
public:
  string videoId;
  Session *session;
  WContainerWidget *commentsContainer;
public:
  CommentsContainerWidgetPrivate(string videoId, Session *session)
    : videoId(videoId), session(session) {}
};


class CommentView : public WContainerWidget {
public:
  CommentView(CommentTuple data, WContainerWidget* parent = 0);
};

CommentView::CommentView(CommentTuple data, WContainerWidget* parent): WContainerWidget(parent)
{
  setStyleClass("row-fluid comment-box");
  string username = data.get<3>();
  string email = data.get<2>();
  WDateTime commentTime = WDateTime::fromTime_t(data.get<1>());
  string commentText = data.get<0>();
  setContentAlignment(AlignLeft);
  addWidget(WW(WContainerWidget).css("span4 label label-success comment-header")
    .add(WW(WText, username).setInline(false))
    .add(WW(WText, email).setInline(false))
    .add(WW(WText, commentTime.toString()).setInline(false)));
  addWidget(WW(WText,WString::fromUTF8(commentText)).css("span8 well comment-text") );
}


CommentsContainerWidget::CommentsContainerWidget(string videoId, Session* session, WContainerWidget* parent)
  : WContainerWidget(parent), d(new CommentsContainerWidgetPrivate(videoId, session))
{
  string querysql = "select content,last_updated,\
      auth_info.email as email,\
    auth_identity.identity as identity\
    from comment\
    inner join auth_info on comment.user_id = auth_info.user_id\
    inner join auth_identity on auth_info.id = auth_identity.auth_info_id\
  ";
  wApp->log("notice") << "comments query: " << querysql;
  auto query = d->session->query<CommentTuple>(querysql);
  query.where("video_id = ?").bind(videoId);
  query.where("auth_identity.provider = 'loginname'");
  query.orderBy("last_updated DESC");
  addWidget(WW(WText, "Comments").css("label").setInline(false));
  
  WTextArea* newCommentContent = new WTextArea();
  newCommentContent->setRows(3);
  newCommentContent->setWidth(500);
  newCommentContent->setInline(false);
  WPushButton* insertComment = WW(WPushButton, "Add Comment").css("btn btn-primary btn-small").onClick([this,videoId,newCommentContent](WMouseEvent){
    Comment *comment = new Comment(videoId, d->session->user(), newCommentContent->text().toUTF8());
    Dbo::Transaction t(*d->session);
    Dbo::ptr< Comment > newComment = d->session->add(comment);
    t.commit();
    newCommentContent->setText("");
    commentViewers.commentAdded(videoId, newComment.id());
  });
  
  addWidget(WW(WContainerWidget).css("add-comment-box").add(newCommentContent).add(insertComment).setContentAlignment(AlignCenter));
  
  addWidget(d->commentsContainer = WW(WContainerWidget).css("container-fluid") );

  Dbo::Transaction t(*d->session);
  for(CommentTuple comment : query.resultList())
    d->commentsContainer->addWidget(new CommentView(comment) );
  
  auto commentAdded = [this,videoId,querysql] (string commentVideoId, long commentId) {
    if(commentVideoId != videoId) return;
    Dbo::Transaction t(*d->session);
    auto query = d->session->query<CommentTuple>(querysql).where("comment.id = ?").bind(commentId);
    query.where("auth_identity.provider = 'loginname'");
    d->commentsContainer->insertWidget(0, new CommentView(query.resultValue()));
    d->commentsContainer->refresh();
    wApp->triggerUpdate();
  };
  
  commentViewers.addClient(wApp->sessionId(), commentAdded);
}


CommentsContainerWidget::~CommentsContainerWidget()
{
  commentViewers.removeClient(wApp->sessionId());
  delete d;
}

