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
    d->app = wApp;
    d->model = new WStandardItemModel(this);
    tree->setMinimumSize(400, WLength::Auto);
    tree->setModel(d->model);
    tree->setRootIsDecorated(true);

    tree->doubleClicked().connect([=](WModelIndex index, WMouseEvent, _n4) {
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
    model->clear();
    model->appendRow(buildStandardItem(path, true));
};

void SelectDirectories::addTo(WContainerWidget* container)
{
    container->addWidget(d->tree);
}

WStandardItem* SelectDirectoriesPrivate::buildStandardItem(boost::filesystem::path path, bool shouldAddSubItems)
{
  string folderName {path.filename().string()};
  WStandardItem* item = new WStandardItem {Settings::icon(Settings::FolderSmall), folderName};
  item->setCheckable(true);
  item->setStyleClass("tree-directory-item link-hand");
  item->setLink("");
  item->setToolTip(wtr("tree.double.click.to.expand"));
  for(string pathSelected: selectedPaths)
    if(pathSelected == path.string())
      item->setChecked(true);
    item->setData(path);
  if(shouldAddSubItems)
    addSubItems(item);
  return item;
}

void SelectDirectoriesPrivate::addSubItems(WStandardItem* item)
{
  WServer::instance()->post(app->sessionId(), [=] { item->removeRows(0, item->rowCount()); });
  fs::path path = boost::any_cast<fs::path>(item->data());
  log("notice") << "Adding sub-items for " << path;
  try {
    fs::directory_iterator it {path};
    vector<fs::path> paths;
    copy_if(it, fs::directory_iterator(), back_insert_iterator<vector<fs::path>>(paths), [=](fs::path p) { return fs::is_directory(p) && p.filename().string()[0] != '.'; });
    sort(paths.begin(), paths.end(), [=](fs::path a, fs::path b) { return a.filename() < b.filename(); });
    for(fs::path p: paths) {
      WServer::instance()->post(app->sessionId(), [=] {
        item->appendRow(buildStandardItem(p, false));
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
