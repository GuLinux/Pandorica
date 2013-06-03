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

#ifndef SELECTDIRECTORIESPRIVATE_H
#define SELECTDIRECTORIESPRIVATE_H

#include <vector>
#include <string>
#include <boost/filesystem.hpp>

namespace boost {
namespace filesystem {
class path;
}
}

namespace Wt {
class WTreeView;
class WStandardItem;
class WStandardItemModel;
}

class SelectDirectories;
namespace StreamingAppPrivate {
class SelectDirectoriesPrivate
{
public:
  SelectDirectoriesPrivate(SelectDirectories* q, std::vector<std::string> selectedPaths);
    virtual ~SelectDirectoriesPrivate();
    Wt::WTreeView* tree;
    Wt::WStandardItemModel* model;
    void populateTree(std::string path);
private:
    Wt::WStandardItem* buildStandardItem(boost::filesystem::path path);
    class SelectDirectories* const q;
    std::vector<std::string> selectedPaths;
};
}
#endif // SELECTDIRECTORIESPRIVATE_H
