#ifndef CUCKOO_HASHMAP_H
#define CUCKOO_HASHMAP_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include <boost/functional/hash.hpp>

#include "common/hash.c"
#include "common/errors.h"


template <typename T>
class CuckooHashMap {

	public:
		const int SLOTS_PER_BUCKET = 4;
		const int MAX_ITERS = 25;

		CuckooHashMap(int num_buckets=64) {
			m_table = new HashPointer*[num_buckets * SLOTS_PER_BUCKET]();
			m_num_buckets = num_buckets;
		};

		~CuckooHashMap() {
			// TODO: Free everything.
		}

		void debug() {
			/*
			for (int i = 0; i < m_num_buckets; i++) {
				//std::cout << "Bucket " << i << ":";
				for (int j = i*4; j < i*4 + SLOTS_PER_BUCKET; j++) {
					HashPointer* hash_pointer = m_table[j];
					if (hash_pointer == NULL)
						//std::cout << " NULL -->";
					else {
						//std::cout << " (" << hash_pointer->ptr->key << "," << hash_pointer->ptr->val << ")";
					}
				}
				//std::cout << std::endl;
			}
			*/
		}

		T get(std::string key) {
		    // Hash to two buckets.
		    // uint32_t h1, h2;
		    // hashlittle2((void*)&key, key.length(), &h1, &h2);
			std::hash<std::string> hasher1;
			boost::hash<std::string> hasher2;

			int h1 = hasher1(key);
			int h2 = hasher2(key);
			unsigned char tag = 'a';

		    //std::cout << "Get Key: " << key << ", Hash 1: " << h1 << ", Hash 2: " << h2 << std::endl;

		    h1 = h1 % m_num_buckets;
		    h2 = h2 % m_num_buckets;
		    // unsigned char tag = tag_hash(key);

		    if (h1 < 0)
		    	h1 = m_num_buckets - abs(h1);

		    if (h2 < 0)
		    	h2 = m_num_buckets - abs(h2);

		   	h1 *= SLOTS_PER_BUCKET;
		   	h2 *= SLOTS_PER_BUCKET;

		    // Look at the first bucket.
		    for (int i = (int)h1; i < (int)h1 + SLOTS_PER_BUCKET; i++) {
		    	HashPointer* hash_pointer = m_table[i];
		    	if (hash_pointer != NULL && tag == hash_pointer->tag) {
		    		HashEntry* hash_entry = hash_pointer->ptr;
		    		if (key == hash_entry->key)
		    			return hash_entry->val;
		    	}
		    }

		    // Look at the second bucket.
			for (int i = (int)h2; i < (int)h2 + SLOTS_PER_BUCKET; i++) {
		    	HashPointer* hash_pointer = m_table[i];
		    	if (hash_pointer != NULL && tag == hash_pointer->tag) {
		    		HashEntry* hash_entry = hash_pointer->ptr;
		    		if (key == hash_entry->key)
		    			return hash_entry->val;
		    	}
		    }
			throw KeyNotFoundError(key.c_str());
		}

			// std::hash<T> hasher;
			// int h1 = hasher(key);
			// int h2 = hash_two(key.c_str());
			// unsigned char tag = 'a';

