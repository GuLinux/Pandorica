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



#ifndef SELECTDIRECTORIESPRIVATE_H
#define SELECTDIRECTORIESPRIVATE_H

#include <vector>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include <Wt/WAbstractItemView>
#include "selectdirectories.h"

namespace boost
{
  namespace filesystem
  {
    class path;
  }
}

namespace Wt
{
  class WTreeView;
  class WStandardItem;
  class WStandardItemModel;
}

class SelectDirectories::Private
{
  public:
    Private( SelectDirectories *q, std::vector<std::string> selectedPaths, SelectDirectories::SelectionType );
    Wt::WTreeView *tree;
    Wt::WStandardItemModel *model;
    void populateTree( std::string path );
    Wt::WApplication *app;
    void addSubItems( Wt::WStandardItem *item, bool sync = false );
    void trySelecting( Wt::WStandardItem *item, boost::filesystem::path path );
    SelectDirectories::SelectionType selectionType;
    std::map<boost::filesystem::path, Wt::WStandardItem *> items;
    void scrollTo(const boost::filesystem::path &p, Wt::WAbstractItemView::ScrollHint scrollHint = Wt::WAbstractItemView::PositionAtTop);
    static const std::string item_css;
  private:
    Wt::WStandardItem *buildStandardItem( boost::filesystem::path path, bool addSubItems );
    class SelectDirectories *const q;
    std::vector<std::string> selectedPaths;
};




#endif // SELECTDIRECTORIESPRIVATE_H
