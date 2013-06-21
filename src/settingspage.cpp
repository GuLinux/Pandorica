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
using namespace WtCommons;

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
  setClosable(true);
  setResizable(true);
  setMaximumSize(700, WLength::Auto);
  content = WW<WContainerWidget>().css("form-horizontal");
  contents()->addWidget(content);
  addSetting(Settings::mediaAutoplay, createCombo(Settings::mediaAutoplay, { "autoplay_never", "autoplay_audio_only", "autoplay_video_only", "autoplay_always" }));
  
  addSetting(Settings::preferredPlayer, createCombo(Settings::preferredPlayer, { "mediaelementjs", "purehtml5", "videojs", "jplayer"} ) );
  addSetting(Settings::guiLanguage, createCombo(Settings::guiLanguage, {"<browserdefault>", "en", "it"}) );
  contents()->addWidget(new WText{wtr("settings.footer")});
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

