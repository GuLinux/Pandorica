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


#ifndef SESSIONDETAILSDIALOG_H
#define SESSIONDETAILSDIALOG_H

#include <Wt/WDialog>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/Query>
#include <boost/tuple/tuple.hpp>

class Settings;
class Session;
class SessionDetails;

typedef boost::tuple<std::string,long,long,std::string> SessionDetailsTuple;
class SessionDetailsDialog : public Wt::WDialog
{

public:
    SessionDetailsDialog(std::string id, Session *session, Settings *settings);
    SessionDetailsDialog(long userId, Session *session, Settings *settings);
    
private:
    SessionDetailsDialog(const Wt::Dbo::Query<SessionDetailsTuple> &query, Settings *settings);
};

#endif // SESSIONDETAILSDIALOG_H
