/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

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
}

class Session;
class MediaCollection;
typedef std::function<void(Wt::Dbo::Transaction &)> MigrateF;

namespace StreamingPrivate {
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
    Wt::WText* summary;
    Wt::WPushButton* nextButton;
    Wt::WPushButton* closeButton;
    Wt::WPushButton* saveButton;
    Wt::WStandardItemModel* model;
    Wt::WStackedWidget *stack;
    Wt::WContainerWidget *movedOrphansContainer;
    Settings* settings;

    void nextButtonClicked();
    
    void populateRemoveOrphansModel(Wt::WStandardItemModel *model, Wt::WApplication *app);
    void populateMovedFiles(Wt::WApplication *app);
    std::vector<std::string> orphans(Wt::Dbo::Transaction &transaction);
    std::vector<MigrateF> migrations;
  private:
      class FindOrphansDialog* const q;
      DataSummary dataSummary;
  };
}
#endif // FINDORPHANSDIALOGPRIVATE_H
