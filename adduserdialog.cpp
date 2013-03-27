#include "adduserdialog.h"
#include "session.h"
#include "authorizeduser.h"
#include "customitemdelegates.h"
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


AddUserDialog::AddUserDialog(Session* session, string email): WDialog(), _session(session)
{
  setTitleBarEnabled(true);
  setCaption("Add User");
  resize(410, 480);
  setClosable(true);
  setResizable(true);
  Dbo::QueryModel< AuthorizedUserPtr >* model = new Dbo::QueryModel< AuthorizedUserPtr >();
  model->setQuery(session->find<AuthorizedUser>());
  model->addColumn("email");
  model->addColumn("role");
  WTableView *table = new WTableView();
  table->setColumn1Fixed(false);
  table->setColumnWidth(0, 270);
  table->setColumnWidth(1, 100);
  table->setItemDelegateForColumn(1, new RoleItemDelegate(model));
  table->setModel(model);
  table->setHeight(440);
  WLineEdit *newUser = new WLineEdit(email);
  newUser->setStyleClass("input-medium");
  
  WRegExpValidator* validator = new WRegExpValidator("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}");
  validator->setMandatory(true);
  
  WPushButton *add = new WPushButton("Add");
  add->setStyleClass("btn");
  add->setEnabled(false);
  newUser->setValidator(validator);
  
  auto submitUser = [=](WMouseEvent) {
    Dbo::Transaction t(*session);
    session->add<AuthorizedUser>(new AuthorizedUser(newUser->text().toUTF8()));
    t.commit();
    newUser->setText("");
    add->disable();
    model->reload();
  };
  
  auto validate = [=](){
    WValidator::State s = newUser->validate();
    add->setEnabled(s == WValidator::Valid);
  };
  
  newUser->keyWentUp().connect([=](WKeyEvent k){
    if(k.key() == Wt::Key_Enter) {
      submitUser(WMouseEvent());
    }
    validate();
  });
  add->clicked().connect(submitUser);
  
  WTemplate *form = new WTemplate("<div class=\"form-inline\" style=\"text-align: center\"><label>Email</label> ${emailEdit} ${addButton}</div>");
  form->bindWidget("emailEdit", newUser);
  form->bindWidget("addButton", add);
  contents()->addWidget(form);
  contents()->addWidget(table);
  if(!email.empty())
    validate();
}

AddUserDialog::~AddUserDialog()
{

}
