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
#include <Wt/WGroupBox>
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
  setWindowTitle(wtr("menu.configure.app"));
  WStackedWidget *stack = new WStackedWidget(contents());
  setClosable(false);
  setHeight(650);
  setWidth(600);
  stack->addWidget(d->selectMediaRootPage());
  stack->addWidget(d->selectDeployTypeContainer = new WContainerWidget);
  stack->addWidget(d->cachePage());
  d->buildDeployTypePage();
  
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
  WGroupBox *groupBox = WW<WGroupBox>(wtr("configure.app.select_media_directories")).css("fieldset-small");
  SelectDirectories *selectDirectories = new SelectDirectories({"/"}, settings->mediasDirectories(session), [=](string p){
    settings->addMediaDirectory(p, session);
    buildDeployTypePage();
  }, [=](string p){
    settings->removeMediaDirectory(p, session);
    buildDeployTypePage();
  }, SelectDirectories::Multiple, q );
  selectDirectories->setHeight(450);
  selectDirectories->addTo(groupBox);
  return groupBox;
}

string sanitizePath(string deployPath) {
  if(deployPath[0] != '/')
    deployPath = string{"/"} + deployPath;
  if(deployPath[deployPath.size()-1] != '/')
    deployPath += "/";
  return deployPath;
}

WContainerWidget* ServerSettingsDialogPrivate::cachePage()
{
  WContainerWidget *container = new WContainerWidget;
  Dbo::Transaction t(*session);
  string cacheDirectory = Setting::value(Setting::cacheDirectory(), t, string{});
  string cacheDeployPath = Setting::value(Setting::cacheDeployPath(), t, string{});
  
  auto getPathLabel = [] (string p) { return p.empty() ? wtr("configure.app.cache.dir.empty").toUTF8() : p; };
  WText *selectedPath = new WText{getPathLabel(cacheDirectory)};
  SelectDirectories *selectDirectories = new SelectDirectories({"/"}, cacheDirectory, [=](string p) {
    Dbo::Transaction t(*session);
    Setting::write(Setting::cacheDirectory(), p, t);
    t.commit();
    selectedPath->setText(getPathLabel(p));
  }, q);
  selectDirectories->addTo(container);
  selectDirectories->setHeight(390);
  container->addWidget(new WText{wtr("configure.app.cache_path.label")});
  container->addWidget(selectedPath);
  WLineEdit *editDeployPath = WW<WLineEdit>(cacheDeployPath).css("input-xlarge");
  WPushButton *saveDeployPath = WW<WPushButton>(wtr("button.save")).css("btn btn-primary").onClick([=](WMouseEvent) {
    string valueToSave = editDeployPath->valueText().empty() ? "" : sanitizePath(editDeployPath->valueText().toUTF8());
    editDeployPath->setText(valueToSave);
    Dbo::Transaction t(*session);
    Setting::write(Setting::cacheDeployPath(), valueToSave, t);
    t.commit();
  });
  WW<WGroupBox>(wtr("configure.app.cache_deploy_path"), container).css("fieldset-small").add(
    WW<WContainerWidget>().css("input-append").add(editDeployPath).add(saveDeployPath)
  );
  return container;
}




void ServerSettingsDialogPrivate::buildDeployTypePage()
{
  selectDeployTypeContainer->clear();
  WContainerWidget *options = new WContainerWidget();
  WButtonGroup *btnGroup = new WButtonGroup(selectDeployTypeContainer);
  WGroupBox *radioBox = WW<WGroupBox>(wtr("configure.app.deploytype"), selectDeployTypeContainer).css("fieldset-small");
  
  btnGroup->addButton(WW<WRadioButton>(wtr("configure.app.deploytype.internal"), radioBox).setInline(false), Settings::DeployType::Internal);
  btnGroup->addButton(WW<WRadioButton>(wtr("configure.app.deploytype.static"), radioBox).setInline(false), Settings::DeployType::Static);
  btnGroup->addButton(WW<WRadioButton>(wtr("configure.app.deploytype.lighttpd_secdownload"), radioBox).setInline(false), Settings::DeployType::LighttpdSecureDownload);
  btnGroup->addButton(WW<WRadioButton>(wtr("configure.app.deploytype.nginx_seclink"), radioBox).setInline(false), Settings::DeployType::NginxSecureLink);
  
  Dbo::Transaction t(*session);
  Settings::DeployType deployType = (Settings::DeployType) Setting::value<int>(Setting::deployType(), t, Settings::DeployType::Internal);
  btnGroup->setCheckedButton(btnGroup->button( deployType ));
  
  auto setupDeployOptions = [=](Settings::DeployType deployType) {
    if(deployType != Settings::DeployType::Internal) {
      Dbo::Transaction t(*session);
      
      if(deployType == Settings::DeployType::LighttpdSecureDownload || deployType == Settings::DeployType::NginxSecureLink) {
        WLineEdit *editPassword = WW<WLineEdit>(Setting::value(Setting::secureDownloadPassword(), t, string{})).css("input-xlarge");
        editPassword->setEchoMode(WLineEdit::EchoMode::Password);
        WPushButton *savePassword = WW<WPushButton>("Save").css("btn btn-primary").onClick([=](WMouseEvent) {
          Dbo::Transaction t(*session);
          Setting::write(Setting::secureDownloadPassword(), editPassword->valueText().toUTF8(), t);
          t.commit();
        });
        WW<WGroupBox>(wtr("configure.app.deploytype.secdl_password_label"), options).css("fieldset-small").add(
          WW<WContainerWidget>().css("input-append").add(editPassword).add(savePassword)
        );
      }
      
      for(string directory: settings->mediasDirectories(session)) {
        string directoryName = fs::path(directory).filename().string();
        string value = Setting::value(Setting::deployPath(directory), t, string{});
        WLineEdit *editDeployPath = WW<WLineEdit>(value).css("input-xlarge");
        WPushButton *saveDeployPath = WW<WPushButton>(wtr("button.save")).css("btn btn-primary").onClick([=](WMouseEvent) {
          string valueToSave = sanitizePath(editDeployPath->valueText().toUTF8());
          editDeployPath->setText(valueToSave);
          Dbo::Transaction t(*session);
          Setting::write(Setting::deployPath(directory), valueToSave, t);
          t.commit();
        });
        WW<WGroupBox>(wtr("configure.app.deploy.path.label").arg(directoryName), options).css("fieldset-small").add(
          WW<WContainerWidget>().css("input-append").add(editDeployPath).add(saveDeployPath)
        );
        editDeployPath->setEmptyText(wtr("configure.app.deploy_dir"));
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
  selectDeployTypeContainer->addWidget(options);
}

void ServerSettingsDialog::run()
{
  show();
}
