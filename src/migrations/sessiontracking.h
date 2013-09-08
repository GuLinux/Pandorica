#ifndef MIGRATIONS_CHANGESESSIONTIMECOLUMNSTODBTIMESTAMPS_TABLE_H
#define MIGRATIONS_CHANGESESSIONTIMECOLUMNSTODBTIMESTAMPS_TABLE_H
#include "Wt-Commons/migratedbo.h"

namespace Migrations
{
  namespace SessionTracking
  {
    void changeColumnFromIntToTimestamp(const std::string &tableName, const std::string &columnName, WtCommons::DboMigration &m)
    {
        std::string tempColumnName = columnName + "_temp";
        m.renameColumn( tableName, columnName, tempColumnName );
        m.addColumn<boost::posix_time::ptime>( tableName, columnName );
        m.execute( "update \"%s\" set \"%s\" = datetime( \"%s\", \"unixepoch\")", {tableName, columnName, tempColumnName}, WtCommons::DboMigration::Sqlite3 );
        m.execute( "update \"%s\" set \"%s\" = TIMESTAMP WITH TIME ZONE 'epoch' + \"%s\" * INTERVAL '1 second';", {tableName, columnName, tempColumnName}, WtCommons::DboMigration::PostgreSQL );
        m.removeColumn( tableName, tempColumnName );
    }
    CREATE_MIGRATION( changeSessionTimeColumnsToDBTimestamps, "2013-09-08 13:22:57",
      {
        changeColumnFromIntToTimestamp("session_info", "session_started", m);
        changeColumnFromIntToTimestamp("session_info", "session_ended", m);
        changeColumnFromIntToTimestamp("session_details", "play_started", m);
        changeColumnFromIntToTimestamp("session_details", "play_ended", m);
      }
    )

    // migrations_placeholder
  }
}
#endif

