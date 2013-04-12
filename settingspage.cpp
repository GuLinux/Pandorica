#include "settingspage.h"
#include "Wt-Commons/wt_helpers.h"
#include "settings.h"
#include <Wt/WText>
#include <Wt/WComboBox>
#include <Wt/WLabel>
#include <Wt/WAbstractListModel>
#include <Wt/WPushButton>

using namespace Wt;
using namespace std;

class ComboPairModel : public Wt::WAbstractListModel {
public:
  ComboPairModel(vector<string> items, WObject* parent = 0);
  inline virtual boost::any data(const WModelIndex& index, int role = Wt::DisplayRole) const { return wtr(items[index.row()] + ".combo"); }
  inline virtual int rowCount(const WModelIndex& parent = WModelIndex()) const { return items.size(); }
  inline string itemAt(int row) { return items[row]; }
private:
  vector<string> items;
};

ComboPairModel::ComboPairModel(vector<string> items, WObject* parent)
  : WAbstractListModel(parent), items(items)
{
}



SettingsDialog::SettingsDialog(Settings* settings, WObject* parent): WDialog(parent), settings(settings)
{
  setWindowTitle(wtr("settings.header"));
  setMaximumSize(700, WLength::Auto);
  content = WW<WContainerWidget>().css("form-horizontal");
  contents()->addWidget(content);
  addSetting(Settings::mediaAutoplay, createCombo(Settings::mediaAutoplay, { "autoplay_never", "autoplay_audio_only", "autoplay_video_only", "autoplay_always" }));
  
  addSetting(Settings::preferredPlayer, createCombo(Settings::preferredPlayer, { "html5", "jplayer"} ) );
  addSetting(Settings::downloadSource, createCombo(Settings::downloadSource, {"lighttpd", "nginx"}) );
  addSetting(Settings::guiLanguage, createCombo(Settings::guiLanguage, {"<browserdefault>", "en", "it"}) );
  contents()->addWidget(new WText{wtr("settings.footer")});
  footer()->addWidget(WW<WPushButton>(wtr("close-button")).css("btn btn-primary").onClick([=](WMouseEvent){ accept(); }));
}

WComboBox* SettingsDialog::createCombo(string name, vector<string> values)
{
  string defaultValue = settings->value(name);
  WComboBox *combo = new WComboBox();
  ComboPairModel* model = new ComboPairModel(values, this);
  combo->setModel(model);
  combo->activated().connect([=](int index, _n5){
    settings->setValue(name, values[index]);
  });
  for(int index=0; index<values.size(); index++) {
    if(values[index]==defaultValue)
      combo->setCurrentIndex(index);
  }
  return combo;
}


void SettingsDialog::addSetting(const string& settingName, WFormWidget* widget)
{
  WString label = wtr(settingName + ".label");
  WString helpText = wtr(settingName + ".description");
  
  WLabel *labelWidget = WW<WLabel>(label).css("control-label");
  labelWidget->setBuddy(widget);
  WContainerWidget *control = WW<WContainerWidget>().css("controls").add(widget);
  if(!helpText.empty()) {
    control->addWidget(WW<WText>(helpText).css("help-block") );
  }
  content->addWidget(WW<WContainerWidget>().css("control-group")
    .add(labelWidget)
    .add( control ));
}


SettingsDialog::~SettingsDialog()
{

}

