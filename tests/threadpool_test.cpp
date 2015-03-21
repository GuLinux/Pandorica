#include "boost_unit_tests_helper.h"
#include "threadpool.h"
#include <boost/thread.hpp>

using namespace std;

BOOST_AUTO_TEST_CASE(TestThreadPool) {
  boost::thread::id threadid_1, threadid_2;
  ThreadPool::instance()->post([&threadid_1] { threadid_1 = boost::this_thread::get_id(); });
  ThreadPool::instance()->post([&threadid_2] { threadid_2 = boost::this_thread::get_id(); });
  
  boost::this_thread::sleep_for(boost::chrono::seconds(2));
  BOOST_REQUIRE(threadid_1 != threadid_2);
  
}