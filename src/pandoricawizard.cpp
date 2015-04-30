/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pandoricawizard.h"
#include "pandoricawizard_p.h"
#include "utils/d_ptr_implementation.h"
#include "Models/setting.h"
#include "selectdirectories.h"
#include "settings.h"
#include <Wt/WGroupBox>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WPushButton>
#include <Wt/WLabel>
#include <Wt/WToolBar>
#include "Wt-Commons/wt_helpers.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;
PandoricaWizard::Private::Private(PandoricaWizard* q) : q(q), stack(new WStackedWidget)
{
}

PandoricaWizard::Private::~Private()
{
}


void PandoricaWizard::Private::addPandoricaModePage()
{
  PandoricaMode currentMode = static_cast<PandoricaMode>(Setting::value<int>(Setting::PandoricaMode, static_cast<int>(PandoricaMode::Unset)));
  auto pandoricaMode = new WGroupBox("Pandorica Mode");
  WButtonGroup *buttonGroup = new WButtonGroup(pandoricaMode);
  WRadioButton *simpleMode, *advancedMode;
  buttonGroup->addButton(simpleMode = WW<WRadioButton>("simple mode").setInline(false));
  buttonGroup->addButton(advancedMode = WW<WRadioButton>("advanced mode").setInline(false));
  simpleMode->setChecked(currentMode == Simple);
  advancedMode->setChecked(currentMode == Advanced);
  pandoricaMode->addWidget(simpleMode);
  WLabel *simpleModeLabel = WW<WLabel>("Sigle user mode, automatically share every content to anyone on the same network without permissions or logging in. This is suggested for regular home usage.").setInline(false);
  simpleModeLabel->setBuddy(simpleMode);
  pandoricaMode->addWidget(simpleModeLabel);
  WLabel *advancedModeLabel = WW<WLabel>("Multi user mode, requires login and permissions for viewing content. Highly reccomended for public network servers.").setInline(false);
  pandoricaMode->addWidget(advancedMode);
  advancedModeLabel->setBuddy(advancedMode);
  pandoricaMode->addWidget(advancedModeLabel);
  buttonGroup->checkedChanged().connect([=](WRadioButton* b,_n5){
    PandoricaMode newMode = b==simpleMode ? Simple : Advanced;
    next->setEnabled(true);
    Setting::write(Setting::PandoricaMode, static_cast<int>(newMode));
  });
  stack->addWidget(pandoricaMode);
  showPage[PandoricaModePage] = [=] {
    stack->setCurrentWidget(pandoricaMode);
    next->setEnabled(static_cast<PandoricaMode>(Setting::value<int>(Setting::PandoricaMode, static_cast<int>(PandoricaMode::Unset))) != Unset);
    previous->setDisabled(true);
    finish->setDisabled(true);
    nextPage = FileSystemChooserPage;
  };
}


void PandoricaWizard::Private::addFileSystemChooser()
{
  auto fileSystemChooser = new WGroupBox("Multimedia Library");
  auto selectDirectories = new SelectDirectories({"/"},
						 Settings::mediasDirectories(), [=](const string &p){Settings::addMediaDirectory(p);},
						 [=](const string &p){Settings::removeMediaDirectory(p);},
						 SelectDirectories::Multiple );
  selectDirectories->setHeight(500);
  selectDirectories->addTo(fileSystemChooser);
  stack->addWidget(fileSystemChooser);
  showPage[FileSystemChooserPage] = [=] {
    stack->setCurrentWidget(fileSystemChooser);
    next->setEnabled(static_cast<PandoricaMode>(Setting::value<int>(Setting::PandoricaMode, static_cast<int>(PandoricaMode::Unset))) != Simple);
    previous->setEnabled(true);
    finish->setEnabled(static_cast<PandoricaMode>(Setting::value<int>(Setting::PandoricaMode, static_cast<int>(PandoricaMode::Unset))) == PandoricaMode::Simple);
    nextPage = FileSystemChooserPage;
    previousPage = PandoricaModePage;
  };
}


PandoricaWizard::~PandoricaWizard()
{
}

PandoricaWizard::PandoricaWizard(Wt::WContainerWidget* parent)
    : d(this)
{
  setImplementation(
    WW<WContainerWidget>().add(d->stack)
    .add(
      WW<WToolBar>().css("pull-right")
      .addButton(d->previous = WW<WPushButton>(WString::tr("button.previous")).css("btn-warning").setEnabled(false).onClick([=](WMouseEvent){ d->showPage[d->previousPage](); }))
      .addButton(d->next = WW<WPushButton>(WString::tr("button.next")).css("btn-primary").setEnabled(false).onClick([=](WMouseEvent){ d->showPage[d->nextPage](); }))
      .addButton(d->finish = WW<WPushButton>(WString::tr("button.finish")).css("btn-success").setEnabled(false).onClick([=](WMouseEvent){ delete this; }))
    )
  );
  d->addPandoricaModePage();
  d->addFileSystemChooser();
  d->showPage[Private::PandoricaModePage]();
}
