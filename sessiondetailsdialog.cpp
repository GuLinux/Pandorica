#include "sessiondetailsdialog.h"
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"
#include "customitemdelegates.h"
#include <Wt/Dbo/QueryModel>
#include <Wt/WTableView>
#include <Wt/WApplication>
#include <Wt/WTimer>

using namespace std;
using namespace Wt;
using namespace boost;

SessionDetailsDialog::SessionDetailsDialog(string id, Session* session)
  : SessionDetailsDialog(session->query<SessionDetailsTuple>("select * from session_details").where("session_info_session_id = ?").bind(id).orderBy("play_started desc"))
{
}

SessionDetailsDialog::SessionDetailsDialog(long userId, Session* session)
  : SessionDetailsDialog(session->query<SessionDetailsTuple>("select session_details.filename as filename,\
							      session_details.play_started as play_started,\
							      session_details.play_ended as play_ended,\
							      session_details.filepath as filepath \
	from session_details\
	inner join session_info on session_id = session_info_session_id").where("user_id = ?").bind(userId).orderBy("play_started desc"))
{
  wApp->log("notice") << "finding by user_id = " << userId;
}

SessionDetailsDialog::SessionDetailsDialog(const Dbo::Query< SessionDetailsTuple>& query): WDialog()
{
  Dbo::QueryModel< SessionDetailsTuple >* model = new Dbo::QueryModel<SessionDetailsTuple>();
  setTitleBarEnabled(true);
  setCaption("Session Details");
  resize(1000, 480);
  setClosable(true);
  setResizable(true);
  model->setQuery(query);
  model->addColumn("filename", "File");
  model->addColumn("play_started", "Started");
  model->addColumn("play_ended", "Ended");
  model->addColumn("filepath", "Full File Path");
  WTableView *table = new WTableView();
  table->setItemDelegateForColumn(1, new DateTimeDelegate(model));
  table->setItemDelegateForColumn(2, new DateTimeDelegate(model));
  table->setColumn1Fixed(false);
  table->setColumnWidth(0, 300);
  table->setColumnWidth(1, 110);
  table->setColumnWidth(2, 110);
  table->setColumnWidth(3, 440);
  table->setModel(model);
  table->setHeight(470);
  contents()->addWidget(table);
  
  WTimer *timer = new WTimer(this);
  timer->setInterval(10000);
  timer->timeout().connect([model](WMouseEvent) {
    model->reload();
  });
  timer->start();
}
