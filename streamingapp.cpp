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
#include <Wt/WVBoxLayout>
#include <Wt/WMediaPlayer>
#include <Wt/WContainerWidget>
#include <Wt/WTreeView>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WAnchor>
#include <Wt/WFileResource>
#include <Wt/WTimer>
#include <Wt/WEnvironment>
#include <Wt/WMenu>
#include <Wt/WSubMenuItem>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/Utils>
#include <boost/algorithm/string.hpp>
#include <functional>

using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;

typedef std::function<void(boost::filesystem::path)> RunOnPath;
class StreamingAppPrivate {
public:
  void addTo ( WMenu *menu, filesystem::path p );
  WMediaPlayer::Encoding encodingFor ( filesystem::path p );
  WLink linkFor(filesystem::path p);
  bool isAllowed(filesystem::path p);
  WMenu *menu;
  string videosDir();
  WMediaPlayer *player;
  string extensionFor(filesystem::path p);
  map<string, WMediaPlayer::Encoding> types;
  StreamingAppPrivate();
  map<WMenuItem*, boost::filesystem::path> menuItemsPaths;
  map<string, boost::filesystem::path> filesHashes;
  void menuItemClicked(WMenuItem *item);
  void play(filesystem::path path);
  void setIconTo(WMenuItem *item, string url);
  WContainerWidget *infoBox;
  void parseFileParameter();
  void listDirectoryAndRun(filesystem::path directoryPath, RunOnPath runAction);
};

StreamingAppPrivate::StreamingAppPrivate() {
  types.insert(pair<string, WMediaPlayer::Encoding>(".mp3", WMediaPlayer::MP3));
  types.insert(pair<string, WMediaPlayer::Encoding>(".m4a", WMediaPlayer::M4A));
  types.insert(pair<string, WMediaPlayer::Encoding>(".m4v", WMediaPlayer::M4V));
  types.insert(pair<string, WMediaPlayer::Encoding>(".oga", WMediaPlayer::OGA));
  types.insert(pair<string, WMediaPlayer::Encoding>(".ogg", WMediaPlayer::OGA));
  types.insert(pair<string, WMediaPlayer::Encoding>(".ogv", WMediaPlayer::OGV));
  types.insert(pair<string, WMediaPlayer::Encoding>(".wav", WMediaPlayer::WAV));
  types.insert(pair<string, WMediaPlayer::Encoding>(".webm", WMediaPlayer::WEBMV));
  types.insert(pair<string, WMediaPlayer::Encoding>(".flv", WMediaPlayer::FLV));
}


StreamingApp::StreamingApp ( const Wt::WEnvironment& environment) : WApplication(environment), d(new StreamingAppPrivate) {
  WBoxLayout *layout = new WHBoxLayout();
  d->player = new WMediaPlayer(WMediaPlayer::Video);
  useStyleSheet("http://test.gulinux.net/videostreaming-styles/videostreaming.css");
//   requireJQuery("http://myrent.gulinux.net/css/jquery-latest.js");
  useStyleSheet("http://myrent.gulinux.net/css/bootstrap/css/bootstrap.css");
  useStyleSheet("http://myrent.gulinux.net/css/bootstrap/css/bootstrap-responsive.css");
  require("http://myrent.gulinux.net/css/bootstrap/js/bootstrap.js");
  d->menu = new WMenu(Wt::Vertical);
  d->menu->itemSelected().connect(d, &StreamingAppPrivate::menuItemClicked);
  WContainerWidget *menuContainer = new WContainerWidget();
  WAnchor* reloadLink = new WAnchor("javascript:false", "Reload");
  reloadLink->clicked().connect([this](WMouseEvent&){
    wApp->changeSessionId();
    wApp->redirect(wApp->bookmarkUrl());
  });
  menuContainer->addWidget(reloadLink);
  menuContainer->setOverflow(WContainerWidget::OverflowAuto);
  menuContainer->addWidget(d->menu);
  d->menu->setRenderAsList(true);
  d->menu->setStyleClass("nav nav-list");
  layout->addWidget(menuContainer);
  
  WContainerWidget *playerContainer = new WContainerWidget();
  playerContainer->addWidget(d->player);
  d->infoBox = new WContainerWidget();
  playerContainer->addWidget(d->infoBox);
  layout->addWidget(playerContainer);
  layout->setResizable(0, true, 400);
  root()->setLayout(layout);

  for(pair<string, WMediaPlayer::Encoding> encodings : d->types)
    d->player->addSource(encodings.second, "");
  
  
  d->listDirectoryAndRun(fs::path(d->videosDir()), [this](fs::path path) {
    d->addTo(d->menu, path);
  });
  d->parseFileParameter();
}


