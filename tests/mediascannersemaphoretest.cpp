#include "boost_unit_tests_helper.h"
#include "MediaScanner/mediascannerstep.h"
using namespace std;

BOOST_AUTO_TEST_CASE(CannotLockParentInstance)
{
  auto parentSemaphore = make_shared<MediaScannerSemaphore>([]{}, []{});
  BOOST_REQUIRE_THROW(parentSemaphore->lock(), runtime_error);
  BOOST_REQUIRE_THROW(parentSemaphore->unlock(), runtime_error);
}

BOOST_AUTO_TEST_CASE(CanLockAndUnlockACopy)
{
  auto parentSemaphore = make_shared<MediaScannerSemaphore>([]{}, []{});
  MediaScannerSemaphore child = *parentSemaphore;
  BOOST_REQUIRE_NO_THROW(child.lock());
  BOOST_REQUIRE_NO_THROW(child.unlock());
}


BOOST_AUTO_TEST_CASE(ItShouldRunFreeAndBusyMethods)
{
  bool freeRunned = false;
  bool busyRunned = false;
  auto parentSemaphore = make_shared<MediaScannerSemaphore>([&freeRunned]{ freeRunned = true; }, [&busyRunned]{ busyRunned = true; });
  MediaScannerSemaphore child = *parentSemaphore;
  child.lock();
  BOOST_REQUIRE( busyRunned );
  child.unlock();
  BOOST_REQUIRE( freeRunned );
}


BOOST_AUTO_TEST_CASE(ItShouldRunFreeAndBusyMethodsWithMoreChildren)
{
  int freeRunned = 0;
  int busyRunned = 0;
  auto parentSemaphore = make_shared<MediaScannerSemaphore>([&freeRunned]{ freeRunned++; }, [&busyRunned]{ busyRunned++; });
  MediaScannerSemaphore child1 = *parentSemaphore;
  MediaScannerSemaphore child2 = *parentSemaphore;
  child1.lock();
  child2.lock();
  BOOST_REQUIRE_EQUAL( 1, busyRunned );
  child1.unlock();
  BOOST_REQUIRE_EQUAL( 0, freeRunned );
  child2.unlock();
  BOOST_REQUIRE_EQUAL( 1, freeRunned );
}

BOOST_AUTO_TEST_CASE(ComplexScenario)
{
  int freeRunned = 0;
  int busyRunned = 0;
  auto parentSemaphore = make_shared<MediaScannerSemaphore>([&freeRunned]{ freeRunned++; }, [&busyRunned]{ busyRunned++; });
  MediaScannerSemaphore child1 = *parentSemaphore;
  MediaScannerSemaphore child2 = *parentSemaphore;
  MediaScannerSemaphore child3 = *parentSemaphore;
  child1.lock();
  child2.lock();
  BOOST_REQUIRE_EQUAL( 1, busyRunned );
  child1.unlock();
  BOOST_REQUIRE_EQUAL( 0, freeRunned );
  child2.unlock();
  BOOST_REQUIRE_EQUAL( 1, freeRunned );
  
  child3.lock();
  child3.lock();
  BOOST_REQUIRE_EQUAL( 2, busyRunned );
  child1.lock();
  BOOST_REQUIRE_EQUAL( 2, busyRunned );
  child2.lock();
  child2.lock();
  BOOST_REQUIRE_EQUAL( 2, busyRunned );
  child1.unlock();
  BOOST_REQUIRE_EQUAL( 1, freeRunned );
  child2.unlock();
  BOOST_REQUIRE_EQUAL( 1, freeRunned );
  child2.lock();
  BOOST_REQUIRE_EQUAL( 2, busyRunned );
  child3.unlock();
  BOOST_REQUIRE_EQUAL( 1, freeRunned );
  child1.unlock();
  child2.unlock();
  child1.unlock();
  BOOST_REQUIRE_EQUAL( 2, busyRunned );
  BOOST_REQUIRE_EQUAL( 2, freeRunned );
}