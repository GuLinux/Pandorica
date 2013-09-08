#ifndef MIGRATIONS_H
#define MIGRATIONS_H
#include "Wt-Commons/migratedbo.h"
#include "migrations/create_migrations_table.h"
#include "src/migrations/comments.h"
#include "src/migrations/sessiontracking.h"
// migrations_includes_placeholder
namespace Migrations
{
  WtCommons::Migrations migrations
  {
    std::shared_ptr<WtCommons::DboMigration>(createMigrationTable()),
    std::shared_ptr<WtCommons::DboMigration>(Comments::convertCommentCreationToDateTime()),
    std::shared_ptr<WtCommons::DboMigration>(SessionTracking::changeSessionTimeColumnsToDBTimestamps()),
// migrations_init_placeholder
  };
}
#endif
