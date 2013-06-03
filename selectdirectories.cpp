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

#include "selectdirectories.h"
#include "selectdirectories_p.h"
#include "settings.h"
#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WContainerWidget>
#include <boost/filesystem.hpp>
#include "Wt-Commons/wt_helpers.h"


using namespace StreamingAppPrivate;
using namespace std;
using namespace Wt;

namespace fs = boost::filesystem;

SelectDirectoriesPrivate::SelectDirectoriesPrivate(SelectDirectories* q, vector<string> selectedPaths) : q(q), selectedPaths(selectedPaths)
{
}
SelectDirectoriesPrivate::~SelectDirectoriesPrivate()
{
}

SelectDirectories::SelectDirectories(vector< string > rootPaths, vector< string > selectedPaths, OnPathClicked onPathSelected, OnPathClicked onPathUnselected, WObject* parent)
  : WObject(parent), d(new SelectDirectoriesPrivate(this, selectedPaths))
{
  WTreeView *tree = new WTreeView();
  d->tree = tree;
  d->model = new WStandardItemModel(this);
  tree->setMinimumSize(400, WLength::Auto);
  tree->setModel(d->model);
  tree->setHeight(320);
  tree->setRootIsDecorated(false);
  
  tree->doubleClicked().connect([=](WModelIndex index, WMouseEvent, _n4){
    tree->setExpanded(index, !tree->isExpanded(index));
  });
  
  d->model->itemChanged().connect([=](WStandardItem *item, _n5) {
    bool itemChecked = (item->checkState() == Wt::Checked);
    string itemPath = boost::any_cast<fs::path>(item->data()).string();
    if(itemChecked) {
      onPathSelected(itemPath);
    } else {
      onPathUnselected(itemPath);
    }
  });
  
  for(string path: rootPaths)
    d->populateTree(path);
}

void SelectDirectories::setHeight(WLength height)
{
  d->tree->setHeight(height);
}

void SelectDirectories::setWidth(WLength width)
{
  d->tree->setWidth(width);
}


void SelectDirectoriesPrivate::populateTree(std::string path)
{
  model->clear();
  map<fs::path, WStandardItem*> items{
    {path, buildStandardItem(path)}
  };
  model->appendRow(items[path]);
  fs::recursive_directory_iterator it{path, fs::symlink_option::recurse};
  while(it != fs::recursive_directory_iterator() ) {
    if(fs::is_directory(*it)) {
      WStandardItem *item = buildStandardItem(*it);
      items[it->path()] = item;
      items[it->path().parent_path()]->appendRow(item);
    }
    it++;
  }
  tree->expandToDepth(1);
};

void SelectDirectories::addTo(WContainerWidget* container)
{
  container->addWidget(d->tree);
}

WStandardItem* SelectDirectoriesPrivate::buildStandardItem(fs::path path)
{
  string folderName{path.filename().string()};
  WStandardItem* item = new WStandardItem{Settings::icon(Settings::FolderSmall), folderName};
  item->setCheckable(true);
  item->setStyleClass("tree-directory-item link-hand");
  item->setLink("");
  item->setToolTip(wtr("tree.double.click.to.expand"));
  for(string pathSelected: selectedPaths)
    if(pathSelected == path.string())
      item->setChecked(true);
    item->setData(path);
  return item;
}


SelectDirectories::~SelectDirectories()
{
  delete d;
}
