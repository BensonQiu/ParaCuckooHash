#include <atomic>
#include <iostream>
#include <mutex>

#include "common/CycleTimer.h"
#include "common/errors.h"


template <typename T>
FineHashMap<T>::FineHashMap(int table_size, float max_load_factor) {

	m_table_size = table_size;
	m_size = 0;
	m_max_load_factor = max_load_factor;
	m_table = new HashEntry*[table_size];
	m_bucket_mutexes = new std::mutex[table_size];

	for (int i = 0; i < table_size; i++)
		m_table[i] = NULL;
}

template <typename T>
FineHashMap<T>::~FineHashMap() {

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

template <typename T>
T FineHashMap<T>::get(std::string key) {

	std::hash<T> hasher;
	int bucket = hasher(key) % m_table_size;

	m_bucket_mutexes[bucket].lock();

	HashEntry *curr = m_table[bucket];

	while (curr != NULL) {
		if (curr->key == key) {
			m_bucket_mutexes[bucket].unlock();
			return curr->val;
		}
		curr = curr->next;
	}

	m_bucket_mutexes[bucket].unlock();
	throw KeyNotFoundError(key.c_str());
}

template <typename T>
float FineHashMap<T>::get_load_factor() {
	return 1.0 * m_size / m_table_size;
}

template <typename T>
int FineHashMap<T>::get_size() {
	return m_size;
}

template <typename T>
void FineHashMap<T>::put(std::string key, T val) {

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

template <typename T>
bool FineHashMap<T>::remove(std::string key) {

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
