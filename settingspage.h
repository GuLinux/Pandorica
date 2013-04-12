#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <Wt/WDialog>

class Settings;
namespace Wt {
  class WComboBox;
}

class SettingsDialog : public Wt::WDialog
{

public:
    SettingsDialog(Settings *settings, Wt::WObject* parent = 0);
    virtual ~SettingsDialog();
private:
  void addSetting(const std::string& settingName, Wt::WFormWidget* widget);
  Wt::WComboBox *createCombo(std::string name, std::vector<std::string> values);
  Wt::WContainerWidget *content;
  Settings *settings;
};

#endif // SETTINGSPAGE_H
