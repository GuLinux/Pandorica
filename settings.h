#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <Wt/WLink>
#include <boost/filesystem.hpp>

class Player;
class SettingsPrivate;
class Settings
{

public:
    Settings();
    virtual ~Settings();
    std::string videosDir() const;
    boost::filesystem::path mediaData() const;
    std::string value(std::string cookieName, std::string defaultValue);
    Wt::WLink linkFor(boost::filesystem::path p);
    Player *newPlayer();
    bool autoplay();
private:
  SettingsPrivate *const d;
};

#endif // SETTINGS_H
