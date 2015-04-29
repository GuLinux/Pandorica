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
#include <Wt/WGroupBox>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WPushButton>
#include <Wt/WLabel>
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
  WGroupBox *groupBox = new WGroupBox("Pandorica Mode");
  WButtonGroup *buttonGroup = new WButtonGroup(groupBox);
  WRadioButton *simpleMode, *advancedMode;
  buttonGroup->addButton(simpleMode = WW<WRadioButton>("simple mode").setInline(false));
  buttonGroup->addButton(advancedMode = WW<WRadioButton>("advanced mode").setInline(false));
  simpleMode->setChecked(currentMode == Simple);
  advancedMode->setChecked(currentMode == Advanced);
  groupBox->addWidget(simpleMode);
  WLabel *simpleModeLabel = WW<WLabel>("Sigle user mode, automatically share every content to anyone on the same network without permissions or logging in. This is suggested for regular home usage.").setInline(false);
  simpleModeLabel->setBuddy(simpleMode);
  groupBox->addWidget(simpleModeLabel);
  WLabel *advancedModeLabel = WW<WLabel>("Multi user mode, requires login and permissions for viewing content. Highly reccomended for public network servers.").setInline(false);
  groupBox->addWidget(advancedMode);
  advancedModeLabel->setBuddy(advancedMode);
  groupBox->addWidget(advancedModeLabel);
  WPushButton *nextButton = WW<WPushButton>(WString::tr("button.next")).css("btn-primary").onClick([=](WMouseEvent){ stack->setCurrentIndex(stack->currentIndex()+1); }).setEnabled(currentMode == Advanced);
  WPushButton *finishButton = WW<WPushButton>(WString::tr("button.next")).css("btn-success").onClick([=](WMouseEvent){ delete q; }).setEnabled(currentMode == Simple);
  groupBox->addWidget(nextButton);
  buttonGroup->checkedChanged().connect([=](WRadioButton* b,_n5){
    PandoricaMode newMode = b==simpleMode ? Simple : Advanced;
    nextButton->setEnabled(newMode == Simple);
    finishButton->setEnabled(newMode == Advanced);
    Setting::write(Setting::PandoricaMode, static_cast<int>(newMode));
  });
  stack->addWidget(groupBox);
}


PandoricaWizard::~PandoricaWizard()
{
}

PandoricaWizard::PandoricaWizard(Wt::WContainerWidget* parent)
    : d(this)
{
  setImplementation(d->stack);
  d->addPandoricaModePage();
}
