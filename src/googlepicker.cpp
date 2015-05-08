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

#include "googlepicker.h"
#include "Models/setting.h"
#include <Wt/WApplication>
#include <Wt/WTemplate>

#include <boost/format.hpp>
#include <Wt-Commons/wt_helpers.h>
#include <Wt/WPushButton>
using namespace std;
using namespace Wt;
using namespace WtCommons;

GooglePicker::~GooglePicker()
{

}

GooglePicker::GooglePicker(const WString &buttonText, WContainerWidget* parent) : WCompositeWidget(parent), _imageChosen(this, "picker_image_chosen")
{
  _developerKey = Setting::value<string>(Setting::GoogleBrowserDeveloperKey, "AIzaSyA1ytuIbBRAsAS8f_4LDapsNUVsCZQmWG8");
  if(_developerKey.empty() ) {
    setImplementation(new WContainerWidget);
    return;
  }
  
  setImplementation(_button = WW<WPushButton>(buttonText).onClick([=](WMouseEvent){
    pick();
  }));
  wApp->declareJavaScriptFunction("onPickerApiLoad", R"(
    function() {
      console.log("picker api loaded");
      pickerApiLoaded = true;
    }
  )");
  wApp->declareJavaScriptFunction("apiLoad", R"(
    function() {
        console.log("onApiLoad() ");
	gapi.load('picker', {'callback': Wt.onPickerApiLoad});
    };
  )");
  wApp->log("notice") << "jsclass=" << wApp->javaScriptClass();
  wApp->doJavaScript("$.getScript('https://apis.google.com/js/api.js', Wt.apiLoad);");
  _button->setJavaScriptMember("pickerResult", (boost::format(R"(
    function(data) {
	console.log(data);
        if (data[google.picker.Response.ACTION] == google.picker.Action.PICKED) {
          var doc = data[google.picker.Response.DOCUMENTS][0];
          url = doc[google.picker.Document.THUMBNAILS][1].url;
	  %s;
	  console.log("loaded url: " + url );
        }
    }
  )") % _imageChosen.createCall("url")).str() );
}

JSignal< WString >& GooglePicker::imageChosen()
{
  return _imageChosen;
}

void GooglePicker::pick()
{
  
  wApp->doJavaScript((
    boost::format(R"(
	  console.log("creating picker");
          var imageView = new google.picker.ImageSearchView();
	  imageView.setQuery(%s);
	  imageView.setLicense(google.picker.ImageSearchView.License.NONE);
	  imageView.setSize(google.picker.ImageSearchView.Size.SIZE_XGA); // TODO: parameters?
          var picker = new google.picker.PickerBuilder().
              addView(imageView).
              setDeveloperKey(%s).
              setCallback(%s).
              build();
          picker.setVisible(true);
          console.log("Created picker: ");
          console.log(picker);
    )")
      % _searchString.jsStringLiteral()
      % WString(_developerKey).jsStringLiteral()
      % _button->javaScriptMember("pickerResult")
    ).str());
}
