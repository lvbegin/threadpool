#ifndef MAP_H__
#define MAP_H__

#include <threadpool.h>
#include <algorithm>

using namespace threadpool;

template<typename M>
void map(std::vector<M> &v, std::function<void( M&)> f, ThreadCache &cache) {
	Threadpool<M> pool(doNothing, f, doNothing, 10, v.size(), cache);
	std::for_each(v.begin(), v.end(), [&pool](M &e) { pool.add(e); });
}

#endif
