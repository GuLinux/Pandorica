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
#include "sessioninfo.h"
#include "sessiondetails.h"
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

LoggedUsersDialog::LoggedUsersDialog(Session* session, bool showAll)
  : WDialog(), session(session)
{
  setTitleBarEnabled(true);
  setCaption("Logged Users");
  resize(1000, 480);
  setClosable(true);
  setResizable(true);
  Dbo::QueryModel< SessionInfoPtr >* model = new Dbo::QueryModel<SessionInfoPtr>();
  auto query = session->find<SessionInfo>().orderBy("session_started desc");
  if(!showAll)
    query.where("session_ended <> 0");
  model->setQuery(query);
  model->addColumn("session_id", "");
  model->addColumn("username", "UserName");
  model->addColumn("ip", "IP");
  model->addColumn("email", "Email");
  model->addColumn("role", "Role");
  model->addColumn("watching", "Last File Played");
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
    setWidth(1120);
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

