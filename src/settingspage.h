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


#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <Wt/WContainerWidget>

class Settings;
namespace Wt {
  class WComboBox;
}

class SettingsPage : public Wt::WContainerWidget
{

public:
    SettingsPage(Settings *settings, Wt::WContainerWidget* parent = 0);
    virtual ~SettingsPage();
    static void dialog(Settings *settings, Wt::WObject *parent = 0);
private:
  void addSetting(const std::string& settingName, Wt::WFormWidget* widget);
  Wt::WComboBox *createCombo(std::string name, std::vector<std::string> values);
  Settings *settings;
};

#endif // SETTINGSPAGE_H
