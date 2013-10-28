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


#ifndef MEDIASCANNER_H
#define MEDIASCANNER_H

#include <Wt/WContainerWidget>
#include "utils/d_ptr.h"
#include <functional>

class Media;
class MediaCollection;
class Settings;
class Session;
class MediaScanner :  Wt::WContainerWidget
{
public:
    ~MediaScanner();
    MediaScanner(Session* session, Settings* settings, MediaCollection* mediaCollection, std::function<bool(Media&)> scanFilter = [](Media&){ return true; }, Wt::WContainerWidget* parent = 0);
    void scan();
private:
    D_PTR;
};

#endif // MEDIASCANNER_H
