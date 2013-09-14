/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/




#ifndef FINDORPHANSDIALOGPRIVATE_H
#define FINDORPHANSDIALOGPRIVATE_H
#include <stdint.h>
#include <Wt/WModelIndex>

class Settings;
namespace Wt {
class WStandardItemModel;
class WText;
class WPushButton;
class WStackedWidget;
class WContainerWidget;
namespace Dbo {
  class Transaction;
}
class WProgressBar;
}

class Session;
class MediaCollection;
typedef std::function<void(Wt::Dbo::Transaction &)> MigrateF;

namespace PandoricaPrivate {
  struct DataSummary {
    uint mediasCount = 0;
    uint attachmentsCount = 0;
    uint64_t bytes = 0;
  };
  
  struct FileSuggestion {
    std::string filePath;
    std::string mediaId;
    uint64_t score = 0;
    FileSuggestion(std::string filePath, std::string mediaId, std::vector< std::string> originalFileTokens);
    FileSuggestion(std::string filePath, std::string mediaId, uint64_t score)
    : filePath(filePath), mediaId(mediaId), score(score) {}
  };
  
  
  class FindOrphansDialogPrivate
  {
  public:
    enum ModelExtraData { MediaId = Wt::UserRole, Score = Wt::UserRole+1, Path = Wt::UserRole + 2 };
    FindOrphansDialogPrivate(FindOrphansDialog* q);
    virtual ~FindOrphansDialogPrivate();
    MediaCollection *mediaCollection;
    Session* session;
    Session* threadsSession;
    Wt::WText* summary;
    Wt::WPushButton* nextButton;
    Wt::WPushButton* closeButton;
    Wt::WPushButton* saveButton;
    Wt::WStandardItemModel* model;
    Wt::WStackedWidget *stack;
    Wt::WContainerWidget *movedOrphansContainer;
    Wt::WProgressBar *migrationProgress;
    Settings* settings;
    
    void fixFilePaths();

    void nextButtonClicked();
    void migrate(Wt::Dbo::Transaction &transaction, std::string oldMediaId, std::string newMediaId);
    void applyMigrations(Wt::WApplication *app);
    void populateRemoveOrphansModel(Wt::WApplication *app);
    void populateMovedFiles(Wt::WApplication *app);
    std::vector<std::string> orphans(Wt::Dbo::Transaction &transaction);
    std::vector<MigrateF> migrations;
  private:
      class FindOrphansDialog* const q;
      DataSummary dataSummary;
  };
}
#endif // FINDORPHANSDIALOGPRIVATE_H
