#include "latestcommentsdialog.h"
#include "session.h"
#include "comment.h"

#include "Wt-Commons/wt_helpers.h"
#include "mediacollection.h"
#include <Wt/Auth/Dbo/AuthInfo>

#include "sessioninfo.h"
#include "sessiondetails.h"
#include <Wt/WText>
#include <Wt/WAnchor>
#include <Wt/WPushButton>
#include "user.h"


using namespace Wt;

LatestCommentsDialog::LatestCommentsDialog(Session* session, MediaCollection *mediaCollection, WObject* parent): WDialog{parent}
{
  setResizable(true);
  setWindowTitle(wtr("menu.latest.comments"));
  setClosable(false);
  setTransient(true);
  setMaximumSize(700, WLength::Auto);
  Dbo::Transaction t(*session);
  Dbo::collection<CommentPtr> latestComments = session->find<Comment>().orderBy("last_updated desc").limit(5);
  for(CommentPtr comment: latestComments) {
    WContainerWidget* commentWidget = new WContainerWidget;
    Media media = mediaCollection->media(comment->videoId());
      
    WContainerWidget *header = WW<WContainerWidget>();
    header->setContentAlignment(AlignCenter);
    
    WAnchor *videoLink = WW<WAnchor>("", media.title(session)).css("link-hand label label-info comment-box-element");
    header->addWidget(videoLink);
    Dbo::ptr<AuthInfo> authInfo = session->find<AuthInfo>().where("user_id = ?").bind(comment->user().id());
    header->addWidget(WW<WText>(WString("{1} ({2})").arg(authInfo->identity("loginname")).arg(comment->lastUpdated().toString()))
    .css("label label-success comment-box-element"));
    commentWidget->addWidget(header);
    videoLink->clicked().connect([=](WMouseEvent){
      _mediaClicked.emit(media);
      accept();
    });
    commentWidget->addWidget(WW<WText>(WString::fromUTF8(comment->content())).css("well comment-text comment-box-element").setInline(false));
    contents()->addWidget(WW<WContainerWidget>().css("comment-text").add(commentWidget));
  }
  footer()->addWidget(WW<WPushButton>(wtr("close-button")).css("btn btn-primary").onClick([=](WMouseEvent){ accept(); }));
}

LatestCommentsDialog::~LatestCommentsDialog()
{
}

Signal<Media>& LatestCommentsDialog::mediaClicked()
{
  return _mediaClicked;
}

