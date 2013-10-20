/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MEDIAINFOPANELPRIVATE_H
#define MEDIAINFOPANELPRIVATE_H
#include "mediainfopanel.h"

class MediaInfoPanel::Private
{
  public:
    Private( MediaInfoPanel *q, Session *session, Settings *settings );
    virtual ~Private();
  void labelValueBox(std::string label, Wt::WString value, Wt::WTable* container);
  void labelValueBox(std::string label, Wt::WWidget *widget, Wt::WTable* container);
  Wt::Signal<Media> play;
  Wt::Signal<Media> queue;
  Wt::Signal<Media> setTitle;
  Wt::Signal<Media> setPoster;
  Wt::Signal<Media> deletePoster;
  Wt::Signal<Media> deleteAttachments;
  Wt::Signal<> gotInfo;
  Wt::Signal<> wasResetted;
  Wt::Signal<> playFolder;
  Wt::Signal<> playFolderRecursive;
  Session *session;
  Settings* settings;
  bool isAdmin;
  std::pair<Wt::WPanel*,Wt::WContainerWidget*> createPanel(std::string titleKey);
  
  private:
    class MediaInfoPanel *const q;
};

#endif // MEDIAINFOPANELPRIVATE_H
