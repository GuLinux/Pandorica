#include "settingspage.h"
#include "Wt-Commons/wt_helpers.h"
#include "settings.h"
#include <Wt/WText>
#include <Wt/WComboBox>
#include <Wt/WLabel>
#include <Wt/WAbstractListModel>

using namespace Wt;
using namespace std;

class ComboPairModel : public Wt::WAbstractListModel {
public:
  ComboPairModel(vector<SettingPair> items, WObject* parent = 0);
  inline virtual boost::any data(const WModelIndex& index, int role = DisplayRole) const { return items[index.row()].second; }
  inline virtual int rowCount(const WModelIndex& parent = WModelIndex()) const { return items.size(); }
  inline SettingPair itemAt(int row) { return items[row]; }
private:
  vector<SettingPair> items;
};

ComboPairModel::ComboPairModel(vector< SettingPair > items, WObject* parent)
  : WAbstractListModel(parent), items(items)
{
}



SettingsPage::SettingsPage(Settings* settings, WContainerWidget* parent): WContainerWidget(parent), settings(settings)
{
  addWidget(WW(WContainerWidget).css("modal-header").add(new WText("<h3>Settings</h3>")));
  content = WW(WContainerWidget).css("modal-body form-horizontal");
  addWidget(content);
  addSetting("AutoPlay", createCombo(Settings::mediaAutoplay, {
    {"autoplay_never", "Never"},
    {"autoplay_audio_only", "Only for Audio Files"},
    {"autoplay_video_only", "Only for Video Files"},
    {"autoplay_always", "Always"}
  }));
  
  addSetting("Player", createCombo(Settings::preferredPlayer, {
    {"html5", "HTML5"},
    {"jplayer", "jPlayer"}
  }), "HTML5 is the most feature complete; jPlayer might be a bit more stable, but it lacks subtitles support");
  addSetting("Download Source", createCombo(Settings::downloadSource, {
    {"lighttpd", "Internal (lighttpd)"},
    {"nginx", "Nginx"}
  }), "Nginx could be faster in some environments, but it might not always work");
}

WComboBox* SettingsPage::createCombo(string name, vector< SettingPair > values)
{
  string defaultValue = settings->value(name);
  WComboBox *combo = new WComboBox();
  ComboPairModel* model = new ComboPairModel(values, this);
  combo->setModel(model);
  combo->activated().connect([this,name,values](int index, _n5){
    settings->setValue(name, values[index].first);
  });
  for(int index=0; index<values.size(); index++) {
    if(values[index].first==defaultValue)
      combo->setCurrentIndex(index);
  }
  return combo;
}


void SettingsPage::addSetting(const WString& label, WFormWidget* widget, const WString& helpText)
{
  WLabel *labelWidget = WW(WLabel, label).css("control-label");
  labelWidget->setBuddy(widget);
  WContainerWidget *control = WW(WContainerWidget).css("controls").add(widget);
  if(!helpText.empty()) {
    control->addWidget(WW(WText, helpText).css("help-block") );
  }
  content->addWidget(WW(WContainerWidget).css("control-group")
    .add(labelWidget)
    .add( control ));
}


SettingsPage::~SettingsPage()
{

}

