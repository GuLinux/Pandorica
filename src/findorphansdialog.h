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




#ifndef FINDORPHANSDIALOG_H
#define FINDORPHANSDIALOG_H
#include <Wt/WDialog>
#include "utils/d_ptr.h"

class Settings;
class Session;
class MediaCollection;
class FindOrphansDialog : public Wt::WDialog
{
public:
    FindOrphansDialog(MediaCollection *mediaCollection, Session *session, Settings *settings, Wt::WObject* parent = 0);
    ~FindOrphansDialog();

    void run();
private:
  D_PTR;
};

#endif // FINDORPHANSDIALOG_H
