/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "utils/utils.h"
#include "utils/d_ptr_implementation.h"
#include "private/concatstrings_p.h"
#include <sstream>


using namespace std;


ConcatStrings::ConcatStrings( const list< string > &strings, const string &separator )
  : d(strings, separator)
{
}

ConcatStrings::~ConcatStrings()
{
}

ConcatStrings::operator string() const
{
  bool isFirst = true;
  stringstream strstream;
  for(auto s: d->strings) {
    if(isFirst) {
      strstream << d->separator;
      isFirst = false;
    }
    strstream << s;
  }
  return strstream.str();
}


