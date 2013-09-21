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

#include "player/purehtml5js.h"
#include <player/private/html5player_p.h>
#include <boost/format.hpp>
#include "Wt-Commons/wt_helpers.h"

using namespace Wt;
using namespace std;


PureHTML5Js::PureHTML5Js(HTML5PlayerSetup html5PlayerSetup, Wt::WObject* parent)
  : PlayerJavascript(html5PlayerSetup, parent)
{
  html5PlayerSetup.bindEmpty("media.footer");
  html5PlayerSetup.bindEmpty("media.header");
}

string PureHTML5Js::customPlayerHTML()
{
  return {};
}

string PureHTML5Js::resizeJs()
{
  return {};
}

void PureHTML5Js::onPlayerReady()
{
  runJavascript((
    boost::format(JS(
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
    % MINIMUM_DESKTOP_SIZE
    % html5PlayerSetup.templateWidgetJSRef()
  ).str() );
}


