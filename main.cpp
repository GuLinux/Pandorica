#include <iostream>

#include "streamingapp.h"
#include "session.h"
#include "Models/models.h"

#include <Wt/WServer>
#include <boost/thread.hpp>
#include <thread>
#include <chrono>
#include <Magick++.h>

using namespace Wt;
using namespace std;

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

int main(int argc, char **argv)
{
  try {
    av_register_all();
    Magick::InitializeMagick(*argv);
    
    WServer server(argv[0]);
    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
    server.addEntryPoint(Application, createApplication);
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