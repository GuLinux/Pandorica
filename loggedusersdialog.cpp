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
#include <Wt/Dbo/QueryModel>
#include <Wt/WTableView>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/WTimer>

using namespace Wt;
using namespace std;
using namespace boost;


class LoggedUsersModel : public Dbo::QueryModel<SessionInfoPtr> {
public:
    LoggedUsersModel(WObject* parent = 0) : Dbo::QueryModel<SessionInfoPtr>(parent) {}
    virtual any data(const WModelIndex& index, int role = Wt::DisplayRole) const;
};


any LoggedUsersModel::data(const WModelIndex& index, int role) const
{
    any realData = Dbo::QueryModel< SessionInfoPtr >::data(index, role);
    if(index.column()>=4 && realData.type() == typeid(long)) {
      return WDateTime::fromTime_t(any_cast<long>(realData));
    }
    return realData;
}


LoggedUsersDialog::LoggedUsersDialog(Session* session, bool showAll)
  : WDialog(), session(session)
{
  setTitleBarEnabled(true);
  setCaption("Logged Users");
  resize(900, 480);
  setClosable(true);
  setResizable(true);
  Dbo::QueryModel< SessionInfoPtr >* model = new LoggedUsersModel();
  auto query = session->find<SessionInfo>().orderBy("sessionStarted desc");
  if(!showAll)
    query.where("active <> 0");
  model->setQuery(query);
  model->addColumn("username");
  model->addColumn("email");
  model->addColumn("role");
  model->addColumn("watching");
  model->addColumn("sessionStarted");
  WTableView *table = new WTableView();
  table->setColumn1Fixed(false);
  table->setColumnWidth(0, 120);
  table->setColumnWidth(1, 200);
  table->setColumnWidth(2, 70);
  table->setColumnWidth(3, 300);
  table->setColumnWidth(4, 150);
  if(showAll) {
    model->addColumn("sessionEnded");
    table->setColumnWidth(5, 150);
    setWidth(1050);
  }
  table->setModel(model);
  contents()->addWidget(table);
  
  WTimer *timer = new WTimer(this);
  timer->setInterval(10000);
  timer->timeout().connect([model](WMouseEvent) { model->reload(); });
  timer->start();
}

LoggedUsersDialog::~LoggedUsersDialog()
{

}

