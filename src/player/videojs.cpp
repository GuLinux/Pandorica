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

#include "videojs.h"
#include <private/html5player_p.h>
#include <Wt/WApplication>
#include "Wt-Commons/wt_helpers.h"
#include "player/player.h"
#include "purehtml5js.h"
#include <boost/format.hpp>


using namespace Wt;
using namespace std;
using namespace boost;


VideoJs::VideoJs(PandoricaPrivate::HTML5PlayerPrivate*const d, WObject* parent)
  : PlayerJavascript(d, parent), pureHTML5Js(new PureHTML5Js(d, this))
{
  d->templateWidget->bindEmpty("media.footer");
  d->templateWidget->bindEmpty("media.header");
}

VideoJs::~VideoJs()
{

}

string VideoJs::resizeJs()
{
  return (boost::format(JS(
    var newWidth = $('#%s').width();
    var videoJs = _V_('%s');
    var newHeight = newWidth * videoJs.height() / videoJs.width();
    videoJs.dimensions(newWidth, newHeight)
  ))
    % d->templateWidget->id()
    % d->playerId()
  ).str();
}

string VideoJs::customPlayerHTML()
{
  if(d->sources[0].type.find("video/") == string::npos) return pureHTML5Js->customPlayerHTML();
  return "class='video-js vjs-default-skin'";
}


void VideoJs::onPlayerReady()
{
  if(d->sources[0].type.find("video/") == string::npos) {
    pureHTML5Js->onPlayerReady();
    return;
  }
  runJavascript(
    (boost::format(JS(
      videojs("%s", {}, function(){
        // Player (this) is initialized and ready.
      });
    ))
      % d->playerId()
    ).str()
  );
  pureHTML5Js->onPlayerReady();
}
