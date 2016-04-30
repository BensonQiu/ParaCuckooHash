#include <iostream>
#include <string>
#include <stdlib.h>

#include "common/hash.h"
#include "common/errors.h"
#include <vector>
#include <iterator>

template <typename T>
SegmentHashMap<T>::SegmentHashMap(int num_buckets, int concurrency) {

    m_segments = new Segment<T>*[concurrency];
    for (int i = 0; i < concurrency; i++) {
        m_segments[i] = new Segment<T>(num_buckets / concurrency);
    }
    m_concurrency = concurrency;
};

template <typename T>
SegmentHashMap<T>::~SegmentHashMap() {

    for (int i = 0; i < m_concurrency; i++)
        delete m_segments[i];
    delete[] m_segments;
}

template <typename T>
T SegmentHashMap<T>::get(std::string key) {

    uint32_t h1, h2;
    h1 = 0;
    h2 = 0;
    hashlittle2(key.c_str(), key.length(), &h1, &h2);

    int segment_index = h1 % m_concurrency;

    return m_segments[segment_index]->get(key, h1, h2);
}

template <typename T>
void SegmentHashMap<T>::put(std::string key, T val) {

    uint32_t h1, h2;
    h1 = 0;
    h2 = 0;
    hashlittle2(key.c_str(), key.length(), &h1, &h2);

    int segment_index = h1 % m_concurrency;

    return m_segments[segment_index]->put(key, val, h1, h2);
}

////////////////////////////////////////////
////////// SEGMENT IMPLEMENTATION //////////
////////////////////////////////////////////

template <typename T>
Segment<T>::Segment(int num_buckets) {

    m_table = new HashEntry*[num_buckets * SLOTS_PER_BUCKET]();
    m_num_buckets = num_buckets;
    m_key_version_counters = new uint32_t[NUM_KEY_VERSION_COUNTERS]();
};

template <typename T>
Segment<T>::~Segment() {

    for (int i = 0; i < m_num_buckets * SLOTS_PER_BUCKET; i++)
        delete m_table[i];
    delete[] m_table;
    delete[] m_key_version_counters;
}

