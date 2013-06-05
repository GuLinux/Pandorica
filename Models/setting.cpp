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

#include "setting.h"

using namespace std;

Setting::Setting()
{

}

Setting::~Setting()
{

}

string Setting::deployType()
{
  return "deploy_type";
}

string Setting::deployPath(const string& path)
{
  return string{"deploy_path_"} + path;
}

string Setting::secureDownloadPassword()
{
  return "securedownload_password";
}

string Setting::cacheDeployPath()
{
  return "cache_deploy_path";
}

string Setting::cacheDirectory()
{
  return "cache_directory";
}
