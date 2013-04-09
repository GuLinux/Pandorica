/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2013  Marco Gulino <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef GROUPSDIALOG_H
#define GROUPSDIALOG_H

#include <Wt/WDialog>

namespace Wt {
class WTable;
}

class Session;

class GroupsDialog : public Wt::WDialog
{
public:
    GroupsDialog(Session *session);
private:
  void populateGroups();
  Session *session;
  Wt::WTable *currentGroups;
};

#endif // GROUPSDIALOG_H
