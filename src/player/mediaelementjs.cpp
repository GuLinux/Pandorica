/*
 * Copyright (c) year Marco Gulino <marco.gulino@gmail.com>
 *
 * This file is part of Pandorica: https://github.com/GuLinux/Pandorica
 *
 * Pandorica is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details (included the COPYING file).
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mediaelementjs.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils.h"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace Wt;
using namespace std;

MediaElementJs::~MediaElementJs()
{
}
MediaElementJs::MediaElementJs(WContainerWidget* parent): HTML5Player(parent)
{

}

void MediaElementJs::onPlayerReady()
{
  map<string,string> mediaElementOptions = {
    {"AndroidUseNativeControls", "false"}
  };
  // works in theory, but it goes with double subs on chrome
//   if(defaultTracks["subtitles"].isValid() && false) {
//       mediaElementOptions["startLanguage"] = (boost::format("'%s'") % defaultTracks["subtitles"].lang).str();
//   }


  string mediaElementOptionsString = boost::algorithm::join(Utils::transform(mediaElementOptions, vector<string>{}, [](pair<string,string> o){
    return (boost::format("%s: %s") % o.first % o.second).str();
  }), ", ");

  log("notice") << "player options: " << mediaElementOptionsString;
  runJavascript((
    boost::format(JS($('video,audio').mediaelementplayer({%s});
    var minimumDesktopSize = %d;
    function autoResizeVideoPlayer() {
      var playerWidget = %s;
      if(playerWidget == null)
        return;
      if($(window).width() >= minimumDesktopSize)
        playerWidget.videoResize(60);
      else
        playerWidget.videoResize(100);
    }
    window.currentWidth = $(window).width();
    autoResizeVideoPlayer();
    var resizeTimer = null;
    $(window).resize(function(){
      clearTimeout(resizeTimer);
      resizeTimer = setTimeout(function() {
        var newWidth = $(window).width();
        if( (window.currentWidth < minimumDesktopSize && newWidth > minimumDesktopSize) ||
          (window.currentWidth > minimumDesktopSize && newWidth < minimumDesktopSize) ) autoResizeVideoPlayer();
        window.currentWidth = newWidth;
      }, 500);
    });
    ))
    % mediaElementOptionsString
    % MINIMUM_DESKTOP_SIZE
    % widgetJSRef()
  ).str() );
}
