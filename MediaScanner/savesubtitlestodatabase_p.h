/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef SAVESUBTITLESTODATABASEPRIVATE_H
#define SAVESUBTITLESTODATABASEPRIVATE_H

class Session;
namespace Wt {
  class WApplication;
};

class SaveSubtitlesToDatabasePrivate
{
public:
  SaveSubtitlesToDatabasePrivate(Session* session, Wt::WApplication* app, SaveSubtitlesToDatabase* q);
    virtual ~SaveSubtitlesToDatabasePrivate();
    Session* session;
    Wt::WApplication* app;

private:
    class SaveSubtitlesToDatabase* const q;
};

#endif // SAVESUBTITLESTODATABASEPRIVATE_H
