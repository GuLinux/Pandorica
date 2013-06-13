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

namespace Wt {

  class WStandardItem;
  class WStandardItemModel;
}
namespace StreamingPrivate {
  class PandoricaPrivate;
}
class Pandorica : public Wt::WApplication {

    public:
    Pandorica ( const Wt::WEnvironment& environment );
    virtual ~Pandorica();
    void setupGui();
    virtual void refresh();
    void authEvent();
    
    private:
      StreamingPrivate::PandoricaPrivate *const d;
    private:
};

#endif // STREAMINGAPP_H
