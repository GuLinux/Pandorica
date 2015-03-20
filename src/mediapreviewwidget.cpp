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

#include "src/mediapreviewwidget.h"
#include <Wt/WContainerWidget>
#include <Wt/WVBoxLayout>
#include "googlepicker.h"
#include "media/media.h"
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WFileUpload>
#include "utils/image.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;

MediaPreviewWidget::~MediaPreviewWidget()
{

}

MediaPreviewWidget::MediaPreviewWidget(const Media& media, WContainerWidget* parent)
{
  WContainerWidget *content = WW<WContainerWidget>();
  content->setLayout(new WVBoxLayout);
  WFileUpload *uploadCover = new WFileUpload;
  content->layout()->addWidget(uploadCover);
  setImplementation(content);
}
