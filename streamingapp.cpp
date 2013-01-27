/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2013  Marco Gulino <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "streamingapp.h"
#include <Wt/WGridLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WMediaPlayer>
#include <Wt/WContainerWidget>
#include <Wt/WTreeView>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WFileResource>
#include <Wt/WTimer>
#include <boost/algorithm/string.hpp>

using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;
class StreamingAppPrivate {
public:
  void addTo ( WStandardItemModel* model, WStandardItem* item, filesystem::path p );
  WMediaPlayer::Encoding encodingFor ( filesystem::path p );
  WLink linkFor(filesystem::path p);
  bool isAllowed(filesystem::path p);
  string extensionFor(filesystem::path p);
  map<string, WMediaPlayer::Encoding> types;
  StreamingAppPrivate();
};

StreamingAppPrivate::StreamingAppPrivate() {
  types.insert(pair<string, WMediaPlayer::Encoding>(".mp3", WMediaPlayer::MP3));
  types.insert(pair<string, WMediaPlayer::Encoding>(".m4a", WMediaPlayer::M4A));
  types.insert(pair<string, WMediaPlayer::Encoding>(".oga", WMediaPlayer::OGA));
  types.insert(pair<string, WMediaPlayer::Encoding>(".ogg", WMediaPlayer::OGA));
  types.insert(pair<string, WMediaPlayer::Encoding>(".ogv", WMediaPlayer::OGV));
  types.insert(pair<string, WMediaPlayer::Encoding>(".wav", WMediaPlayer::WAV));
  types.insert(pair<string, WMediaPlayer::Encoding>(".webm", WMediaPlayer::WEBMV));
  types.insert(pair<string, WMediaPlayer::Encoding>(".flv", WMediaPlayer::FLV));
}


StreamingApp::StreamingApp ( const Wt::WEnvironment& environment) : WApplication(environment), d(new StreamingAppPrivate) {
  WHBoxLayout *layout = new WHBoxLayout();
  WMediaPlayer *player = new WMediaPlayer(WMediaPlayer::Video);
  WStandardItemModel *model = new WStandardItemModel(this);
  WTreeView *tree = new WTreeView();
  tree->setModel(model);
  layout->addWidget(tree);
  layout->addWidget(player);
  layout->setResizable(0, true, 250);
  root()->setLayout(layout);
  WTimer::singleShot(200, [tree] (WMouseEvent) {
    tree->setColumnWidth(0, 600);
  });

  for(pair<string, WMediaPlayer::Encoding> encodings : d->types)
    player->addSource(encodings.second, "");
  tree->doubleClicked().connect([model,player,this](WModelIndex index, WMouseEvent, NoClass, NoClass, NoClass, NoClass) {
    fs::path p = any_cast<fs::path>(model->itemFromIndex(index)->data());
    if(!fs::is_regular_file(p) || ! d->isAllowed(p)) return;
    player->addSource(d->encodingFor(p), d->linkFor(p));
    player->play();
  });
  
  for(fs::directory_iterator it(fs::path(".")); it != fs::directory_iterator(); ++it) {
      filesystem::directory_entry& entry = *it;
      d->addTo(model, 0, entry.path());
    }
}


WMediaPlayer::Encoding StreamingAppPrivate::encodingFor ( filesystem::path p ) {
  return types[extensionFor(p)];
}

WLink StreamingAppPrivate::linkFor ( filesystem::path p ) {
  WLink link = WLink(new WFileResource(p.generic_string()));
  wApp->log("notice") << "Generated url: " << link.url();
  return link;
}

string StreamingAppPrivate::extensionFor ( filesystem::path p ) {
  string extension = p.extension().string();
  boost::algorithm::to_lower(extension);
  wApp->log("notice") << "extension for " << p << ": " << extension;
  return extension;
}


bool StreamingAppPrivate::isAllowed ( filesystem::path p ) {
  return types.count(extensionFor(p)) || fs::is_directory(p);
}



void StreamingAppPrivate::addTo ( WStandardItemModel* model, WStandardItem* item, filesystem::path p ) {
  if(!isAllowed(p)) return;
  WStandardItem *newItem = new WStandardItem(WString(p.filename().wstring()));
  newItem->setData(p);
  if(model!=0) {
    model->appendRow(newItem);
  } else if(item!=0) {
    item->appendRow(newItem);
  }
  
  if(fs::is_directory(p)) {
    for(fs::directory_iterator it(p); it != fs::directory_iterator(); ++it) {
      filesystem::directory_entry& entry = *it;
      addTo(0, newItem, entry.path());
    }
  }
}



StreamingApp::~StreamingApp() {
  delete d;
}

