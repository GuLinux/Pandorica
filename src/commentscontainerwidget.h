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


#ifndef COMMENTSCONTAINERWIDGET_H
#define COMMENTSCONTAINERWIDGET_H

#include <Wt/WContainerWidget>
#include "utils/d_ptr.h"

class Session;

class CommentsContainerWidget : public Wt::WContainerWidget
{

public:
    CommentsContainerWidget(std::string videoId, Session *session, Wt::WContainerWidget* parent = 0);
    virtual ~CommentsContainerWidget();
private:
  D_PTR;
};

#endif // COMMENTSCONTAINERWIDGET_H
