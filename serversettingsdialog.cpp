/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "serversettingsdialog.h"
#include "private/serversettingsdialog_p.h"
#include "settings.h"
#include "session.h"
#include "Wt-Commons/wt_helpers.h"
#include "mediacollection.h"
#include "selectdirectories.h"
#include "Models/setting.h"
#include <boost/thread.hpp>
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WInPlaceEdit>
#include <boost/filesystem.hpp>

using namespace std;
using namespace Wt;
using namespace StreamingPrivate;
using namespace WtCommons;

namespace fs = boost::filesystem;

ServerSettingsDialogPrivate::ServerSettingsDialogPrivate(Settings* settings, Session* session, MediaCollection *mediaCollection, ServerSettingsDialog* q)
  : settings(settings), session(session), mediaCollection(mediaCollection), q(q)
{

}

ServerSettingsDialogPrivate::~ServerSettingsDialogPrivate()
{
}

ServerSettingsDialog::~ServerSettingsDialog()
{
    delete d;

}

ServerSettingsDialog::ServerSettingsDialog(Settings* settings, Session* session, MediaCollection *mediaCollection, WObject* parent)
    : WDialog(parent), d(new ServerSettingsDialogPrivate(settings, session, mediaCollection, this))
{
  setWindowTitle(wtr("menu.setmediaroot"));
  WStackedWidget *stack = new WStackedWidget(contents());
  setClosable(false);
  setHeight(600);
  stack->addWidget(d->selectMediaRootPage());
  stack->addWidget(d->selectDeployTypePage());
  
  footer()->addWidget(d->buttonNext = WW<WPushButton>(wtr("button.next")).css("btn").onClick([=](WMouseEvent) {
    int nextIndex{stack->currentIndex()+1};
    if(nextIndex==stack->count()-1) {
      d->buttonNext->disable();
      d->buttonOk->enable();
    }
    stack->setCurrentIndex(nextIndex);
  }));
  footer()->addWidget(d->buttonOk = WW<WPushButton>(wtr("button.ok")).disable().css("btn btn-primary").onClick([=](WMouseEvent) { accept(); }));
  finished().connect([=](WDialog::DialogCode, _n5) {
    boost::thread t([=]{
      Session privateSession;
      Dbo::Transaction t(privateSession);
      mediaCollection->rescan(t);
    });
  });
}

WContainerWidget* ServerSettingsDialogPrivate::selectMediaRootPage()
{
  SelectDirectories *selectDirectories = new SelectDirectories({"/"}, settings->mediasDirectories(session), [=](string p){
    settings->addMediaDirectory(p, session);
  }, [=](string p){
    settings->removeMediaDirectory(p, session);
  }, q );
  selectDirectories->setHeight(460);
  WContainerWidget *selectDirectoriesContainer = new WContainerWidget;
  selectDirectories->addTo(selectDirectoriesContainer);
  return selectDirectoriesContainer;
}

WContainerWidget* ServerSettingsDialogPrivate::selectDeployTypePage()
{
  WContainerWidget *container = WW<WContainerWidget>();
  WContainerWidget *options = new WContainerWidget();
  WButtonGroup *btnGroup = new WButtonGroup(container);
  
  btnGroup->addButton(WW<WRadioButton>("Internal", container).setInline(false), Settings::DeployType::Internal);
  btnGroup->addButton(WW<WRadioButton>("Static Folder", container).setInline(false), Settings::DeployType::Static);
  btnGroup->addButton(WW<WRadioButton>("Lighttpd Secure Download", container).setInline(false), Settings::DeployType::LighttpdSecureDownload);
  btnGroup->addButton(WW<WRadioButton>("Nginx Secure Link", container).setInline(false), Settings::DeployType::NginxSecureLink);
  
  Dbo::Transaction t(*session);
  Settings::DeployType deployType = (Settings::DeployType) Setting::value<int>(Setting::deployType(), t, Settings::DeployType::Internal);
  btnGroup->setCheckedButton(btnGroup->button( deployType ));
  
  auto setupDeployOptions = [=](Settings::DeployType deployType) {
    if(deployType != Settings::DeployType::Internal) {
      Dbo::Transaction t(*session);
      for(string directory: settings->mediasDirectories(session)) {
        string directoryName = fs::path(directory).filename().string();
        string value = Setting::value(Setting::deployPath(directory), t, string{});
        WLabel *label = WW<WLabel>(string{"Deploy path for "} + directoryName, options).css("label label-info").setInline(false);
        WInPlaceEdit *editDeployPath = WW<WInPlaceEdit>(value, options).setInline(false);
        editDeployPath->valueChanged().connect([=](WString newValue, _n5){
          string valueToSave = newValue.toUTF8();
          if(valueToSave[0] != '/')
            valueToSave = string{"/"} + valueToSave;
          if(valueToSave[valueToSave.size()-1] != '/')
            valueToSave += "/";
          editDeployPath->setText(valueToSave);
          Dbo::Transaction t(*session);
          Setting::write(Setting::deployPath(directory), valueToSave, t);
          t.commit();
        });
        editDeployPath->setEmptyText("click to set deploy directory");
      }
      if(deployType == Settings::DeployType::LighttpdSecureDownload || deployType == Settings::DeployType::NginxSecureLink) {
        WW<WLabel>("Secret Password for Secure Download/Secure Link", options).css("label label-info").setInline(false);
        
        WLineEdit *editPassword = WW<WLineEdit>(Setting::value(Setting::secureDownloadPassword(), t, string{}));
        editPassword->setEchoMode(WLineEdit::EchoMode::Password);
        WPushButton *savePassword = WW<WPushButton>("Save").onClick([=](WMouseEvent) {
          Dbo::Transaction t(*session);
          Setting::write(Setting::secureDownloadPassword(), editPassword->valueText().toUTF8(), t);
          t.commit();
        });
        options->addWidget(WW<WContainerWidget>().css("input-append input-block-level").add(editPassword).add(savePassword));
      }
    }
  };
  setupDeployOptions(deployType);
  btnGroup->checkedChanged().connect([=](WRadioButton *button, _n5){
    Settings::DeployType deployType = (Settings::DeployType) btnGroup->id(button);
    Dbo::Transaction t(*session);
    Setting::write(Setting::deployType(), deployType, t);
    options->clear();
    setupDeployOptions(deployType);
    t.commit();
  });
  container->addWidget(options);
  return container;
}

void ServerSettingsDialog::run()
{
  show();
}
