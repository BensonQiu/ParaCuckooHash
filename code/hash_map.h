#ifndef HASHMAP_H
#define HASHMAP_H


template <typename T, typename U>
class HashMap {

	public:
		HashMap(int table_size=64, float max_load_factor=1.5f);
		~HashMap();

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
		int m_size = 0; // Number of keys.
		float m_max_load_factor; // Threshold for resizing table.
		HashEntry **m_table;

		void resize();

};

#include "hash_map.cpp"

#endif
