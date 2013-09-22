#ifndef COMMENTS_CONTAINER_WIDGET_PRIVATE
#define COMMENTS_CONTAINER_WIDGET_PRIVATE

#include "commentscontainerwidget.h"
#include <string>

class Session;
class CommentsContainerWidget::Private {
public:
  std::string videoId;
  Session *session;
  WContainerWidget *commentsContainer;
public:
  Private(std::string videoId, Session *session)
    : videoId(videoId), session(session) {}
};


#endif