void StreamingAppPrivate::listDirectoryAndRun(filesystem::path directoryPath, RunOnPath runAction)
{
  vector<filesystem::path> v;
  copy(filesystem::directory_iterator(directoryPath), filesystem::directory_iterator(), back_inserter(v));
  sort(v.begin(), v.end());
  for(filesystem::path path: v) {
    runAction(path);
  }
}


void StreamingAppPrivate::parseFileParameter() {
  if(wApp->environment().getParameter("file")) {
    log("notice") << "Got parameter file: " << *wApp->environment().getParameter("file");
    WTimer::singleShot(1000, [this](WMouseEvent&) {
      string fileHash = * wApp->environment().getParameter("file");
      play(filesystem::path( filesHashes[fileHash]));
    });
  }    
}


void StreamingApp::refresh() {
  Wt::WApplication::refresh();
  d->parseFileParameter();
}



WMediaPlayer::Encoding StreamingAppPrivate::encodingFor ( filesystem::path p ) {
  return types[extensionFor(p)];
}

WLink StreamingAppPrivate::linkFor ( filesystem::path p ) {
  string videosDeployDir;
  if(wApp->readConfigurationProperty("videos-deploy-dir", videosDeployDir )) {
    string relpath = p.string();
    boost::replace_all(relpath, videosDir(), videosDeployDir);
    return WLink(relpath);
  }

   WLink link = WLink(new WFileResource(p.string()));
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


string StreamingAppPrivate::videosDir() {
  string videosDir = string(getenv("HOME")) + "/Video";
  wApp->readConfigurationProperty("videos-dir", videosDir);
  return videosDir;
}



class IconMenuItem : public WSubMenuItem {
public:
  IconMenuItem(string text) : WSubMenuItem(text, 0) {
    this->text = text;
  }
  virtual WWidget* createItemWidget();
  string text;
};

WWidget* IconMenuItem::createItemWidget() {
  WAnchor *anchor = new WAnchor();
  anchor->setText(text);
  return anchor;
}


void StreamingAppPrivate::addTo ( WMenu* menu, filesystem::path p ) {
  if(!isAllowed(p)) return;
  WSubMenuItem *menuItem = new WSubMenuItem(p.filename().string(), 0);
  menuItem->setPathComponent(p.string());
  WMenu *subMenu = new WMenu(Wt::Vertical);
  subMenu->itemSelected().connect(this, &StreamingAppPrivate::menuItemClicked);
  subMenu->setRenderAsList(true);
  subMenu->setStyleClass("nav nav-list");
  menuItem->setSubMenu(subMenu);
  subMenu->hide();
  menu->addItem(menuItem);
  if(fs::is_directory(p)) {
    setIconTo(menuItem, "http://test.gulinux.net/videostreaming-styles/folder.png");
    menu->itemSelected().connect([menuItem, subMenu](WMenuItem* selItem, NoClass, NoClass, NoClass, NoClass, NoClass) {
      if(selItem == menuItem) {
	if(subMenu->isVisible())
	  subMenu->animateHide(WAnimation(WAnimation::SlideInFromBottom));
	else
	  subMenu->animateShow(WAnimation(WAnimation::SlideInFromTop));
      }
    });
    listDirectoryAndRun(p, [subMenu,this](fs::path p){
      addTo(subMenu, p);
    });
  } else {
    filesHashes[Utils::hexEncode(Utils::md5(p.string()))] = p;
    setIconTo(menuItem, "http://test.gulinux.net/videostreaming-styles/video.png");
    menuItemsPaths[menuItem] = p;
  }
}

void StreamingAppPrivate::setIconTo ( WMenuItem* item, string url ) {
    WContainerWidget *cont = dynamic_cast<WContainerWidget*>(item->itemWidget());
    WImage *icon = new WImage(url);
    icon->setStyleClass("menu_item_ico");
    cont->insertWidget(0, icon);
}

void StreamingAppPrivate::menuItemClicked ( WMenuItem* item ) {
  filesystem::path path = menuItemsPaths[item];
  play(path);
}


void StreamingAppPrivate::play ( filesystem::path path ) {
  log("notice") << "Playing file " << path;
  if(!fs::is_regular_file( path ) || ! isAllowed( path )) return;
      player->stop();
  player->clearSources();
  player->addSource(encodingFor( path ), linkFor( path ));
  infoBox->clear();
  infoBox->addWidget(new WText(string("File: ") + path.filename().string()));
  WLink shareLink(wApp->bookmarkUrl("/") + string("?file=") + Utils::hexEncode(Utils::md5(path.string())));
  infoBox->addWidget(new WBreak() );
  infoBox->addWidget(new WAnchor(shareLink, "Link per la condivisione"));
  wApp->setTitle( path.filename().string());
  log("notice") << "using url " << linkFor( path ).url();
  WTimer::singleShot(1000, player, &WMediaPlayer::play);  
}



StreamingApp::~StreamingApp() {
  delete d;
}

