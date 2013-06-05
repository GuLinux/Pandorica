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
#include <Wt/WCheckBox>
#include <Wt/WTemplate>
#include <Wt/WImage>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

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
  selectDirectories->setHeight(465);
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
  Dbo::Transaction t(*session);
  bool useCache = Setting::value(Setting::useCache(), t, false);
  string cacheDirectory = Setting::value<string>(Setting::cacheDirectory(), t);
  string cacheDeployPath = Setting::value<string>(Setting::cacheDeployPath(), t);
  
  WGroupBox *container = WW<WGroupBox>(wtr("configure.app.cache_settings")).css("fieldset-small");
  WContainerWidget *cacheParamsContainer = new WContainerWidget;
  WCheckBox *cacheCheckBox = WW<WCheckBox>(wtr("configure.app.use_cache_check"));
  
  auto cacheParamsVisibility = [=]{
    cacheParamsContainer->setHidden(!cacheCheckBox->isChecked(), {WAnimation::Fade});
  };
  cacheCheckBox->changed().connect([=](_n1){
    cacheParamsVisibility();
    Dbo::Transaction t(*session);
    Setting::write(Setting::useCache(), cacheCheckBox->isChecked(), t);
    t.commit();
  });
  cacheCheckBox->setChecked(useCache);
  cacheParamsVisibility();
  container->addWidget(cacheCheckBox);
  container->addWidget(cacheParamsContainer);
  

  
  auto getPathLabel = [] (string p) { return p.empty() ? wtr("configure.app.cache.dir.empty").toUTF8() : p; };
  WText *selectedPath = new WText{getPathLabel(cacheDirectory)};
  SelectDirectories *selectDirectories = new SelectDirectories({"/"}, cacheDirectory, [=](string p) {
    Dbo::Transaction t(*session);
    Setting::write(Setting::cacheDirectory(), p, t);
    t.commit();
    selectedPath->setText(getPathLabel(p));
  }, q);
  selectDirectories->addTo(cacheParamsContainer);
  selectDirectories->setHeight(355);
  cacheParamsContainer->addWidget(new WText{wtr("configure.app.cache_path.label")});
  cacheParamsContainer->addWidget(selectedPath);
  WLineEdit *editDeployPath = WW<WLineEdit>(cacheDeployPath).css("input-xlarge");
  WPushButton *saveDeployPath = WW<WPushButton>(wtr("button.save")).css("btn btn-primary").onClick([=](WMouseEvent) {
    string valueToSave = editDeployPath->valueText().empty() ? "" : sanitizePath(editDeployPath->valueText().toUTF8());
    editDeployPath->setText(valueToSave);
    Dbo::Transaction t(*session);
    Setting::write(Setting::cacheDeployPath(), valueToSave, t);
    t.commit();
  });
  WW<WGroupBox>(wtr("configure.app.cache_deploy_path"), cacheParamsContainer).css("fieldset-small").add(
    WW<WContainerWidget>().css("input-append").add(editDeployPath).add(saveDeployPath)
  );
  return container;
}


WInteractWidget* ServerSettingsDialogPrivate::help(string titleKey, string contentKey, string side, WLength size)
{
  WImage *image = WW<WImage>(settings->staticPath("/icons/help.png")).css("link-hand")
  .setAttribute("data-toggle", "popover")
  .setAttribute("data-placement", side)
  .setAttribute("data-html", "true")
  ;
  image->doJavaScript(
    (boost::format("$('#%s').popover({title: %s, content: %s})") % image->id() 
    % wtr(titleKey).jsStringLiteral()
    % wtr(contentKey).jsStringLiteral()
    ).str()
  );
  image->resize(size, size);
  return image;  
}



void ServerSettingsDialogPrivate::buildDeployTypePage()
{
  selectDeployTypeContainer->clear();
  WContainerWidget *options = new WContainerWidget();
  WButtonGroup *btnGroup = new WButtonGroup(selectDeployTypeContainer);
  WGroupBox *radioBox = WW<WGroupBox>(wtr("configure.app.deploytype"), selectDeployTypeContainer).css("fieldset-small");
  WRadioButton *buttonInternal, *buttonStatic, *buttonLighttpd, *buttonNginx;
  btnGroup->addButton(buttonInternal =WW<WRadioButton>(wtr("configure.app.deploytype.internal")).css("span3"), Settings::DeployType::Internal);
  radioBox->addWidget(WW<WContainerWidget>().css("row").add(buttonInternal).add(help("config.help.wt_internal_deploy.title", "config.help.wt_internal_deploy.content", "bottom", 24)));

  btnGroup->addButton(buttonStatic = WW<WRadioButton>(wtr("configure.app.deploytype.static")).css("span3"), Settings::DeployType::Static);
  radioBox->addWidget(WW<WContainerWidget>().css("row").add(buttonStatic).add(help("config.help.static_deploy.title", "config.help.static_deploy.content", "bottom", 24)));
  
  btnGroup->addButton(buttonLighttpd = WW<WRadioButton>(wtr("configure.app.deploytype.lighttpd_secdownload")).css("span3"), Settings::DeployType::LighttpdSecureDownload);
  radioBox->addWidget(WW<WContainerWidget>().css("row").add(buttonLighttpd).add(help("config.help.lighttpd_secdownload.title", "config.help.lighttpd_secdownload.content", "bottom", 24)));
  
  btnGroup->addButton(buttonNginx = WW<WRadioButton>(wtr("configure.app.deploytype.nginx_seclink")).css("span3"), Settings::DeployType::NginxSecureLink);
  radioBox->addWidget(WW<WContainerWidget>().css("row").add(buttonNginx).add(help("config.help.nginx_seclink.title", "config.help.nginx_seclink.content", "bottom", 24)));
  
  Dbo::Transaction t(*session);
  Settings::DeployType deployType = (Settings::DeployType) Setting::value<int>(Setting::deployType(), t, Settings::DeployType::Internal);
  btnGroup->setCheckedButton(btnGroup->button( deployType ));
  
  auto setupDeployOptions = [=](Settings::DeployType deployType) {
    if(deployType != Settings::DeployType::Internal) {
      Dbo::Transaction t(*session);
      
      if(deployType == Settings::DeployType::LighttpdSecureDownload || deployType == Settings::DeployType::NginxSecureLink) {
        WLineEdit *editPassword = WW<WLineEdit>(Setting::value<string>(Setting::secureDownloadPassword(), t)).css("input-xlarge");
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
        string value = Setting::value<string>(Setting::deployPath(directory), t);
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
