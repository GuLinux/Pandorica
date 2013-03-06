#ifndef COMMENTSCONTAINERWIDGET_H
#define COMMENTSCONTAINERWIDGET_H

#include <Wt/WContainerWidget>

class Session;

class CommentsContainerWidget : public Wt::WContainerWidget
{

public:
    CommentsContainerWidget(std::string videoId, Session *session, Wt::WContainerWidget* parent = 0);
    virtual ~CommentsContainerWidget();
private:
  std::string videoId;
  Session *session;
};

#endif // COMMENTSCONTAINERWIDGET_H
