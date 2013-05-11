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
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WSortFilterProxyModel>
#include <Wt-Commons/wt_helpers.h>

#include "Models/models.h"

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
	WAnchor* link = new WAnchor("", columnValue(any_cast<ColumnType>(model->data(index))));
	IdType id = idColumn(model,index);
    link->setStyleClass("link-hand");
    link->clicked().connect([id,this](WMouseEvent){
	  WDialog *dialog = new SessionDetailsDialog(id, session);
	  dialog->show();
	});
	return link;
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
Dbo::Query<LoggedUserEntry> buildQuery(Session *session, bool showAll, string excludeLoginName) {
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
  if(!excludeLoginName.empty())
    query.where("identity <> ?").bind(excludeLoginName);
  query.where("auth_identity.provider = 'loginname'");
  query.orderBy("session_started desc");
  return query;
}

LoggedUsersDialog::LoggedUsersDialog(Session* session, bool showAll)
  : WDialog(), session(session)
{
  setTitleBarEnabled(true);
  setClosable(true);
  setCaption(wtr("users.current.title"));
  setResizable(false);
  Dbo::QueryModel< LoggedUserEntry >* model = new Dbo::QueryModel<LoggedUserEntry>();

  model->setQuery(buildQuery( session, showAll, string{}) );
  model->addColumn("session_id", "");
  model->addColumn("identity", wtr("session.username"));
  model->addColumn("ip", wtr("session.ip.address"));
  model->addColumn("filewatching", wtr("session.file.last"));
  model->addColumn("session_started", wtr("session.started"));
  model->addColumn("session_ended", wtr("session.ended"));
  model->addColumn("user_id", "");
  model->addColumn("email", "");
  WTableView *table = new WTableView();
  WSortFilterProxyModel *filterModel = new WSortFilterProxyModel(this);
  filterModel->setDynamicSortFilter(true);
  filterModel->setSourceModel(model);
  filterModel->setFilterKeyColumn(1);
  filterModel->setFilterFlags(RegExpFlag::MatchCaseInsensitive);
  table->setModel(filterModel);

  table->setRowHeight(28);
  table->setHeaderHeight(28);
  table->setEditTriggers(Wt::WAbstractItemView::NoEditTrigger);
  
  table->setItemDelegateForColumn(0, new DetailsButtonDelegate<string, string>(filterModel, session,
    [](WAbstractItemModel *model, const WModelIndex &index) { return any_cast<string>(model->data(index.row(), 0)); },
    [](string) { return wtr("session.details").toUTF8(); }));
  table->setItemDelegateForColumn(1, new DetailsButtonDelegate<long, string>(filterModel, session,
    [session](WAbstractItemModel *model, const WModelIndex &index) {
      Dbo::Transaction t(*session);
      long userId = any_cast<long>(model->data(index.row(), 6));
      auto authInfo = session->find<AuthInfo>().where("user_id = ?").bind(userId).resultValue();
      return authInfo->user().id();
    },
    [](string s) { return s; }));
  int columns{0};
  
  auto fixColumnSize = [=](int col, int width) {
    table->setColumnWidth(col, width);
  };
  
  table->setSortingEnabled(columns, false);
  fixColumnSize(columns++, 70);
  fixColumnSize(columns++, 110);
  fixColumnSize(columns++, 110);
//   fixColumnSize(columns++, 300);
  fixColumnSize(columns++, 450);
  fixColumnSize(columns, 110);
  table->setItemDelegateForColumn(columns++, new DateTimeDelegate(filterModel));
  if(showAll) {
    table->setColumnHidden(columns, false);
    setWindowTitle(wtr("users.history.title"));
    table->setItemDelegateForColumn(columns, new DateTimeDelegate(filterModel));
    fixColumnSize(columns++, 110);
//     setWidth(1140);
  } else {
    table->setColumnHidden(columns++, true);
  }
  table->setColumnHidden(columns++, true);
  table->setColumnHidden(columns++, true);
  table->setMaximumSize(WLength::Auto, 350);
  if(showAll) {
    WCheckBox *excludeMyUser = new WCheckBox(wtr("session.filter.exclude.myself"));
    excludeMyUser->changed().connect([=](_n1) {
      string myUsername = session->login().user().identity("loginname").toUTF8();
      model->setQuery(buildQuery(session, showAll, excludeMyUser->checkState() == Wt::Checked ? myUsername : ""));
      model->reload();
      table->refresh();
    });
    excludeMyUser->setMargin(10, Side::Left | Side::Right);
    WLineEdit *searchUser = new WLineEdit();
    searchUser->setEmptyText(wtr("session.filter.login"));
    searchUser->addStyleClass("input-xlarge search-query");
    searchUser->keyWentUp().connect([=](WKeyEvent) {
      filterModel->setFilterRegExp(WString(".*{1}.*").arg(searchUser->text()));
      wApp->log("notice") << "new filter regex: " << filterModel->filterRegExp();
    });
    contents()->addWidget(WW<WContainerWidget>().setMargin(10).css("form-inline").add(searchUser).add(excludeMyUser));
    resize(1050, 500);
  }
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

