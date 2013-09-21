#ifndef BOOST_UNIT_TESTS_HELPER_H
#define BOOST_UNIT_TESTS_HELPER_H
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#ifdef IN_KDEVELOP_PARSER
#define BOOST_CHECK_WITH_ARGS_IMPL(...) void(__VA_ARGS__)
#endif
#endif