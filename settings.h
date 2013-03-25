#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <Wt/WLink>
#include <boost/filesystem.hpp>
#include "media.h"

class Player;
class SettingsPrivate;
class Settings
{

public:
    Settings();
    virtual ~Settings();
    std::string videosDir() const;
    boost::filesystem::path mediaData() const;
    std::string value(std::string cookieName);
    void setValue(std::string settingName, std::string value);
    Wt::WLink linkFor(boost::filesystem::path p);
    Player *newPlayer();
    bool autoplay(const Media &media);
    static const std::string downloadSource;
    static const std::string mediaAutoplay;
    static const std::string preferredPlayer;
private:
  SettingsPrivate *const d;
};

#endif // SETTINGS_H
