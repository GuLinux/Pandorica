#ifndef STREAMINGAPP_PRIVATE_H
#define STREAMINGAPP_PRIVATE_H
#include <string>
#include "media.h"
#include "settings.h"
#include <boost/filesystem.hpp>
#include <Wt/WJavaScript>

class AuthPage;
class Playlist;


class MediaCollection;
class MediaCollectionBrowser;
class Player;
class StreamingApp;
namespace StreamingPrivate {
  class StreamingAppPrivate {
  public:
    StreamingAppPrivate(StreamingApp* q);
    Player *player = 0;
    std::string extensionFor(boost::filesystem::path p);
    void parseFileParameter();
    Playlist *playlist;
    Wt::WContainerWidget* playerContainerWidget;
    void setupMenus(bool isAdmin);
    void setupAdminMenus(Wt::WMenu* mainMenu);
    Session session;
    StreamingApp *q;
    Wt::WContainerWidget *mainWidget = 0;
    AuthPage *authPage;
    Wt::WTemplate* topBarTemplate;
    Wt::WStackedWidget* widgetsStack;
    void queue(Media media, bool autoplay = true);
    void queueAndPlay(Media media);
    void play(Media media);
    Wt::JSignal<std::string> playSignal;
    Wt::JSignal<std::string> queueSignal;
    Settings settings;
    MediaCollectionBrowser* mediaCollectionBrowser;
    MediaCollection* mediaCollection;
    long userId = -1;
  private:
    void setupUserMenus(Wt::WMenu* mainMenu);
    Wt::WMenuItem* activeUsersMenuItem;
    Wt::WNavigationBar* navigationBar;
    Wt::WMenuItem* mediaListMenuItem;
  };
  
}
#endif