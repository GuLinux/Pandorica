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
#include "comment.h"
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
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt-Commons/wt_helpers.h>

using namespace Wt;
using namespace std;
using namespace boost;

template<class IdType,class ColumnType>
class DetailsButtonDelegate : public WItemDelegate {
  typedef std::function<string(ColumnType)> ColumnValue;
  typedef std::function<IdType(WAbstractItemModel *model, const WModelIndex& index)> GetId;
public:
    DetailsButtonDelegate(WAbstractItemModel *model, Session *session, GetId idColumn, ColumnValue columnValue, WObject* parent = 0)
      : WItemDelegate(parent), model(model), session(session), idColumn(idColumn), columnValue(columnValue) {}
    virtual WWidget* update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags) {
      if(!widget) {
	WPushButton* button = new WPushButton(columnValue(any_cast<ColumnType>(model->data(index))));
	IdType id = idColumn(model,index);
        button->setStyleClass("btn btn-link btn-small");
	button->clicked().connect([id,this](WMouseEvent){
	  WDialog *dialog = new SessionDetailsDialog(id, session);
	  dialog->show();
	});
	return button;
      }
      return widget;
    }
private:
  WAbstractItemModel *model;
  Session *session;
  GetId idColumn;
  ColumnValue columnValue;
};

typedef boost::tuple<string,string,long,long,string,string,string,long> LoggedUserEntry;
LoggedUsersDialog::LoggedUsersDialog(Session* session, bool showAll)
  : WDialog(), session(session)
{
  setTitleBarEnabled(true);
  setCaption(wtr("users.current.title"));
  setResizable(true);
  Dbo::QueryModel< LoggedUserEntry >* model = new Dbo::QueryModel<LoggedUserEntry>();
  auto query = session->query<LoggedUserEntry>("select distinct session_id,ip,session_started,session_ended,\
    (select filename from session_details WHERE session_info.session_id = session_details.session_info_session_id ORDER BY play_started DESC LIMIT 1) as filewatching,\
    auth_info.email as email,\
    auth_identity.identity as identity,\
    session_info.user_id as user_id\
    from session_info\
    inner join auth_info on session_info.user_id = auth_info.user_id\
    inner join auth_identity on auth_info.id = auth_identity.auth_info_id\
    ");
  if(!showAll)
    query.where("session_ended = 0");
  query.where("auth_identity.provider = 'loginname'");
  query.orderBy("session_started desc");
  model->setQuery(query);
  model->addColumn("session_id", "");
  model->addColumn("identity", "UserName");
  model->addColumn("ip", "IP");
  model->addColumn("filewatching", "Last File Played");
  model->addColumn("session_started", "Started");
  model->addColumn("user_id", "User Id");
  model->addColumn("email", "Email");
  WTableView *table = new WTableView();
  table->setModel(model);

  table->setAlternatingRowColors(true);
  table->setRowHeight(28);
//   table->setHeaderHeight(28);
  table->setEditTriggers(Wt::WAbstractItemView::NoEditTrigger);
  
  table->setItemDelegateForColumn(0, new DetailsButtonDelegate<string, string>(model, session,
    [](WAbstractItemModel *model, const WModelIndex &index) { return any_cast<string>(model->data(index.row(), 0)); },
    [](string) { return "Details"; }));
  table->setItemDelegateForColumn(1, new DetailsButtonDelegate<long, string>(model, session,
    [session](WAbstractItemModel *model, const WModelIndex &index) {
      Dbo::Transaction t(*session);
      long userId = any_cast<long>(model->data(index.row(), 5));
      auto authInfo = session->find<AuthInfo>().where("user_id = ?").bind(userId).resultValue();
      return authInfo->user().id();
    },
    [](string s) { return s; }));
  int columns{0};
  table->setColumnWidth(columns++, 50);
  table->setColumnWidth(columns++, 60);
  table->setColumnWidth(columns++, 60);
//   table->setColumnWidth(columns++, 300);
  table->setColumnWidth(columns++, 300);
  table->setColumnWidth(columns, 110);
  table->setItemDelegateForColumn(columns++, new DateTimeDelegate(model));
  if(showAll) {
    setWindowTitle(wtr("users.history.title"));
    model->addColumn("session_ended", "Ended");
    table->setItemDelegateForColumn(columns, new DateTimeDelegate(model));
    table->setColumnWidth(columns++, 110);
//     setWidth(1140);
  }
  table->setColumnHidden(columns++, true);
  table->setColumnHidden(columns++, true);
  table->setMaximumSize(WLength::Auto, 350);
  contents()->addWidget(table);
  
  WTimer *timer = new WTimer(this);
  timer->setInterval(10000);
  timer->timeout().connect([model](WMouseEvent) {
    model->reload();
  });
  timer->start();
  footer()->addWidget(WW<WPushButton>(wtr("close-button")).css("btn btn-primary").onClick([=](WMouseEvent){ accept(); }));
}

LoggedUsersDialog::~LoggedUsersDialog()
{

}

