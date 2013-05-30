#include "sessiondetailsdialog.h"
#include "session.h"
#include "customitemdelegates.h"
#include <Wt/Dbo/QueryModel>
#include <Wt/WTableView>
#include <Wt/WApplication>
#include <Wt/WTimer>
#include <Wt/WPushButton>
#include <Wt-Commons/wt_helpers.h>

#include "Models/models.h"
#include "settings.h"

using namespace std;
using namespace Wt;
using namespace boost;

SessionDetailsDialog::SessionDetailsDialog(string id, Session* session, Settings* settings)
  : SessionDetailsDialog(session->query<SessionDetailsTuple>("select \
    filename,\
    play_started,\
    play_ended,\
    filepath from session_details").where("session_info_session_id = ?").bind(id).orderBy("play_started desc"), settings)
{
}

SessionDetailsDialog::SessionDetailsDialog(long int userId, Session* session, Settings* settings)
  : SessionDetailsDialog(session->query<SessionDetailsTuple>("select session_details.filename as filename,\
							      session_details.play_started as play_started,\
							      session_details.play_ended as play_ended,\
							      session_details.filepath as filepath \
	from session_details\
	inner join session_info on session_id = session_info_session_id").where("user_id = ?").bind(userId).orderBy("play_started desc"), settings)
{
  wApp->log("notice") << "finding by user_id = " << userId;
}

SessionDetailsDialog::SessionDetailsDialog(const Dbo::Query< SessionDetailsTuple >& query, Settings* settings): WDialog()
{
  Dbo::QueryModel< SessionDetailsTuple >* model = new Dbo::QueryModel<SessionDetailsTuple>();
  setTitleBarEnabled(true);
  setCaption("Session Details");
//   resize(1020, 480);
  setResizable(false);
  setClosable(true);
  model->setQuery(query);
  model->addColumn("filename", wtr("session.details.filename"));
  model->addColumn("play_started", wtr("session.details.started"));
  model->addColumn("play_ended", wtr("session.details.ended"));
  model->addColumn("filepath", wtr("session.details.fullpath"));
  WTableView *table = new WTableView();
  table->setItemDelegateForColumn(1, new DateTimeDelegate(model));
  table->setItemDelegateForColumn(2, new DateTimeDelegate(model));
  table->setItemDelegateForColumn(3, new StringTransformDelegate([=](string path) { return settings->relativePath(path); }, model, table ));
  table->setColumnWidth(0, 300);
  table->setColumnWidth(1, 110);
  table->setColumnWidth(2, 110);
  table->setColumnWidth(3, 440);
  table->setModel(model);
  table->setMaximumSize(WLength::Auto, 350);
  table->setRowHeight(28);
  table->setHeaderHeight(28);
  contents()->addWidget(table);
  
  WTimer *timer = new WTimer(this);
  timer->setInterval(10000);
  timer->timeout().connect([model](WMouseEvent) {
    model->reload();
  });
  timer->start();
}
