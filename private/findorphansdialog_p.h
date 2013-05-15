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

namespace Wt {
class WStandardItemModel;
class WText;
class WPushButton;
}

class Session;
class MediaCollection;

namespace StreamingPrivate {
  struct DataSummary {
    uint mediasCount = 0;
    uint attachmentsCount = 0;
    uint64_t bytes = 0;
  };
  
  class FindOrphansDialogPrivate
  {
  public:
    FindOrphansDialogPrivate(FindOrphansDialog* q);
    virtual ~FindOrphansDialogPrivate();
    MediaCollection *mediaCollection;
    Session* session;
    Wt::WText* summary;
    Wt::WPushButton* closeButton;
    Wt::WPushButton* saveButton;
    Wt::WStandardItemModel* model;

      void populateModel(Wt::WStandardItemModel *model, Wt::WApplication *app);
  private:
      class FindOrphansDialog* const q;
      DataSummary dataSummary;
  };
}
#endif // FINDORPHANSDIALOGPRIVATE_H
