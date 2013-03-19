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


#include "readbwstats.h"
#include <Wt/WTimer>
#include <Wt/WText>
#include <Wt/Http/Client>
#include <Wt/Http/Message>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "Wt-Commons/wt_helpers.h"


using namespace Wt;
using namespace std;

#define MAX_BW 1280

void ReadBWStats::startPolling()
{
  WTimer *timer = new WTimer(this);
  int interval = 5;
  timer->setInterval(interval * 1000);
  Http::Client *client = new Http::Client(this);
  
  client->done().connect([this,interval](boost::system::error_code err, Http::Message message, _n4){
    if(err || message.status() != 200) {
      widget->setText("N/A");
      return;
    }
    string stats = message.body();
    list<string> statsLines;
    boost::split(statsLines, stats, [](char c) { return c=='\n';});
    for(string s: statsLines) {
      if(s.find("Total kBytes") != string::npos) {
	vector<string> s_splitted;
	boost::split(s_splitted, s, [](char c) { return c==':';});
	if(s_splitted.size()<2) return;
	string kb = s_splitted[1];
	boost::trim(kb);
	long kbNow = atol(kb.c_str());
	double kbSec = double(kbNow-prevKB)/interval;
	if(!prevKB) {
	  prevKB = kbNow;
	  return;
	}
	prevKB = kbNow;
	double percent = kbSec * 100 / MAX_BW;
	string formattedPercent = (boost::format("Bandwidth Load: %.2f%%") % percent).str();
	widget->setText(formattedPercent);
      }
    }
  });
  
  timer->timeout().connect([this,client](WMouseEvent){
    client->get(url);
  });
  timer->start();
}
