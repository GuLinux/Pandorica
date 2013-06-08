/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

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
