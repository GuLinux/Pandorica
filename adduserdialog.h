#ifndef ADDUSERDIALOG_H
#define ADDUSERDIALOG_H

#include <Wt/WDialog>

class Session;
class AddUserDialog : public Wt::WDialog
{
public:
  AddUserDialog(Session *session, std::string email = std::string());
  virtual ~AddUserDialog();
private:
  Session *_session;
};

#endif // ADDUSERDIALOG_H
