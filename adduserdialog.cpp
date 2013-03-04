#include "adduserdialog.h"
#include "session.h"
#include "authorizeduser.h"
#include <Wt/Dbo/QueryModel>
#include <Wt/WTableView>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>

using namespace Wt;
using namespace std;
using namespace boost;

AddUserDialog::AddUserDialog(Session* session): WDialog(), _session(session)
{
  setTitleBarEnabled(true);
  setCaption("Add User");
  resize(640, 480);
  setClosable(true);
  setResizable(true);
  Dbo::QueryModel< AuthorizedUserPtr >* model = new Dbo::QueryModel<AuthorizedUserPtr>();
  model->setQuery(session->find<AuthorizedUser>());
  model->addColumn("email");
  model->addColumn("role");
  WTableView *table = new WTableView();
  table->setColumn1Fixed(false);
  table->setColumnWidth(0, 200);
  table->setModel(model);
  WLineEdit *newUser = new WLineEdit();
  
  WRegExpValidator* validator = new WRegExpValidator("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}$");
  validator->setMandatory(true);
  
  WPushButton *add = new WPushButton("Add");
  add->setEnabled(false);
  newUser->setValidator(validator);
  newUser->keyWentUp().connect([newUser,add](WKeyEvent){
    newUser->validate();
  });
  newUser->validated().connect([add](WValidator::Result r, NoClass, NoClass, NoClass, NoClass, NoClass){
    add->setEnabled(r.state() == WValidator::Valid);
  });
  add->clicked().connect([newUser,session, model](WMouseEvent) {
    Dbo::Transaction t(*session);
    session->add<AuthorizedUser>(new AuthorizedUser(newUser->text().toUTF8()));
    t.commit();
    model->reload();
  });
  contents()->addWidget(newUser);
  contents()->addWidget(add);
  contents()->addWidget(table);
}

AddUserDialog::~AddUserDialog()
{

}
