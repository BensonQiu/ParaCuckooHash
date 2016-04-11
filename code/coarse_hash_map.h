#ifndef COARSE_HASHMAP_H
#define COARSE_HASHMAP_H

#include <atomic>
#include <iostream>
#include <mutex>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

template <typename T, typename U>
class CoarseHashMap {

	boost::shared_mutex* m_foo;

	public:
		CoarseHashMap(int table_size=64, float max_load_factor=1.5f);
		~CoarseHashMap();

		CoarseHashMap(const CoarseHashMap&) =delete;
	    CoarseHashMap& operator=(const CoarseHashMap&) =delete;

		U get(T key);
		void put(T key, U val);
		bool remove(T key);

		float get_load_factor();
		int get_size();


	private:
		struct HashEntry {
			T key;
			U val;
			HashEntry *next;
		};

		int m_table_size; // Number of buckets.
		std::atomic_int m_size; // Number of keys.
		float m_max_load_factor; // Threshold for resizing table.
		HashEntry **m_table;
		std::mutex *m_bucket_mutexes; // Coarse grained mutex on each bucket.
		// boost::shared_mutex *m_bucket_mutexes;
		// boost::shared_mutex *m_foo;

};

#include "coarse_hash_map.cpp"

#endif
