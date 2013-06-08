#include <iostream>

#include "streamingapp.h"
#include "session.h"
#include "Models/models.h"
#include "Wt-Commons/compositeresource.h"
#include "settings.h"


#include <Wt/WServer>
#include <Wt/WFileResource>
#include <boost/thread.hpp>
#include <thread>
#include <chrono>
#include <Magick++.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>


#ifdef HAVE_QT
#include <QtGui/QApplication>
#include "qttrayicon.h"
#endif

using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = filesystem;
using namespace WtCommons;
namespace po = program_options;

WApplication *createApplication(const WEnvironment& env)
{
    return new StreamingApp(env);
}

extern "C" {
  #include <libavcodec/avcodec.h>    // required headers
  #include <libavformat/avformat.h>
}

void expireStaleSessions() {
  try {
    Session session;
    Dbo::Transaction t(session);
    auto oldSessions = session.find<SessionInfo>().where("session_ended = 0").resultList();
    for(auto oldSession: oldSessions)
      oldSession.modify()->end();
    t.commit();
  } catch(std::exception &e) {
    cerr << "Database error: " << e.what() << endl;
  }
}

map<string,string> extensionsMimetypes {
  { ".css", "text/css" },
  { ".less", "text/css" },
  { ".png", "image/png" },
  { ".jpg", "image/jpeg" },
  { ".jpeg", "image/jpeg" },
  { ".gif", "image/gif" },
  { ".js", "application/javascript" },
  { ".svg", "image/svg+xml" },
  { ".swf", "application/x-shockwave-flash" },
  { ".xap", "application/x-silverlight-app" },
};


struct CmdOptions {
  int argc;
  char **argv;
  static CmdOptions from(vector<string> strings) {
    char **argv_temp = new char*[strings.size()]; int current=0;
    for(string argument: strings) {
      char *pc = new char[argument.size()+1];
      strcpy(pc, argument.c_str());
      argv_temp[current++] = pc;
    }
    return { lexical_cast<int>(strings.size()), argv_temp };
  }
};

template<class T> T optionValue(po::variables_map &options, string optionName) {
  return any_cast<T>(options[optionName].value());
}

bool checkForWrongOptions(bool errorCondition, string errorMessage) {
  if(errorCondition)
    cerr << "Error!\t" << errorMessage << "\n\n";
  return errorCondition;
}

bool initServer(int argc, char **argv, WServer &server, po::variables_map &vm) {
  string configDirectory = string{getenv("HOME")} + "/.config/Pandorica";
  try {
    filesystem::create_directories(configDirectory);
  } catch(std::exception &e) {
  }
  po::options_description pandorica_visible_options("Pandorica Options");
  po::options_description pandorica_general_options("General");
  pandorica_general_options.add_options()
  ("http-address", po::value<string>()->default_value("0.0.0.0"), "http address for listening")
  ("http-port", po::value<int>()->default_value(8080), "http port for listening")
  ("server-mode", po::value<string>()->default_value("standalone"), "Server run mode.\nAllowed modes are:\n\
  \t* standalone: Pandorica will run without an external http server.\n\
  \t* managed:    Pandorica will run inside an http server (apache, lighttp, etc). This means you have to take care of shared resource files.")
#ifdef HAVE_QT
  ("disable-tray", "Disable system tray icon")
#endif
  ("help", "shows Pandorica options")
  ("help-full", "shows full options list, including Wt.")
  ;
  
  po::options_description pandorica_db_options("Database Options");
  pandorica_db_options.add_options()
  ("sqlite3-database-path", po::value<string>()->default_value(configDirectory + "/Pandorica.sqlite"), "sqlite3 database path.")
  ("dump-schema", po::value<string>(), "dumps the schema to a file (argument) and exits, useful to manually execute migrations.")
  ;
  po::options_description pandorica_managed_options("Managed Mode Options");
  pandorica_managed_options.add_options()
  ("static-deploy-path", po::value<string>(), "Path in your web server pointing to Pandorica static files directory (" SHARED_FILES_DIR "/static)")
  ;
  
  pandorica_visible_options.add(pandorica_general_options).add(pandorica_db_options).add(pandorica_managed_options);
  po::options_description pandorica_invisible_options;
  pandorica_invisible_options.add_options()
  ("docroot", po::value<string>()->default_value("/usr/share/Wt"));
  
  po::options_description pandorica_all_options;
  pandorica_all_options.add(pandorica_visible_options).add(pandorica_invisible_options);
  
  auto po_parsed = po::command_line_parser(argc, argv).options(pandorica_all_options).allow_unregistered().run();
  po::store(po_parsed, vm);
  vector<string> wt_options = po::collect_unrecognized(po_parsed.options, program_options::include_positional); 
  wt_options.insert(wt_options.begin(), argv[0]);
  bool optionsAreWrong = false;
  
  optionsAreWrong |= checkForWrongOptions(optionValue<string>(vm, "server-mode") != "managed" && optionValue<string>(vm, "server-mode") != "standalone", "server-mode must be one of 'managed' or 'standalone'");
  optionsAreWrong |= checkForWrongOptions(optionValue<string>(vm, "server-mode") == "managed" && !vm.count("static-deploy-path"), "You must set static-deploy-path in managed server mode." );
  if(vm.count("help") || optionsAreWrong) {
    std::cout << pandorica_visible_options;
    return false;
  }
  if(vm.count("help-full")) {
    std::cout << pandorica_visible_options << "\n\t\t***** Wt Options *****\n";
    wt_options.push_back("--help");
  }
  
  if(!vm.count("https-address")) {
    wt_options.push_back("--http-address");
    wt_options.push_back(optionValue<string>(vm, "http-address"));
  }
  
  
  wt_options.push_back("--docroot");
  wt_options.push_back(optionValue<string>(vm, "docroot"));
  
  wt_options.push_back("--http-port");
  wt_options.push_back(lexical_cast<string>(optionValue<int>(vm, "http-port")) );
  std::cerr << "wt options: ";
  for(string option: wt_options)
    std::cerr << " " << option;
  std::cerr << '\n';
  
  CmdOptions options = CmdOptions::from(wt_options);
  
  server.setServerConfiguration(options.argc, options.argv, WTHTTP_CONFIGURATION);
  return true;
}

