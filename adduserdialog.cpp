#include "adduserdialog.h"
#include "session.h"
#include "authorizeduser.h"
#include <Wt/Dbo/QueryModel>
#include <Wt/WTableView>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WTemplate>

using namespace Wt;
using namespace std;
using namespace boost;

class AuthorizedUserModel : public Dbo::QueryModel<AuthorizedUserPtr> {
public:
    AuthorizedUserModel(WObject* parent = 0) : Dbo::QueryModel<AuthorizedUserPtr>(parent) {}
    virtual any data(const WModelIndex& index, int role = Wt::DisplayRole) const;
};


any AuthorizedUserModel::data(const WModelIndex& index, int role) const
{
    any realData = Dbo::QueryModel< AuthorizedUserPtr >::data(index, role);
    if(index.column()==1 && realData.type() == typeid(int)) {
      return (any_cast<int>(realData) == AuthorizedUser::Admin) ? "Admin" : "Normal User";
    }
    return realData;
}


AddUserDialog::AddUserDialog(Session* session): WDialog(), _session(session)
{
  setTitleBarEnabled(true);
  setCaption("Add User");
  resize(640, 480);
  setClosable(true);
  setResizable(true);
  Dbo::QueryModel< AuthorizedUserPtr >* model = new AuthorizedUserModel();
  model->setQuery(session->find<AuthorizedUser>());
  model->addColumn("email");
  model->addColumn("role");
  WTableView *table = new WTableView();
  table->setColumn1Fixed(false);
  table->setColumnWidth(0, 200);
  table->setModel(model);
  WLineEdit *newUser = new WLineEdit();
  newUser->setStyleClass("input-medium");
  
  WRegExpValidator* validator = new WRegExpValidator("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}");
  validator->setMandatory(true);
  
  WPushButton *add = new WPushButton("Add");
  add->setStyleClass("btn");
  add->setEnabled(false);
  newUser->setValidator(validator);
  newUser->keyWentUp().connect([newUser,add](WKeyEvent){
    WValidator::State s = newUser->validate();
    add->setEnabled(s == WValidator::Valid);
  });
  add->clicked().connect([newUser,session, model](WMouseEvent) {
    Dbo::Transaction t(*session);
    session->add<AuthorizedUser>(new AuthorizedUser(newUser->text().toUTF8()));
    t.commit();
    model->reload();
  });
  
  WTemplate *form = new WTemplate("<form class=\"form-inline\" style=\"text-align: center\"><label>Email</label> ${emailEdit} ${addButton}</form>");
  form->bindWidget("emailEdit", newUser);
  form->bindWidget("addButton", add);
  contents()->addWidget(form);
  contents()->addWidget(table);
}

AddUserDialog::~AddUserDialog()
{

}
