AUTOMAKE_OPTIONS = subdir-objects

TESTS=threadpoolTest
check_PROGRAMS=threadpoolTest
threadpoolTest_SOURCES=test/threadpoolTest.cpp
threadpoolTest_HEADERS=include/map.h include/queue.h include/reduce.h include/threadCache.h include/threadpool.h
threadpoolTest_CPPFLAGS=-I$(top_srcdir)/include
threadpoolTestdir=$(top_srcdir)
AM_LD_FLAGS=-lpthread

