#include "adduserdialog.h"
#include "session.h"
#include "authorizeduser.h"
#include <Wt/Dbo/QueryModel>
#include <Wt/WTableView>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>

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
  WPushButton *add = new WPushButton("Add");
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
