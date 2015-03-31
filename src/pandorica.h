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




#ifndef STREAMINGAPP_H
#define STREAMINGAPP_H

#include <Wt/WApplication>
#include <boost/filesystem.hpp>
#include "utils/d_ptr.h"

namespace Wt
{

  class WStandardItem;
  class WStandardItemModel;
}
class Pandorica : public Wt::WApplication
{

  public:
    Pandorica( const Wt::WEnvironment &environment );
    virtual ~Pandorica();
    void setupGui();
    virtual void refresh();
    void authEvent();
    enum NotificationType { Alert, Error, Success, Information };
    void notify(const Wt::WString &text, NotificationType notificationType, int autocloseAfterSeconds = 0);
    static Pandorica *instance();
  protected:
    virtual void notify( const Wt::WEvent &e );
  private:
    D_PTR;
};

#define pApp dynamic_cast<Pandorica*>(wApp)

#endif // STREAMINGAPP_H
