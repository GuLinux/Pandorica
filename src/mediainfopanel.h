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

#ifndef MEDIAINFOPANEL_H
#define MEDIAINFOPANEL_H
#include "utils/d_ptr.h"
#include <Wt/WContainerWidget>

class Media;
class Session;
class Settings;
class MediaInfoPanel : public Wt::WContainerWidget
{
public:
    MediaInfoPanel(Session* session, Settings* settings, Wt::WContainerWidget* parent = 0);
    ~MediaInfoPanel();
    void info(Media &media);
    void reset();
    inline Wt::Signal<Media> &play() { return _play; }
    inline Wt::Signal<Media> &queue() { return _queue; }
    inline Wt::Signal<Media> &setTitle() { return _setTitle; }
    inline Wt::Signal<Media> &setPoster() { return _setPoster; }
    inline Wt::Signal<Media> &deletePoster() { return _deletePoster; }
    inline Wt::Signal<Media> &deleteAttachments() { return _deleteAttachments; }
    inline Wt::Signal<> &gotInfo() { return _gotInfo; }
    inline Wt::Signal<> &wasResetted() { return _wasResetted; }
    inline Wt::Signal<> &playFolder() { return _playFolder; }
    inline Wt::Signal<> &playFolderRecursive() { return _playFolderRecursive; }
private:
  void labelValueBox(std::string label, Wt::WString value, Wt::WTable* container);
  void labelValueBox(std::string label, Wt::WWidget *widget, Wt::WTable* container);
  Wt::Signal<Media> _play;
  Wt::Signal<Media> _queue;
  Wt::Signal<Media> _setTitle;
  Wt::Signal<Media> _setPoster;
  Wt::Signal<Media> _deletePoster;
  Wt::Signal<Media> _deleteAttachments;
  Wt::Signal<> _gotInfo;
  Wt::Signal<> _wasResetted;
  Wt::Signal<> _playFolder;
  Wt::Signal<> _playFolderRecursive;
  Session *session;
  Settings* settings;
  bool isAdmin;
  std::pair<Wt::WPanel*,Wt::WContainerWidget*> createPanel(std::string titleKey);
  
  private:
    D_PTR;
};

#endif // MEDIAINFOPANEL_H

class Media;
