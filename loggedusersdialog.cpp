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
#include <Wt/WItemDelegate>
#include <Wt/WAnchor>

using namespace Wt;
using namespace std;
using namespace boost;


class DateTimeDelegate : public WItemDelegate {
public:
    DateTimeDelegate(WAbstractItemModel *model, WObject* parent = 0) : WItemDelegate(parent), model(model) {}
    virtual WWidget* update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags);
private:
  WAbstractItemModel *model;
};


class DetailsButtonDelegate : public WItemDelegate {
public:
    DetailsButtonDelegate(WAbstractItemModel *model, WObject* parent = 0) : WItemDelegate(parent), model(model) {}
    virtual WWidget* update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags);
private:
  WAbstractItemModel *model;
};

WWidget* DetailsButtonDelegate::update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags)
{
  if(!widget) {
    WPushButton* button = new WPushButton("Details");
    button->setStyleClass("btn btn-link");
    return button;
  }
  return widget;
}


WWidget* DateTimeDelegate::update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags)
{
  long timeT = any_cast<long>(model->data(index));
  string label = timeT ? WDateTime::fromTime_t(timeT).toString("dd/M/yyyy HH:mm").toUTF8() : "Active Session";
  if(!widget) {
    WText* labelWidget = new WText(label);
    labelWidget->setStyleClass("small-text");
    return labelWidget;
  }
  ((WText*) widget)->setText(label);
  return widget;
}





LoggedUsersDialog::LoggedUsersDialog(Session* session, bool showAll)
  : WDialog(), session(session)
{
  setTitleBarEnabled(true);
  setCaption("Logged Users");
  resize(910, 480);
  setClosable(true);
  setResizable(true);
  Dbo::QueryModel< SessionInfoPtr >* model = new Dbo::QueryModel<SessionInfoPtr>();
  auto query = session->find<SessionInfo>().orderBy("sessionStarted desc");
  if(!showAll)
    query.where("active <> 0");
  model->setQuery(query);
  model->addColumn("sessionId", "");
  model->addColumn("username", "UserName");
  model->addColumn("email", "Email");
  model->addColumn("role", "Role");
  model->addColumn("watching", "Last File Played");
  model->addColumn("sessionStarted", "Started");
  WTableView *table = new WTableView();
  table->setItemDelegateForColumn(0, new DetailsButtonDelegate(model));
  table->setColumn1Fixed(false);
  table->setColumnWidth(0, 50);
  table->setColumnWidth(1, 120);
  table->setColumnWidth(2, 200);
  table->setColumnWidth(3, 70);
  table->setColumnWidth(4, 300);
  table->setColumnWidth(5, 110);
  table->setItemDelegateForColumn(5, new DateTimeDelegate(model));
  if(showAll) {
    model->addColumn("sessionEnded", "Ended");
    table->setItemDelegateForColumn(6, new DateTimeDelegate(model));
    table->setColumnWidth(6, 110);
    setWidth(1020);
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

