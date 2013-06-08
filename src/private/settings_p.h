/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/


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