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
#include <boost/thread.hpp>
#include "Wt-Commons/wt_helpers.h"


using namespace StreamingPrivate;
using namespace std;
using namespace Wt;

namespace fs = boost::filesystem;

SelectDirectoriesPrivate::SelectDirectoriesPrivate(SelectDirectories* q, vector< string > selectedPaths, SelectDirectories::SelectionType selectionType)
: q(q), selectedPaths(selectedPaths), selectionType(selectionType)
{
}
SelectDirectoriesPrivate::~SelectDirectoriesPrivate()
{
}

SelectDirectories::SelectDirectories(vector< string > rootPaths, string selectedPath, OnPathClicked onPathSelected, WObject* parent)
   : SelectDirectories(rootPaths, {selectedPath}, onPathSelected, [](string){}, Single, parent)
{

}


SelectDirectories::SelectDirectories(vector< string > rootPaths, vector< string > selectedPaths, OnPathClicked onPathSelected, OnPathClicked onPathUnselected, SelectionType selectionType, WObject* parent)
    : WObject(parent), d(new SelectDirectoriesPrivate(this, selectedPaths, selectionType))
{
    WTreeView *tree = new WTreeView();
    d->tree = tree;
    d->app = wApp;
    d->model = new WStandardItemModel(this);
    tree->setMinimumSize(400, WLength::Auto);
    tree->setHeaderHeight(0);
    tree->setSortingEnabled(false);
    tree->setModel(d->model);
    tree->setRootIsDecorated(true);

    tree->doubleClicked().connect([=](WModelIndex index, WMouseEvent, _n4) {
        tree->setExpanded(index, !tree->isExpanded(index));
    });
    
    if(selectionType == Single) {
      d->tree->clicked().connect([=](WModelIndex index, WMouseEvent, _n4) {
        WStandardItem *item = d->model->itemFromIndex(index);
        fs::path p = boost::any_cast<fs::path>(item->data());
        onPathSelected(p.string());
      });
    }
    
    d->model->itemChanged().connect([=](WStandardItem *item, _n5) {
        bool itemChecked = (item->checkState() == Wt::Checked);
        string itemPath = boost::any_cast<fs::path>(item->data()).string();
        if(itemChecked) {
          onPathSelected(itemPath);
        } else {
          onPathUnselected(itemPath);
        }
    });
    
    tree->expanded().connect([=](WModelIndex index, _n5) {
      WStandardItem *item = d->model->itemFromIndex(index);
      boost::thread t([=] {
        for(int i=0; i<item->rowCount(); i++) {
          d->addSubItems(item->child(i));
        }
      });
    });

    for(string path: rootPaths)
        d->populateTree(path);
    // TODO: expand selected items
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
    model->appendRow(buildStandardItem(path, true));
    for(string p: selectedPaths) {
      if(items.count(fs::path(p)) > 0) {
        WStandardItem *item = items[fs::path(p)];
        while(item->parent() != 0) {
          item = item->parent();
          tree->expand(model->indexFromItem(item));
        }
      }
    }
};


void SelectDirectories::addTo(WContainerWidget* container)
{
    container->addWidget(d->tree);
}

WStandardItem* SelectDirectoriesPrivate::buildStandardItem(boost::filesystem::path path, bool shouldAddSubItems)
{
  string folderName {path.filename().string()};
  WStandardItem* item = new WStandardItem {Settings::icon(Settings::FolderSmall), folderName};
  item->setCheckable(selectionType == SelectDirectories::Multiple);
  item->setStyleClass("tree-directory-item link-hand");
  item->setLink("");
  item->setToolTip(wtr("tree.double.click.to.expand"));
  item->setData(path);
  bool hasSelectedSubItems = false;
  for(string pathSelected: selectedPaths) {
    hasSelectedSubItems |= pathSelected.find(path.parent_path().string()) != string::npos;
    if(pathSelected == path.string())
      item->setChecked(true);
  }
  if(shouldAddSubItems || hasSelectedSubItems) {
    addSubItems(item, true);
  }
  items[path] = item;
  return item;
}

void SelectDirectoriesPrivate::addSubItems(WStandardItem* item, bool sync)
{
  fs::path path = boost::any_cast<fs::path>(item->data());
  try {
    fs::directory_iterator it {path};
    vector<fs::path> paths;
    copy_if(it, fs::directory_iterator(), back_insert_iterator<vector<fs::path>>(paths), [=](fs::path p) { return fs::is_directory(p) && p.filename().string()[0] != '.'; });
    sort(paths.begin(), paths.end(), [=](fs::path a, fs::path b) { return a.filename() < b.filename(); });
    auto addItems = [=] {
      for_each(paths.begin(), paths.end(), [=](fs::path p) {
        if(items.count(p) >0) return;
        item->appendRow( buildStandardItem(p, false) );
      });
    };
    if(sync) {
      addItems();
    }
    else {
      WServer::instance()->post(app->sessionId(), [=] {
        addItems();
        app->triggerUpdate();
      });
    }
  } catch(std::exception &e) {
    log("warning") << "Error adding subdirectories for path " << path << ": " << e.what();
  }
}


SelectDirectories::~SelectDirectories()
{
    delete d;
}
