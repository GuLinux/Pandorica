#include "commentscontainerwidget.h"
#include "session.h"

using namespace Wt;
using namespace boost;
using namespace std;
CommentsContainerWidget::CommentsContainerWidget(string videoId, Session* session, WContainerWidget* parent)
  : WContainerWidget(parent), videoId(videoId), session(session)
{

}

CommentsContainerWidget::~CommentsContainerWidget()
{

}

