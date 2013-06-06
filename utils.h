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

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <Wt/WString>
#include <Wt/WLength>

namespace Wt {
class WInteractWidget;
}
namespace StreamingPrivate {
  class UtilsPrivate;
}
class Utils
{
public:
    Utils();
    ~Utils();
    static std::string titleHintFromFilename(std::string filename);
    static void mailForUnauthorizedUser(std::string email, Wt::WString identity);
    static void mailForNewAdmin(std::string email, Wt::WString identity);
    static std::string formatFileSize(long size);
    static Wt::WInteractWidget *help(std::string titleKey, std::string contentKey, std::string side, Wt::WLength size = Wt::WLength::Auto);
    
private:
  StreamingPrivate::UtilsPrivate* const d;
};

#endif // UTILS_H
