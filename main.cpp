#include <iostream>

#include "streamingapp.h"
#include <Wt/WServer>


Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
    return new StreamingApp(env);
}


int main(int argc, char **argv) {
  return Wt::WRun(argc, argv, createApplication);
}
