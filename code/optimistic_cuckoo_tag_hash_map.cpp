#include <iostream>
#include <string>
#include <stdlib.h>

#include "common/hash.h"
#include "common/errors.h"
#include <vector>
#include <iterator>

template <typename T>
OptimisticCuckooTagHashMap<T>::OptimisticCuckooTagHashMap(int num_buckets) {

    m_table = new HashPointer[num_buckets * SLOTS_PER_BUCKET]();
    m_num_buckets = num_buckets;
    m_key_version_counters = new uint32_t[NUM_KEY_VERSION_COUNTERS]();
    m_visited_bitmap = new int[num_buckets * SLOTS_PER_BUCKET]();
};

template <typename T>
OptimisticCuckooTagHashMap<T>::~OptimisticCuckooTagHashMap() {

    for (int i = 0; i < m_num_buckets * SLOTS_PER_BUCKET; i++) {
        if (m_table[i].ptr != NULL) {
            delete m_table[i].ptr;
        }
    }
    delete[] m_table;
    delete[] m_key_version_counters;
    delete[] m_visited_bitmap;
}

template <typename T>
T OptimisticCuckooTagHashMap<T>::get(std::string key) {

    // Hash to two buckets.
    uint32_t h1, h2;
    h1 = 0;
    h2 = 0;

    hashlittle2(key.c_str(), key.length(), &h1, &h2);

    unsigned char tag = tag_hash(h1);

    h1 = h1 % m_num_buckets;
    h2 = h2 % m_num_buckets;

    h1 *= SLOTS_PER_BUCKET;
    h2 *= SLOTS_PER_BUCKET;

    int key_version_index = h1 % NUM_KEY_VERSION_COUNTERS;

    Try:
    uint32_t key_version_start = __sync_fetch_and_add(&m_key_version_counters[key_version_index], 0);

    // Look at the first bucket.
    for (unsigned int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {
        HashPointer hash_pointer = m_table[i];
        if (hash_pointer.tag == tag && hash_pointer.ptr != NULL) {
            HashEntry* hash_entry = hash_pointer.ptr;
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
    for (unsigned int i = h2; i < h2 + SLOTS_PER_BUCKET; i++) {
        // HashEntry* hash_entry = m_table[i];
        HashPointer hash_pointer = m_table[i];
        if (hash_pointer.tag == tag && hash_pointer.ptr != NULL) {
            HashEntry* hash_entry = hash_pointer.ptr;
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
    // return "";
    throw KeyNotFoundError(key.c_str());
}

template <typename T>
void OptimisticCuckooTagHashMap<T>::put(std::string key, T val) {
    m_write_mutex.lock();

    int num_iters = 0;
    std::string curr_key = key;
    T curr_val = val;

    uint32_t h1, h2;
    h1 = 0;
    h2 = 0;

    hashlittle2(curr_key.c_str(), curr_key.length(), &h1, &h2);

    unsigned char tag = tag_hash(h1);

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
        for (unsigned int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {

            // HashEntry* hash_entry = m_table[i];
            HashPointer hash_pointer = m_table[i];
            if (hash_pointer.ptr == NULL) {
                // Found an empty bucket slot for the current key.
                path.push_back(i);
                goto SwapPathKeys;
            } else if (num_iters == 1 && hash_pointer.ptr->key == curr_key) {
                // If the key already exists on the first iteration,
                // update the value and return immediately.
                // For the case where num_iters > 1, we are searching for an
                // eviction path but haven't evicted any elements yet,
                // so we should always expect to find key we are evicting,
                // but should not return from the function.
                __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
                hash_pointer.ptr->val = curr_val;
                __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
                m_write_mutex.unlock();
                return;
            }
        }

        // Look at the second bucket.
        for (unsigned int i = h2; i < h2 + SLOTS_PER_BUCKET; i++){

            HashPointer hash_pointer = m_table[i];
            if (hash_pointer.ptr == NULL){
                // Found an empty bucket slot for the current key.
                path.push_back(i);
                goto SwapPathKeys;
            } else if (num_iters == 1 && hash_pointer.ptr->key == curr_key) {
                // If the key already exists on the first iteration,
                // update the value and return immediately.
                // For the case where num_iters > 1, we are searching for an
                // eviction path but haven't evicted any elements yet,
                // so we should always expect to find key we are evicting,
                // but should not return from the function.
                __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
                hash_pointer.ptr->val = curr_val;
                __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
                m_write_mutex.unlock();
                return;
            }
        }

        // If the key can't be placed in either bucket,
        // randomly choose an existing key to evict.
        int index = rand() % (2 * SLOTS_PER_BUCKET);
        // HashEntry* temp_hash_entry;
        int evicted_index;
        if (0 <= index && index < SLOTS_PER_BUCKET) {
            evicted_index = h1 + index;
        } else {
            evicted_index = h2 + index - SLOTS_PER_BUCKET;
        }

        HashEntry* temp_hash_entry;
        // HashPointer temp_hash_pointer;
        if (m_visited_bitmap[evicted_index] == 0) {
            m_visited_bitmap[evicted_index] = 1;
            path.push_back(evicted_index);
            temp_hash_entry = m_table[evicted_index].ptr;
        } else {
            // Try to evict another index so that we don't get a cycle.
            for (unsigned int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {
                if (m_visited_bitmap[i] == 0) {
                    m_visited_bitmap[i] = 1;
                    path.push_back(i);
                    temp_hash_entry = m_table[i].ptr;
                    goto EvictedKey;
                }
            }
            for (unsigned int i = h2; i < h2 + SLOTS_PER_BUCKET; i++) {
                if (m_visited_bitmap[i] == 0) {
                    m_visited_bitmap[i] = 1;
                    path.push_back(i);
                    temp_hash_entry = m_table[i].ptr;
                    goto EvictedKey;
                }
            }
            std::cout << "Cycle case: " << key << std::endl;
            m_write_mutex.unlock();
            return;
        }

EvictedKey:

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

    if ((int)path.size() >= MAX_ITERS) {
        m_write_mutex.unlock();
        return;
    }

    // Case 1: Key finds a slot immediately.
    if (path.size() == 1) {
        int index = path.front();

        HashEntry* hash_entry = new HashEntry();
        hash_entry->key = key;
        hash_entry->val = val;

        (&m_table[index])->tag = tag;
        m_table[index].ptr = hash_entry;
        m_write_mutex.unlock();
        return;
    }

    // Case 2: Evictions needed to insert key.
    std::vector<int>::reverse_iterator key_version_iterator = key_version_array.rbegin();
    std::vector<int>::reverse_iterator path_iterator = path.rbegin();


    int to_index = *path_iterator++;
    m_visited_bitmap[to_index] = 0;

    for (; path_iterator != path.rend(); path_iterator++) {

        int from_index = *path_iterator;
        int key_version_index = *key_version_iterator++;

        m_visited_bitmap[from_index] = 0;

        __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
        HashPointer from_hash_pointer = m_table[from_index];
        HashPointer* to_hash_pointer = &m_table[to_index];
        to_hash_pointer->tag = from_hash_pointer.tag;
        to_hash_pointer->ptr = new HashEntry();
        to_hash_pointer->ptr->key = from_hash_pointer.ptr->key;
        to_hash_pointer->ptr->val = from_hash_pointer.ptr->val;
        if (m_table[from_index].ptr != NULL) {
            delete m_table[from_index].ptr;
        }
        __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
        to_index = from_index;
    }

    int first_index = path.front();

    m_visited_bitmap[first_index] = 0;

    __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);
    HashPointer* hash_pointer = &m_table[first_index];
    hash_pointer->tag = tag;
    hash_pointer->ptr = new HashEntry();
    hash_pointer->ptr->key = key;
    hash_pointer->ptr->val = val;
    __sync_fetch_and_add(&m_key_version_counters[key_version_index], 1);

    m_write_mutex.unlock();
}

template <typename T>
unsigned char OptimisticCuckooTagHashMap<T>::tag_hash(const uint32_t hv) {
    uint32_t r =  hv & m_tagmask;
    return (unsigned char) r + (r == 0);
}
