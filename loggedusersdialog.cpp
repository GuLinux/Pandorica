/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2013  Marco Gulino <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "loggedusersdialog.h"
#include "session.h"
#include "customitemdelegates.h"
#include "sessiondetailsdialog.h"
#include <Wt/Dbo/QueryModel>
#include <Wt/WTableView>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/WTimer>
#include <Wt/WItemDelegate>
#include <Wt/WAnchor>

using namespace Wt;
using namespace std;
using namespace boost;


class DetailsButtonDelegate : public WItemDelegate {
public:
    DetailsButtonDelegate(WAbstractItemModel *model, Session *session, WObject* parent = 0)
      : WItemDelegate(parent), model(model), session(session) {}
    virtual WWidget* update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags);
private:
  WAbstractItemModel *model;
  Session *session;
};

WWidget* DetailsButtonDelegate::update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags)
{
  if(!widget) {
    WPushButton* button = new WPushButton("Details");
    string id = any_cast<string>(model->data(index.row(), 0));
    button->setStyleClass("btn btn-link");
    button->clicked().connect([id,this](WMouseEvent){
      WDialog *dialog = new SessionDetailsDialog(id, session);
      dialog->show();
    });
    return button;
  }
  return widget;
}

typedef boost::tuple<string,string,long,long,string,string,string,int> LoggedUserEntry;
LoggedUsersDialog::LoggedUsersDialog(Session* session, bool showAll)
  : WDialog(), session(session)
{
  setTitleBarEnabled(true);
  setCaption("Logged Users");
  resize(1020, 510);
  setClosable(true);
  setResizable(true);
  Dbo::QueryModel< LoggedUserEntry >* model = new Dbo::QueryModel<LoggedUserEntry>();
  auto query = session->query<LoggedUserEntry>("select session_id,ip,session_started,session_ended,\
    session_details.filename as filewatching,\
    auth_info.email as email,\
    auth_identity.identity as identity,\
    authorized_users.role as role\
    from session_info left join session_details\
    on session_info.session_id = session_details.session_info_session_id\
    inner join auth_info on session_info.user_id = auth_info.user_id\
    inner join auth_identity on auth_info.id = auth_identity.auth_info_id\
    inner join authorized_users on authorized_users.email = auth_info.email");
  if(!showAll)
    query.where("session_ended = 0");
  query.where("session_details.play_ended = 0 OR session_details.play_ended is null");
  query.where("auth_identity.provider = 'loginname'");
  query.orderBy("session_started desc");
  model->setQuery(query);
  model->addColumn("session_id", "");
  model->addColumn("identity", "UserName");
  model->addColumn("ip", "IP");
  model->addColumn("email", "Email");
  model->addColumn("role", "Role");
  model->addColumn("filewatching", "Last File Played");
  model->addColumn("session_started", "Started");
  WTableView *table = new WTableView();
  table->setItemDelegateForColumn(0, new DetailsButtonDelegate(model, session));
  table->setItemDelegateForColumn(4, new RoleItemDelegate(model));
  table->setColumn1Fixed(false);
  table->setColumnWidth(0, 50);
  table->setColumnWidth(1, 120);
  table->setColumnWidth(2, 90);
  table->setColumnWidth(3, 200);
  table->setColumnWidth(4, 70);
  table->setColumnWidth(5, 300);
  table->setColumnWidth(6, 110);
  table->setHeight(470);
  table->setItemDelegateForColumn(6, new DateTimeDelegate(model));
  if(showAll) {
    model->addColumn("session_ended", "Ended");
    table->setItemDelegateForColumn(7, new DateTimeDelegate(model));
    table->setColumnWidth(7, 110);
    setWidth(1140);
  }
  table->setModel(model);
  contents()->addWidget(table);
  
  WTimer *timer = new WTimer(this);
  timer->setInterval(10000);
  timer->timeout().connect([model](WMouseEvent) {
    model->reload();
  });
  timer->start();
}

LoggedUsersDialog::~LoggedUsersDialog()
{

}

