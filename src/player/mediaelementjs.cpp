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
#include "purehtml5js.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils.h"
#include <private/html5player_p.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace Wt;
using namespace std;

MediaElementJs::~MediaElementJs()
{
}
MediaElementJs::MediaElementJs(PandoricaPrivate::HTML5PlayerPrivate*const d, WObject* parent)
  : PlayerJavascript(d, parent), pureHTML5Js(new PureHTML5Js(d, this))
{
}

string MediaElementJs::resizeJs()
{
  return "$(playerId).mediaelementplayer().resize();";
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

  string mediaElementOptionsString = boost::algorithm::join(
    Utils::transform(mediaElementOptions, vector<string>{}, [](pair<string,string> o){
    return (boost::format("%s: %s") % o.first % o.second).str();
  }), ", ");

  log("notice") << "player options: " << mediaElementOptionsString;
  // Adding workaround for mediaelementjs bug https://github.com/johndyer/mediaelement/issues/902
  runJavascript((
    boost::format("$('video,audio').mediaelementplayer({%s});\
    var captionTracks = $('video,audio')[0].textTracks; \
      for(var i=0; i<captionTracks.length; i++) \
        if(captionTracks[i].kind == 'subtitles') captionTracks[i].mode='hidden'; \
    ")
    % mediaElementOptionsString
  ).str() );
  pureHTML5Js->onPlayerReady();
}

string MediaElementJs::customPlayerHTML()
{
  return {};
}