bool addStaticResources(WServer &server) {
  CompositeResource *staticResources = new CompositeResource();
  string staticDirectory{SHARED_FILES_DIR "/static"};
  if(!filesystem::is_directory(staticDirectory)) {
    std::cerr << "Error! Shared files directory " << staticDirectory << " doesn't exist. Perhaps you didn't install Pandorica correctly?\n";
    return false;
  }
  fs::recursive_directory_iterator it(staticDirectory, fs::symlink_option::recurse);
  while(it != fs::recursive_directory_iterator()) {
    if(fs::is_regular(*it)) {
      string filePath{it->path().string()};
      string fileUrl{filePath};
      replace_first(fileUrl, staticDirectory, "");
      auto resource = new WFileResource{filePath, staticResources};
      if(!extensionsMimetypes[it->path().extension().string()].empty())
        resource->setMimeType(extensionsMimetypes[it->path().extension().string()]);
      staticResources->add(fileUrl, resource);
    }
    it++;
  }
  server.addResource(staticResources, Settings::staticDeployPath());
  return true;
}

int main(int argc, char **argv)
{
  try {
    av_register_all();
    Magick::InitializeMagick(*argv);
    WServer server(argv[0]);
    po::variables_map vm;
    if(!initServer(argc, argv, server, vm)) return 1;
    Settings::init(vm);
    server.addEntryPoint(Application, createApplication);
    
    
    if(optionValue<string>(vm, "server-mode") == "standalone" && ! addStaticResources(server)) {
      return 1;
    }
    
    
    if(vm.count("dump-schema")) {
      Session session(false);
      ofstream schema;
      string schemaPath{any_cast<string>(vm["dump-schema"].value())};
      schema.open( schemaPath );
      schema << session.tableCreationSql();
      schema.close();
      std::cout << "Schema correctly wrote to " << schemaPath << '\n';
      return 0;
  }

    Session::configureAuth();
    expireStaleSessions();
    
#ifdef HAVE_QT
    if(! vm.count("disable-tray")) {
      QApplication app(argc, argv);
      QtTrayIcon icon(server);
      app.exec();
      return 0;
    }
#endif

    boost::thread t([&server]{
      std::this_thread::sleep_for(std::chrono::milliseconds{30000});
      while(server.isRunning()) {
        server.expireSessions();
        std::this_thread::sleep_for(std::chrono::milliseconds{30000});
      }
    });

    if (server.start()) {
      WServer::waitForShutdown();
      server.stop();
    }
  } catch (WServer::Exception& e) {
    cerr << e.what() << endl;
  } catch (std::exception &e) {
    cerr << "exception: " << e.what() << endl;
  }
}