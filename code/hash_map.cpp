#include <iostream>
#include <mutex>

#include "common/CycleTimer.h"
#include "common/errors.h"


template <typename T, typename U>
HashMap<T,U>::HashMap(int table_size, float max_load_factor) {

	m_table_size = table_size;
	m_max_load_factor = max_load_factor;
	m_table = new HashEntry*[table_size];

	for (int i = 0; i < table_size; i++)
		m_table[i] = NULL;
}

template <typename T, typename U>
HashMap<T,U>::~HashMap() {

	for (int i = 0; i < m_table_size; i++) {
		HashEntry *curr = m_table[i];
		HashEntry *temp;
		while (curr != NULL) {
			temp = curr;
			curr = curr->next;
			delete temp;
		}
	}
	delete[] m_table;
}

template <typename T, typename U>
U HashMap<T,U>::get(T key) {

	std::hash<T> hasher;
	int bucket = hasher(key) % m_table_size;

	HashEntry *curr = m_table[bucket];

	while (curr != NULL) {
		if (curr->key == key)
			return curr->val;
		curr = curr->next;
	}

	throw KeyNotFoundError(key.c_str());
}

template <typename T, typename U>
void HashMap<T,U>::put(T key, U val) {

	std::hash<T> hasher;
	int bucket = hasher(key) % m_table_size;

	HashEntry *prev = NULL;
	HashEntry *curr = m_table[bucket];

	// Traverse the chain. If the key already exists,
	// update the value for that key.

	while (curr != NULL) {
		if (curr->key == key) {
			curr->val = val;
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

	if (get_load_factor() > m_max_load_factor)
		resize();
}

template <typename T, typename U>
bool HashMap<T,U>::remove(T key) {

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

template <typename T, typename U>
float HashMap<T,U>::get_load_factor() {
	return 1.0 * m_size / m_table_size;
}

template <typename T, typename U>
int HashMap<T,U>::get_size() {
	return m_size;
}

template <typename T, typename U>
void HashMap<T,U>::resize() {

	double startTime = CycleTimer::currentSeconds();

	HashEntry **old_m_table = m_table;
	int old_m_table_size = m_table_size;

	m_table_size *= 2;
	m_size = 0;
	m_table = new HashEntry*[2*old_m_table_size];

	for (int i = 0; i < m_table_size; i++)
		m_table[i] = NULL;

	for (int i = 0; i < old_m_table_size; i++) {
		HashEntry *curr = old_m_table[i];
		HashEntry *temp;
		while (curr != NULL) {
			temp = curr;
			put(curr->key, curr->val);
			curr = curr->next;
			delete temp;
		}
	}

	delete[] old_m_table;

	double endTime = CycleTimer::currentSeconds();
	std::cout << "Resized table to: " << m_table_size << " in " << endTime-startTime << " seconds" << std::endl;
}
