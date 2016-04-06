#ifndef COARSE_HASHMAP_H
#define COARSE_HASHMAP_H

#include <atomic>
#include <iostream>
#include <mutex>

#include "common/CycleTimer.h"
#include "common/errors.h"


template <typename T, typename U>
class CoarseHashMap {

	public:
		CoarseHashMap(int table_size=64, float max_load_factor=1.5f) {

			m_table_size = table_size;
			m_size = 0;
			m_max_load_factor = max_load_factor;
			m_table = new HashEntry*[table_size];
			m_bucket_mutexes = new std::mutex[table_size];

			for (int i = 0; i < table_size; i++)
				m_table[i] = NULL;
		}

		~CoarseHashMap() {

			for (int i = 0; i < m_table_size; i++) {
				HashEntry *curr = m_table[i];
				HashEntry *temp;
				while (curr != NULL) {
					temp = curr;
					curr = curr->next;
					delete temp;
				}
			}
			delete[] m_bucket_mutexes;
			delete[] m_table;
		}

		U get(T key) {

			std::hash<T> hasher;
			int bucket = hasher(key) % m_table_size;

			HashEntry *prev = NULL;
			HashEntry *curr = m_table[bucket];

			while (curr != NULL) {
				if (curr->key == key)
					return curr->val;
				curr = curr->next;
			}

			throw KeyNotFoundError(std::to_string(key).c_str());
		}

		float get_load_factor() {
			return 1.0 * m_size / m_table_size;
		}

		int get_size() {
			return m_size;
		}

		void put(T key, U val, int thread_ID=-1) {

			std::hash<T> hasher;
			int bucket = hasher(key) % m_table_size;

			m_bucket_mutexes[bucket].lock();

			HashEntry *prev = NULL;
			HashEntry *curr = m_table[bucket];

			// Traverse the chain. If the key already exists,
			// update the value for that key.
			while (curr != NULL) {
				if (curr->key == key) {
					curr->val = val;
					m_bucket_mutexes[bucket].unlock();
					return;
				}
				prev = curr;
				curr = curr->next;
			}

			m_size++;

			HashEntry *temp = new HashEntry();
			temp->key = key;
			temp->val = val;

			if (prev == NULL) {
				// First element in bucket.
				m_table[bucket] = temp;
			} else {
				prev->next = temp;
			}

			m_bucket_mutexes[bucket].unlock();
			
		}

		bool remove(T key) {

			std::hash<T> hasher;
			int bucket = hasher(key) % m_table_size;

			HashEntry *prev = NULL;
			HashEntry *curr = m_table[bucket];

			while (curr != NULL) {
				if (curr->key == key) {
					if (prev == NULL) {
						m_table[bucket] = NULL;
					} else {
						prev->next = curr->next;
					}
					m_size--;
					delete curr;
					return true;
				}
				prev = curr;
				curr = curr->next;
			}
			return false;
		}

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

};

#endif
