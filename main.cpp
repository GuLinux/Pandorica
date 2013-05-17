#include <iostream>

#include "streamingapp.h"
#include "session.h"
#include "Models/models.h"
#include "Wt-Commons/compositeresource.h"


#include <Wt/WServer>
#include <Wt/WFileResource>
#include <boost/thread.hpp>
#include <thread>
#include <chrono>
#include <Magick++.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace Wt;
using namespace std;
namespace fs = boost::filesystem;

WApplication *createApplication(const WEnvironment& env)
{
    return new StreamingApp(env);
}

extern "C" {
  #include <libavcodec/avcodec.h>    // required headers
  #include <libavformat/avformat.h>
}

void expireStaleSessions() {
  Session session;
  Dbo::Transaction t(session);
  auto oldSessions = session.find<SessionInfo>().where("session_ended = 0").resultList();
  for(auto oldSession: oldSessions)
    oldSession.modify()->end();
  t.commit();
}

map<string,string> extensionsMimetypes {
  { ".css", "text/css" },
  { ".png", "image/png" },
  { ".jpg", "image/jpeg" },
  { ".jpeg", "image/jpeg" },
  { ".gif", "image/gif" },
  { ".js", "application/javascript" },
  { ".svg", "image/svg+xml" },
  { ".swf", "application/x-shockwave-flash" },
  { ".xap", "application/x-silverlight-app" },
};

int main(int argc, char **argv)
{
  try {
    av_register_all();
    Magick::InitializeMagick(*argv);
    
    WServer server(argv[0]);
    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
    server.addEntryPoint(Application, createApplication);
    
    CompositeResource staticResources;
    if(true) {
      string staticDirectory = "../files/static";
      fs::recursive_directory_iterator it(staticDirectory, fs::symlink_option::recurse);
      while(it != fs::recursive_directory_iterator()) {
        if(fs::is_regular(*it)) {
          string filePath{it->path().string()};
          string fileUrl{filePath};
          boost::replace_first(fileUrl, staticDirectory, "");
          auto resource = new WFileResource{filePath, &staticResources};
          if(!extensionsMimetypes[it->path().extension().string()].empty())
            resource->setMimeType(extensionsMimetypes[it->path().extension().string()]);
          staticResources.add(fileUrl, resource);
        }
        it++;
      }
      server.addResource(&staticResources, "/static");
    }
    
    Session::configureAuth();
    expireStaleSessions();

    boost::thread t([&server]{
      this_thread::sleep_for(chrono::milliseconds{30000});
      while(server.isRunning()) {
        server.expireSessions();
        this_thread::sleep_for(chrono::milliseconds{30000});
      }
    });
    if (server.start()) {
      WServer::waitForShutdown();
      server.stop();
    }
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}