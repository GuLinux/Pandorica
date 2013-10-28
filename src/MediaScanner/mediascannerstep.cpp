#include "MediaScanner/mediascannerstep.h"
#include <set>
using namespace std;

MediaScannerStep::StepResult MediaScannerStep::result()
{
  return _result;
}

void MediaScannerStep::setResult( MediaScannerStep::StepResult result )
{
  _result = result;
}

class MediaScannerSemaphore::Private {
public:
    Private(function<void()> runOnFree, function<void()> runOnBusy, MediaScannerSemaphore *q);
    function<void()> runOnFree;
    function<void()> runOnBusy;
    shared_ptr<MediaScannerSemaphore> parent;
    void addLock(MediaScannerSemaphore *child);
    void tryUnlock(MediaScannerSemaphore *child);
    set<MediaScannerSemaphore*> locks;
private:
    MediaScannerSemaphore *q;
};



MediaScannerSemaphore::Private::Private(function<void()> runOnFree, function<void()> runOnBusy, MediaScannerSemaphore* q)
  : runOnFree(runOnFree), runOnBusy(runOnBusy), q(q)
{
}

MediaScannerSemaphore::~MediaScannerSemaphore()
{
}

MediaScannerSemaphore::MediaScannerSemaphore(function<void()> runOnFree, function<void()> runOnBusy)
  : d(new Private(runOnFree, runOnBusy, this))
{
}

MediaScannerSemaphore::MediaScannerSemaphore(MediaScannerSemaphore& parent)
  : d(new Private([]{}, []{}, this))
{
  d->parent = parent.shared_from_this();
}

MediaScannerSemaphore& MediaScannerSemaphore::operator=(MediaScannerSemaphore& parent)
{
  d->parent = parent.shared_from_this();
}



void MediaScannerSemaphore::Private::addLock(MediaScannerSemaphore* child)
{
  if(locks.empty())
    runOnBusy();
  locks.insert(child);
}

void MediaScannerSemaphore::Private::tryUnlock(MediaScannerSemaphore* child)
{
  if(locks.empty())
    return;
  for(auto it = begin(locks); it != end(locks); ++it)
    if(child == *it) locks.erase(it);
  if(locks.empty())
    runOnFree();
}


void MediaScannerSemaphore::lock()
{
  if(! d->parent)
    throw runtime_error("Cannot lock a parent MediaScannerSemaphore instance");
  d->parent->d->addLock(this);
}

void MediaScannerSemaphore::unlock()
{
  if(! d->parent)
    throw runtime_error("Cannot unlock a parent MediaScannerSemaphore instance");
  d->parent->d->tryUnlock(this);
}
