#ifndef COMMENTSCONTAINERWIDGET_H
#define COMMENTSCONTAINERWIDGET_H

#include <Wt/WContainerWidget>

class Session;
namespace StreamingPrivate {
  class CommentsContainerWidgetPrivate;
}

class CommentsContainerWidget : public Wt::WContainerWidget
{

public:
    CommentsContainerWidget(std::string videoId, Session *session, Wt::WContainerWidget* parent = 0);
    virtual ~CommentsContainerWidget();
private:
  StreamingPrivate::CommentsContainerWidgetPrivate *const d;
};

#endif // COMMENTSCONTAINERWIDGET_H
