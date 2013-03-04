#ifndef ADDUSERDIALOG_H
#define ADDUSERDIALOG_H

#include <Wt/WDialog>

class Session;
class AddUserDialog : public Wt::WDialog
{
public:
  AddUserDialog(Session *session);
  virtual ~AddUserDialog();
private:
  Session *_session;
};

#endif // ADDUSERDIALOG_H
