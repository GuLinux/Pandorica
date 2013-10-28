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


#include "mediascanner.h"
#include "private/mediascanner_p.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils/d_ptr_implementation.h"
#include <session.h>
#include <settings.h>
#include <media/mediacollection.h>

using namespace Wt;
using namespace WtCommons;
using namespace std;

MediaScanner::Private::Private(MediaScanner* q, Session* session, Settings* settings, MediaCollection* mediaCollection, std::function<bool(Media&)> scanFilter)
  : session(session), settings(settings), mediaCollection(mediaCollection), scanFilter(scanFilter), q(q)
{
}
MediaScanner::Private::~Private()
{
}

MediaScanner::~MediaScanner()
{
}

MediaScanner::MediaScanner(Session* session, Settings* settings, MediaCollection* mediaCollection, function<bool(Media&)> scanFilter, WContainerWidget* parent)
    : d(this, session, settings, mediaCollection, scanFilter)
{
}

void MediaScanner::scan()
{
}
