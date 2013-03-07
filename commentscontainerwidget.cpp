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
#include "wt_helpers.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "streamingapp.h"

using namespace Wt;
using namespace boost;
using namespace std;
//                          content, last_updated, username, email
typedef boost::tuple<string, long,         string,    string> CommentTuple;

class CommentsContainerWidgetPrivate {
public:
  string videoId;
  Session *session;
  WContainerWidget *commentsContainer;
    WTextArea* newCommentContent;
public:
  CommentsContainerWidgetPrivate(string videoId, Session *session)
    : videoId(videoId), session(session) {}
    void notifyCommentAdded(Dbo::ptr< Comment > newComment);
    void emitCommentAddedSignal(string, int);
};


class CommentView : public WContainerWidget {
public:
  CommentView(CommentTuple data, WContainerWidget* parent = 0);
};

CommentView::CommentView(CommentTuple data, WContainerWidget* parent): WContainerWidget(parent)
{
  setStyleClass("row-fluid");
  string username = data.get<3>();
  string email = data.get<2>();
  WDateTime commentTime = WDateTime::fromTime_t(data.get<1>());
  setContentAlignment(AlignLeft);
  addWidget(WW(WContainerWidget).css("span5")
    .add(WW(WText, username).setInline(false))
    .add(WW(WText, email).setInline(false))
    .add(WW(WText, commentTime.toString()).setInline(false)));
  
  addWidget(WW(WText,data.get<0>()).css("span7") );
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
  query.orderBy("last_updated DESC");
  
  addWidget(d->newCommentContent = new WTextArea());
  WPushButton* insertComment = WW(WPushButton, "AddComment").onClick([this,videoId](WMouseEvent){
    Comment *comment = new Comment(videoId, d->session->user(), d->newCommentContent->text().toUTF8());
    Dbo::Transaction t(*d->session);
    Dbo::ptr< Comment > newComment = d->session->add(comment);
    t.commit();
    d->notifyCommentAdded(newComment);
  });
  addWidget(insertComment);
  addWidget(d->commentsContainer = WW(WContainerWidget).css("container-fluid") );

  Dbo::Transaction t(*d->session);
  for(CommentTuple comment : query.resultList())
    d->commentsContainer->addWidget(new CommentView(comment) );
}


typedef boost::function<void (const string&, int)> CommentEventCallback;


void CommentsContainerWidgetPrivate::notifyCommentAdded(Dbo::ptr< Comment > newComment)
{
  Dbo::Transaction t(*session);
  Dbo::Query<string> sessionsQuery = session->query<string>("SELECT session_id FROM session_info WHERE session_ended = 0");
  for(string sessionId : sessionsQuery.resultList()) {
  }
}

CommentsContainerWidget::~CommentsContainerWidget()
{
  delete d;
}

