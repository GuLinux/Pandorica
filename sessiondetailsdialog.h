#ifndef SESSIONDETAILSDIALOG_H
#define SESSIONDETAILSDIALOG_H

#include <Wt/WDialog>

class Session;

class SessionDetailsDialog : public Wt::WDialog
{

public:
    SessionDetailsDialog(std::string id, Session *session);
};

#endif // SESSIONDETAILSDIALOG_H
