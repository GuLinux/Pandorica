/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2013  Marco Gulino <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "videojs.h"
#include <Wt/WText>
#include <Wt/WApplication>
#include "wt_helpers.h"

using namespace Wt;
using namespace std;
using namespace boost;

VideoJS::VideoJS()
  : m_widget(new WText("", Wt::TextFormat::XHTMLUnsafeText)), m_ended(m_widget, "video_js_ended"), m_playing(false), has_subtitles(false)
{
  m_ended.connect([this](_n6){
    m_playing = false;
      WString js("try {\
  var myPlayer = _V_(\"gum_video_{1}\");\
    myPlayer.cancelFullScreen();\
} catch(err) {}");
  js.arg(m_widget->id());
  wApp->doJavaScript(js.toUTF8());
  });
}

VideoJS::~VideoJS()
{
    WString js("try {\
  var myPlayer = _V_(\"gum_video_{1}\");\
    myPlayer.src("");\
    myPlayer.erase();\
} catch(err) {}");
  js.arg(m_widget->id());
  wApp->doJavaScript(js.toUTF8());
  delete m_widget;
}

void VideoJS::setSource(Wt::WMediaPlayer::Encoding encoding, const Wt::WLink& path, bool autoPlay)
{
  m_text = WString("<video id=\"gum_video_{1}\" class=\"video-js vjs-default-skin\" controls {2} width=\"640\" height=\"264\" poster=\"http://video-js.zencoder.com/oceans-clip.jpg\" preload=\"auto\" data-setup=\"{}\">\
  <source type=\"video/{3}\" src=\"{4}\">{5}\
  </video>\
  <a onclick=\"_V_('gum_video_{1}').size(640,360);\" style=\"cursor: hand; cursor: pointer;\">Small</a>\
  <a onclick=\"_V_('gum_video_{1}').size(800,450);\" style=\"cursor: hand; cursor: pointer;\">Medium</a>\
  <a onclick=\"_V_('gum_video_{1}').size(1200,675);\" style=\"cursor: hand; cursor: pointer;\">Large</a>\
");
  string type;
  if(encoding == WMediaPlayer::WEBMV)
    type = "webm";
  if(encoding == WMediaPlayer::OGV)
    type = "ogg";
  if(encoding == WMediaPlayer::M4V)
    type = "mp4";
  m_text.arg(m_widget->id()).arg(autoPlay ? "autoplay" : "").arg(type).arg(path.url());
  wApp->log("notice") << "text: " << m_text;
  wApp->log("notice") << "textformat: " << m_widget->textFormat();
  if(autoPlay)
    this->m_playing = true;
}

void VideoJS::addSubtitles(const WLink& path, string name, string lang)
{
  WString subs("<track kind=\"subtitles\" src=\"{1}\" srclang=\"{2}\" label=\"{3}\" {4} > {5}");
  subs.arg(path.url()).arg(lang).arg(name).arg("");
//   subs.arg(has_subtitles ? "" : "default");
  m_text.arg(subs);
  has_subtitles = true;
}


Wt::JSignal< Wt::NoClass >& VideoJS::ended()
{
  return m_ended;
}

void VideoJS::play()
{
  m_widget->setText(m_text.arg(""));
  WString js = "var myPlayer = _V_(\"gum_video_{1}\");\
  myPlayer.play();";
  wApp->doJavaScript(js.arg(m_widget->id()).arg(m_ended.createCall()).toUTF8());
  m_playing = true;
}

bool VideoJS::playing()
{
  return m_playing;
}

void VideoJS::stop()
{
  WString js("try {\
  var myPlayer = _V_(\"gum_video_{1}\");\
    myPlayer.pause();\
} catch(err) {}");
  js.arg(m_widget->id());
  wApp->doJavaScript(js.toUTF8());
  m_playing = false;
}

Wt::WWidget* VideoJS::widget()
{
  m_widget->setText(m_text.arg(""));
  WString js = "var myPlayer = _V_(\"gum_video_{1}\");\
  var playerEnded = function() { \
    console.log(\"player ended\");\
    {2};\
  };\
  myPlayer.addEvent(\"ended\", playerEnded);";
//   m_widget->doJavaScript(js.toUTF8());
  return m_widget;
}