template <typename T>
T Segment<T>::get(std::string key, uint32_t h1, uint32_t h2) {

    h1 = h1 % m_num_buckets;
    h2 = h2 % m_num_buckets;

    h1 *= SLOTS_PER_BUCKET;
    h2 *= SLOTS_PER_BUCKET;

    int key_version_index = h1 % NUM_KEY_VERSION_COUNTERS;

    Try:
    uint32_t key_version_start = __sync_fetch_and_add(&m_key_version_counters[key_version_index], 0);

    // Look at the first bucket.
    for (int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {
        HashEntry* hash_entry = m_table[i];
        if (hash_entry != NULL) {
            if (key == hash_entry->key) {
                T val = hash_entry->val;
                uint32_t key_version_end = __sync_add_and_fetch(&m_key_version_counters[key_version_index], 0);
                if (key_version_start & 0x1 || key_version_start != key_version_end) {
                    goto Try;
                }
                return val;
            }
        }
    }

    // Look at the second bucket.
    for (int i = h2; i < h2 + SLOTS_PER_BUCKET; i++) {
        HashEntry* hash_entry = m_table[i];
        if (hash_entry != NULL) {
            if (key == hash_entry->key) {
                T val = hash_entry->val;
                uint32_t key_version_end = __sync_add_and_fetch(&m_key_version_counters[key_version_index], 0);
                if (key_version_start & 0x1 || key_version_start != key_version_end) {
                    goto Try;
                }
                return val;
            }
        }
    }

    uint32_t key_version_end = __sync_add_and_fetch(&m_key_version_counters[key_version_index], 0);
    if (key_version_start & 0x1 || key_version_start != key_version_end) {
        goto Try;
    }
    throw KeyNotFoundError(key.c_str());
}

template <typename T>
void Segment<T>::put(std::string key, T val, uint32_t h1, uint32_t h2) {
    m_write_mutex.lock();

    int num_iters = 0;
    std::string curr_key = key;
    T curr_val = val;

    h1 = h1 % m_num_buckets;
    h2 = h2 % m_num_buckets;

    h1 *= SLOTS_PER_BUCKET;
    h2 *= SLOTS_PER_BUCKET;

    int key_version_index = h1 % NUM_KEY_VERSION_COUNTERS;

    std::vector<int> path;
    std::vector<int> key_version_array;

    // Find the path of elements to be evicted for key insertion.
    while (num_iters++ < MAX_ITERS) {

        // Look at the first bucket.
        for (int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {

            HashEntry* hash_entry = m_table[i];

            if (hash_entry == NULL) {
                // Found an empty bucket slot for the current key.
                path.push_back(i);
                goto SwapPathKeys;
            } else if (num_iters == 1 && hash_entry->key == curr_key) {
                // If the key already exists, update the value and return immediately.
                __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
                hash_entry->val = curr_val;
                __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
                m_write_mutex.unlock();
                return;
            }
        }

        // Look at the second bucket.
        for (int i = h2; i < h2 + SLOTS_PER_BUCKET; i++){

            HashEntry* hash_entry = m_table[i];

            if (hash_entry == NULL){
                // Found an empty bucket slot for the current key.
                path.push_back(i);

                goto SwapPathKeys;
            } else if (num_iters == 1 && hash_entry->key == curr_key) {
                // If the key already exists, update the value and return immediately.
                __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
                hash_entry->val = curr_val;
                __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
                m_write_mutex.unlock();
                return;
            }
        }

        // If the key can't be placed in either bucket,
        // randomly choose an existing key to evict.
        int index = rand() % (2 * SLOTS_PER_BUCKET);
        HashEntry* temp_hash_entry;
        if (0 <= index && index < SLOTS_PER_BUCKET) {
            path.push_back(h1 + index);
            temp_hash_entry = m_table[h1 + index];
        }
        else {
            path.push_back(h2 + index - SLOTS_PER_BUCKET);
            temp_hash_entry = m_table[h2 + index - SLOTS_PER_BUCKET];
        }

        std::string temp_key = temp_hash_entry->key;
        T temp_val = temp_hash_entry->val;

        h1 = 0;
        h2 = 0;
        hashlittle2(temp_key.c_str(), temp_key.length(), &h1, &h2);

        h1 = h1 % m_num_buckets;
        h2 = h2 % m_num_buckets;

        h1 *= SLOTS_PER_BUCKET;
        h2 *= SLOTS_PER_BUCKET;

        key_version_index = h1 % NUM_KEY_VERSION_COUNTERS;
        key_version_array.push_back(key_version_index);

        // Now move on to the next iteration and try to place evicted key
        // into a bucket
        curr_key = temp_key;
        curr_val = temp_val;
    }

 SwapPathKeys:

    if (path.size() == MAX_ITERS) {
        std::cout << "Abort" << std::endl;
        m_write_mutex.unlock();
        return;
    }

    // Case 1: Key finds a slot immediately.
    if (path.size() == 1) {
        int index = path.front();
        HashEntry* hash_entry = new HashEntry();
        hash_entry->key = key;
        hash_entry->val = val;
        m_table[index] = hash_entry;
        m_write_mutex.unlock();
        return;
    }

    // Case 2: Evictions needed to insert key.
    std::vector<int>::reverse_iterator key_version_iterator = key_version_array.rbegin();
    std::vector<int>::reverse_iterator path_iterator = path.rbegin();

    int to_index = *path_iterator++;

    for (; path_iterator != path.rend(); path_iterator++) {

        int from_index = *path_iterator;
        int key_version_index = *key_version_iterator++;

        __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
        HashEntry* from_hash_entry = m_table[from_index];
        HashEntry* to_hash_entry = new HashEntry();
        to_hash_entry->key = from_hash_entry->key;
        to_hash_entry->val = from_hash_entry->val;
        m_table[to_index] = to_hash_entry;
        __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);

        to_index = from_index;
    }

    int first_index = path.front();
    __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
    m_table[first_index]->key = key;
    m_table[first_index]->val = val;
    __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);

    m_write_mutex.unlock();
}
