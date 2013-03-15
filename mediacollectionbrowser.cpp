#include "mediacollectionbrowser.h"
#include "mediacollection.h"
#include <Wt/WMenu>
#include <Wt/WAnchor>
#include <Wt/WText>
#include <Wt/WImage>
#include "Wt-Commons/wt_helpers.h"

using namespace Wt;
using namespace std;
using namespace boost;

namespace fs = boost::filesystem;

typedef pair<string,Media> MediaEntry;

class MediaCollectionBrowserPrivate {
public:
  MediaCollectionBrowserPrivate(MediaCollection *collection)
    : collection(collection) {}
    void rebuildBreadcrumb();
    void browse(filesystem::path currentPath);
    
public:
  MediaCollection *const collection;
  filesystem::path currentPath;
  WContainerWidget* breadcrumb;
    WContainerWidget* browser;
private:
    void addDirectory(filesystem::path directory);
    void addMedia(Media media);
    WContainerWidget* addIcon(string filename, string icon, MouseEventListener onClick);
};

MediaCollectionBrowser::MediaCollectionBrowser(MediaCollection *collection, Wt::WContainerWidget* parent)
  : WContainerWidget(parent), d(new MediaCollectionBrowserPrivate(collection))
{
  d->breadcrumb = WW(WContainerWidget).css("breadcrumb");
  d->breadcrumb->setList(true);
  d->browser = WW(WContainerWidget).css("thumbnails").setMargin(WLength::Auto, Left).setMargin(WLength::Auto, Right);
  d->browser->setList(true);
  addWidget(d->breadcrumb);
  addWidget(d->browser);
  d->browse(d->collection->rootPath());
}

void MediaCollectionBrowserPrivate::browse(filesystem::path currentPath)
{
  wApp->log("notice") << "**** browsing " << currentPath.string();
  this->currentPath = currentPath;
  browser->clear();
  rebuildBreadcrumb();
  auto belongsToCurrent = [this](fs::path p){
    return p.parent_path() == this->currentPath;
  };

  set<fs::path> directories;
  list<Media> medias;
  
  for(MediaEntry m: collection->collection()) {
    if(belongsToCurrent(m.second.path()))
      medias.push_back(m.second);
    fs::path directory = m.second.parentDirectory();
    if(directory != currentPath && belongsToCurrent(directory) && !directories.count(directory))
      directories.insert(directory);
  }
  
//   std::sort(directories.begin(), directories.end(), [](fs::path a, fs::path b) { return a.filename().string() < b.filename().string(); } );
//   std::sort(medias.begin(), medias.end(), [](Media a, Media b) { return (a.filename() <b.filename()); } );
  
  for(fs::path directory: directories) addDirectory(directory);
  for(Media media: medias) addMedia(media);
}

void MediaCollectionBrowserPrivate::addDirectory(filesystem::path directory)
{
  wApp->log("notice") << "Adding directory " << directory.filename().string();
  auto onClick = [this,directory](WMouseEvent){
    browse(directory);
  };
  addIcon(directory.filename().string(), "http://gulinux.net/css/folder.png", onClick);
}

void MediaCollectionBrowserPrivate::addMedia(Media media)
{
  wApp->log("notice") << "Adding media " << media.filename();
  auto onClick = [](WMouseEvent){};
  addIcon(media.filename(), "http://gulinux.net/css/video.png", onClick);
}

WContainerWidget* MediaCollectionBrowserPrivate::addIcon(string filename, string icon, MouseEventListener onClick)
{
    WContainerWidget *item = WW(WContainerWidget).css("span4");
    item->setContentAlignment(AlignmentFlag::AlignCenter);
    WAnchor *link = WW(WAnchor, "#").css("thumbnail");
    link->setImage(new WImage(icon));
    link->setText(filename);
    item->addWidget(link);
    link->clicked().connect(onClick);
    browser->addWidget(item);
    return item;
}



void MediaCollectionBrowserPrivate::rebuildBreadcrumb()
{
  breadcrumb->clear();
  list<fs::path> paths;
  fs::path p = currentPath;
  while(p != collection->rootPath()) {
    paths.push_front(p);
    p = p.parent_path();
  }
  paths.push_front(p);
  
  for(fs::path p: paths) {
    WContainerWidget *item = new WContainerWidget;
    if(breadcrumb->count())
      item->addWidget(WW(WText, "/").css("divider"));
    item->addWidget( WW(WAnchor, "#", p.filename().string()).onClick([this,p](WMouseEvent){
      browse(p);
    }) );
    breadcrumb->addWidget(item);
  }
}


MediaCollectionBrowser::~MediaCollectionBrowser()
{

}

