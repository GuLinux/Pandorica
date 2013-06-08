#ifndef SETTINGSPRIVATE_H
#define SETTINGSPRIVATE_H
#include <map>
#include <boost/filesystem.hpp>

class Settings;
namespace StreamingPrivate {
class SettingsPrivate {
public:
  SettingsPrivate(Settings *q) : q(q) {}
  Wt::WLink lightySecDownloadLinkFor(std::string secDownloadPrefix, std::string secDownloadRoot, std::string secureDownloadPassword, boost::filesystem::path p);
  Wt::WLink nginxSecLinkFor(std::string secDownloadPrefix, std::string secLinkRoot, std::string secureDownloadPassword, boost::filesystem::path p);
  Settings *q;
  std::map<std::string,std::string> sessionSettings;
  std::vector<std::string> mediaDirectories;
};
}
#endif