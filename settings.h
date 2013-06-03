#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <Wt/WLink>
#include <boost/filesystem.hpp>
#include "media.h"

class Player;
namespace StreamingPrivate {
  class SettingsPrivate;
}
class Settings
{

public:
    Settings();
    virtual ~Settings();
    std::vector<std::string> mediasDirectories() const;
    void setMediasDirectories(std::vector<std::string> directories);
    std::string value(std::string cookieName);
    std::string locale();
    void setValue(std::string settingName, std::string value);
    Wt::WLink linkFor(boost::filesystem::path p);
    Wt::WLink shareLink(std::string mediaId);
    Player *newPlayer();
    bool autoplay(const Media &media);
    static const std::string guiLanguage;
    static const std::string downloadSource;
    static const std::string mediaAutoplay;
    static const std::string preferredPlayer;
    enum Icons { FolderBig, FolderSmall, VideoFile, AudioFile };
    static std::string icon(Icons icon);
    static std::string staticPath(const std::string &relativeUrl);
    static std::string staticDeployPath();
    std::string relativePath(std::string mediaPath, bool removeTrailingSlash = false) const;
private:
  StreamingPrivate::SettingsPrivate *const d;
};

#endif // SETTINGS_H
