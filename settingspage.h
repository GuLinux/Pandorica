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
private:
  void addSetting(const std::string& settingName, Wt::WFormWidget* widget);
  Wt::WComboBox *createCombo(std::string name, std::vector<std::string> values);
  Wt::WContainerWidget *content;
  Settings *settings;
};

#endif // SETTINGSPAGE_H
