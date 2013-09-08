#ifndef MIGRATIONS_CONVERTCOMMENTCREATIONTODATETIME_TABLE_H
#define MIGRATIONS_CONVERTCOMMENTCREATIONTODATETIME_TABLE_H
#include "Wt-Commons/migratedbo.h"

namespace Migrations
{
  namespace Comments
  {
    CREATE_MIGRATION( convertCommentCreationToDateTime, "2013-09-08 11:33:21",
      {
        m.renameColumn("comment", "last_updated", "temp_lastupdated");
        m.addColumn<boost::posix_time::ptime>("comment", "last_updated");
        m.execute("update \"comment\" set \"last_updated\" = datetime( \"temp_lastupdated\", \"unixepoch\")", {}, WtCommons::DboMigration::Sqlite3);
        m.execute("update \"comment\" set \"last_updated\" = TIMESTAMP WITH TIME ZONE 'epoch' + \"temp_lastupdated\" * INTERVAL '1 second';", {}, WtCommons::DboMigration::PostgreSQL);
        m.removeColumn("comment", "temp_lastupdated");
      }
    )

    // migrations_placeholder
  }
}
#endif

