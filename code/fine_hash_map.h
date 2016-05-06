#ifndef FINE_HASHMAP_H
#define FINE_HASHMAP_H

#include <atomic>
#include <iostream>
#include <mutex>

template <typename T>
class FineHashMap {

	public:
		FineHashMap(int table_size=64, float max_load_factor=1.5f);
		~FineHashMap();

		FineHashMap(const FineHashMap&) =delete;
	    FineHashMap& operator=(const FineHashMap&) =delete;

		T get(std::string key);
		void put(std::string key, T val);
		bool remove(std::string key);

		float get_load_factor();
		int get_size();


	private:
		struct HashEntry {
			std::string key;
			T val;
			HashEntry *next;
		};

		int m_table_size; // Number of buckets.
		std::atomic_int m_size; // Number of keys.
		float m_max_load_factor; // Threshold for resizing table.
		HashEntry **m_table;
		std::mutex *m_bucket_mutexes; // Fine grained mutex on each bucket.

};

#include "fine_hash_map.cpp"

#endif
