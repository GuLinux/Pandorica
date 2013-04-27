#include <iostream>

#include "streamingapp.h"
#include "session.h"
#include <Wt/WServer>


Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
    return new StreamingApp(env);
}

extern "C" {
  #include <libavcodec/avcodec.h>    // required headers
  #include <libavformat/avformat.h>
}


int main(int argc, char **argv)
{
  try {
    av_register_all();
    
    Wt::WServer server(argv[0]);
    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
    server.addEntryPoint(Wt::Application, createApplication);
    Session::configureAuth();

    if (server.start()) {
      Wt::WServer::waitForShutdown();
      server.stop();
    }
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}