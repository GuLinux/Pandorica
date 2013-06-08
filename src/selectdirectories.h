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




#ifndef SELECTDIRECTORIES_H
#define SELECTDIRECTORIES_H

#include <vector>
#include <string>
#include <functional>
#include <Wt/WObject>
#include <Wt/WLength>

namespace Wt {
class WContainerWidget;
}

namespace StreamingPrivate {
  class SelectDirectoriesPrivate;
}

typedef std::function<void(std::string)> OnPathClicked;

class SelectDirectories : public Wt::WObject
{
public:
  enum SelectionType { Single, Multiple };
  SelectDirectories(std::vector<std::string> rootPaths, std::string selectedPath, OnPathClicked onPathSelected, Wt::WObject *parent = 0);
  SelectDirectories(std::vector<std::string> rootPaths, std::vector<std::string> selectedPaths, OnPathClicked onPathSelected, OnPathClicked onPathUnselected, SelectionType selectionType, Wt::WObject *parent = 0);
  virtual ~SelectDirectories();
  void addTo(Wt::WContainerWidget *container);
  void setWidth(Wt::WLength width);
  void setHeight(Wt::WLength height);
  inline void resize(Wt::WLength &width, Wt::WLength &height) { setWidth(width); setHeight(height); }
private:
  StreamingPrivate::SelectDirectoriesPrivate* const d;
};

#endif // SELECTDIRECTORIES_H
