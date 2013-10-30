#include "MediaScanner/mediascannerstep.h"
#include <utils/utils.h>
#include <set>
#include <boost/thread.hpp>
using namespace std;

class MediaScannerSemaphore::Private {
public:
    Private(function<void()> runOnFree, function<void()> runOnBusy, MediaScannerSemaphore *q);
    function<void()> runOnFree;
    function<void()> runOnBusy;
    shared_ptr<MediaScannerSemaphore> parent;
    void addLock(MediaScannerSemaphore *child);
    void tryUnlock(MediaScannerSemaphore *child);
    void addNeedsSaving(MediaScannerSemaphore *child);
    void removeNeedsSaving(MediaScannerSemaphore *child);
    bool needsSaving(MediaScannerSemaphore *child);
    set<MediaScannerSemaphore*> locks;
    set<MediaScannerSemaphore*> needingSaving;
    boost::mutex mutex;
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
  boost::unique_lock<boost::mutex> lock(mutex);
  if(locks.empty())
    runOnBusy();
  locks.insert(child);
}

void MediaScannerSemaphore::Private::tryUnlock(MediaScannerSemaphore* child)
{
  boost::unique_lock<boost::mutex> lock(mutex);
  if(locks.empty())
    return;
  for(auto it = begin(locks); it != end(locks); ++it)
    if(child == *it) locks.erase(it);
  if(locks.empty())
    runOnFree();
}

void MediaScannerSemaphore::needsSaving(bool saving)
{
  if(! d->parent && saving)
    throw runtime_error("Cannot call needsSaving(true) on a parent MediaScannerSemaphore instance");
  if(! d->parent) {
    boost::unique_lock<boost::mutex> lock(d->mutex);
    d->needingSaving.clear();
    return;
  }
  if(saving)
    d->parent->d->addNeedsSaving(this);
  else
    d->parent->d->removeNeedsSaving(this);
}

bool MediaScannerSemaphore::needsSaving()
{
  if(! d->parent)
    return !d->needingSaving.empty();
  return d->parent->d->needsSaving(this);
}

void MediaScannerSemaphore::Private::addNeedsSaving(MediaScannerSemaphore* child)
{
  boost::unique_lock<boost::mutex> lock(mutex);
  needingSaving.insert(child);
}
bool MediaScannerSemaphore::Private::needsSaving(MediaScannerSemaphore* child)
{
  boost::unique_lock<boost::mutex> lock(mutex);
  return needingSaving.find(child) != needingSaving.end();
}
void MediaScannerSemaphore::Private::removeNeedsSaving(MediaScannerSemaphore* child)
{
  boost::unique_lock<boost::mutex> lock(mutex);
  for(auto it = begin(needingSaving); it != end(needingSaving); ++it)
    if(child == *it) needingSaving.erase(it);
}

MediaScannerStep::MediaScannerStep(const std::shared_ptr< MediaScannerSemaphore >& semaphore)
  : semaphore(*semaphore)
{
}

void MediaScannerStep::saveIfNeeded(Wt::Dbo::Transaction& transaction)
{
  Scope scope([=]{ semaphore.needsSaving(false); });
  if(!semaphore.needsSaving() ) {
    return;
  }
  save(transaction);
}

bool MediaScannerStep::needsSaving()
{
  return semaphore.needsSaving();
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
