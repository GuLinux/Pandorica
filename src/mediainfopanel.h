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
    void info(const Media& media);
    void reset();
    Wt::Signal<Media> &play() const;
    Wt::Signal<Media> &queue() const;
    Wt::Signal<> &gotInfo() const;
    Wt::Signal<> &wasResetted() const;
    Wt::Signal<> &playFolder() const;
    Wt::Signal<> &playFolderRecursive() const;
  private:
    D_PTR;
};

#endif // MEDIAINFOPANEL_H

class Media;

