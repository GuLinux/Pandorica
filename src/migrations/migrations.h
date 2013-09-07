#ifndef MIGRATIONS_H
#define MIGRATIONS_H
#include "Wt-Commons/migratedbo.h"
#include "migrations/create_migrations_table.h"
namespace Migrations
{
  WtCommons::Migrations migrations
  {
    std::shared_ptr<WtCommons::DboMigration>(createMigrationTable()),
    std::shared_ptr<WtCommons::DboMigration>(addNameToMigrationTable()),
    std::shared_ptr<WtCommons::DboMigration>(renameWhenColumnToWhenApplied()),
    std::shared_ptr<WtCommons::DboMigration>(addMigrationCreationColumn()),
  };
}
#endif
