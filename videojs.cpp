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

using namespace Wt;
using namespace std;
using namespace boost;

VideoJS::VideoJS()
  : m_widget(new WText("", Wt::TextFormat::XHTMLUnsafeText)), m_ended(m_widget, "video_js_ended"), m_playing(false), has_subtitles(false)
{
  m_ended.connect([this](NoClass,NoClass,NoClass,NoClass,NoClass,NoClass){m_playing = false;});
}

VideoJS::~VideoJS()
{
  delete m_widget;
}

void VideoJS::addSource(Wt::WMediaPlayer::Encoding encoding, const Wt::WLink& path)
{
  m_text = WString("<video id=\"gum_video_{1}\" class=\"video-js vjs-default-skin\" controls width=\"640\" height=\"264\" poster=\"http://video-js.zencoder.com/oceans-clip.jpg\" preload=\"auto\">\
  <source type=\"video/{2}\" src=\"{3}\">{4}\
</video>");
  string type;
  if(encoding == WMediaPlayer::WEBMV)
    type = "webm";
  if(encoding == WMediaPlayer::OGV)
    type = "ogg";
  if(encoding == WMediaPlayer::M4V)
    type = "mp4";
  m_text.arg(m_widget->id()).arg(type).arg(path.url());
  wApp->log("debug") << "text: " << m_text;
  wApp->log("debug") << "textformat: " << m_widget->textFormat();
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
  var playerEnded = function() { \
    {1}\
  };\
  myPlayer.addEvent(\"ended\", playerEnded);\
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
  wApp->doJavaScript("try {myPlayer.pause(); } catch(err) {}");
  m_playing = false;
}

Wt::WWidget* VideoJS::widget()
{
  return m_widget;
}
