#ifndef MEDIASCANNERSTEP_H
#define MEDIASCANNERSTEP_H
#include <Wt/WSignal>

class Media;
class FFMPEGMedia;
namespace Wt {
namespace Dbo {

class Transaction;
}

class WContainerWidget;
class WApplication;
}

#define guiRun(app, f) WServer::instance()->post(app->sessionId(), f)

class MediaScannerStep
{
public:
  enum StepResult { Waiting, Done, Skip, Redo };
  enum ExistingFlags { SkipIfExisting, OverwriteIfExisting};
  virtual void run(FFMPEGMedia *ffmpegMedia, Media *media, Wt::WContainerWidget *container, Wt::Dbo::Transaction *transaction, ExistingFlags onExisting = SkipIfExisting) = 0;
  virtual StepResult result() = 0;
  virtual void save(Wt::Dbo::Transaction *transaction) = 0;
};

#endif // MEDIASCANNERSTEP_H