		void put(std::string key, T val) {			
			// Hash to two buckets.
		    int num_iters = 0;
			std::string curr_key = key;
			T curr_val = val;

			std::hash<std::string> hasher1;
			boost::hash<std::string> hasher2;

		    while (num_iters < MAX_ITERS) {
		    	debug();

		    	num_iters ++;

				int h1 = hasher1(curr_key);
				int h2 = hasher2(curr_key);

			    //std::cout << "Put Key: " << curr_key << ", Hash 1: " << h1 << ", Hash 2: " << h2 << std::endl;

			    h1 = h1 % m_num_buckets;
			    h2 = h2 % m_num_buckets;

			    if (h1 < 0)
			    	h1 = m_num_buckets - abs(h1);

			    if (h2 < 0)
			    	h2 = m_num_buckets - abs(h2);

			    h1 *= SLOTS_PER_BUCKET;
			    h2 *= SLOTS_PER_BUCKET;

			    //std::cout << "Put Key: " << curr_key << ", Bucket 1: " << h1 << ", Bucket 2: " << h2 << std::endl;

			    // unsigned char tag = tag_hash(curr_key);
			    unsigned char tag = 'a';

			    // Look at the first bucket.
			    for (int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {
	
			    	HashPointer* hash_pointer = m_table[i];

			    	if (hash_pointer == NULL) {

			    		HashEntry* hash_entry = new HashEntry();
			    		hash_entry->key = curr_key;
			    		hash_entry->val = curr_val;

			    		hash_pointer = new HashPointer();
			    		hash_pointer->tag = tag;
			    		hash_pointer->ptr = hash_entry;
			    		m_table[i] = hash_pointer;
			    		
			    		//std::cout <<"Put Key: " << curr_key << " in slot index: " << i << std::endl;
			    		
			    		return;
			    	}
			    }

			    // Look at the second bucket.
				for (int i = (int)h2; i < (int)h2 + SLOTS_PER_BUCKET; i++) {
			    	HashPointer* hash_pointer = m_table[i];
			    	if (hash_pointer == NULL) {
			    		HashEntry* hash_entry = new HashEntry();
			    		hash_entry->key = curr_key;
			    		hash_entry->val = curr_val;

			    		hash_pointer = new HashPointer();
			    		hash_pointer->tag = tag;
			    		hash_pointer->ptr = hash_entry;
			    		m_table[i] = hash_pointer;

			    		//std::cout <<"Put Key: " << curr_key << " in slot index: " << i << std::endl;

			    		return;
			    	}
			    }

				int index = rand() % (2 * SLOTS_PER_BUCKET);
				HashPointer* temp_hash_pointer;
				if (0 <= index && index < SLOTS_PER_BUCKET)
					temp_hash_pointer = m_table[h1 + index];
				else
					temp_hash_pointer = m_table[h2 + index - SLOTS_PER_BUCKET];

				std::string temp_key = temp_hash_pointer->ptr->key;
				T temp_val = temp_hash_pointer->ptr->val;

				temp_hash_pointer->tag = tag;
				temp_hash_pointer->ptr->key = curr_key;
				temp_hash_pointer->ptr->val = curr_val;

				std::cout << "Swap " << curr_key << " with " << temp_key << std::endl;

				curr_key = temp_key;
				curr_val = temp_val;
			}

			//std::cout << "Abort" << std::endl;
		}

		bool remove(std::string key) {
			// // Hash to two buckets.
		 //    uint32_t h1, h2;
		 //    hashlittle2((void*)&key, key.length(), &h1, &h2);

		 //    h1 = h1 % m_num_buckets;
		 //    h2 = h2 % m_num_buckets;
		 //    unsigned char tag = tag_hash(key);

		 //    // Look at the first bucket.
		 //    for (int i = (int)h1; i < (int)h1 + SLOTS_PER_BUCKET; i++) {
		 //    	HashPointer* hash_pointer = m_table[i];
		 //    	if (hash_pointer != NULL && tag == hash_pointer->tag) {
		 //    		HashEntry* hash_entry = hash_pointer->ptr;
		 //    		if (key == hash_entry->key) {
		 //    			delete hash_entry;
		 //    			delete hash_pointer;
		 //    			m_table[i] = NULL;
		 //    			return true;
		 //    		}
		 //    	}
		 //    }

		 //    // Look at the second bucket.
			// for (int i = (int)h2; i < (int)h2 + SLOTS_PER_BUCKET; i++) {
		 //    	HashPointer* hash_pointer = m_table[i];
		 //    	if (hash_pointer != NULL && tag == hash_pointer->tag) {
		 //    		HashEntry* hash_entry = hash_pointer->ptr;
		 //    		if (key == hash_entry->key) {
		 //    			delete hash_entry;
		 //    			delete hash_pointer;
		 //    			m_table[i] = NULL;
		 //    			return true;
		 //    		}
		 //    	}
		 //    }
		 //    return false;
		}

	private:
		struct HashEntry {
			std::string key;
			T val;
		};
		struct HashPointer {
			unsigned char tag; // One byte tag.
			HashEntry* ptr;
		};
		HashPointer **m_table;
		int m_num_buckets;

		unsigned char tag_hash(std::string to_hash) {
	  		int hash = 0;
	  		for (int i = 0; i < to_hash.length(); i++)
		     	hash = ((37 * hash) + to_hash.at(i)) & 0xff;
		    return (unsigned char)hash;
		}

		// T search_for_key(bool delete_entry=false) {

		// }



};

#endif