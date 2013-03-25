#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <Wt/WContainerWidget>

class Settings;
namespace Wt {
  class WComboBox;
}

typedef std::pair<std::string,Wt::WString> SettingPair;
class SettingsPage : public Wt::WContainerWidget
{

public:
    SettingsPage(Settings *settings, Wt::WContainerWidget* parent = 0);
    virtual ~SettingsPage();
private:
  void addSetting(const Wt::WString &label, Wt::WFormWidget *widget, const Wt::WString &helpText = Wt::WString());
  Wt::WComboBox *createCombo(std::string name, std::vector<SettingPair> values);
  Wt::WContainerWidget *content;
  Settings *settings;
};

#endif // SETTINGSPAGE_H
