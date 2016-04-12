#include <iostream>
#include <string>
#include <stdlib.h>

#include "common/hash.h"
#include "common/errors.h"


template <typename T>
OptimisticCuckooHashMap<T>::OptimisticCuckooHashMap(int num_buckets) {

    m_table = new HashEntry*[num_buckets * SLOTS_PER_BUCKET]();
    m_num_buckets = num_buckets;
    m_key_version_counters = new uint32_t[NUM_KEY_VERSION_COUNTERS]();
};

template <typename T>
OptimisticCuckooHashMap<T>::~OptimisticCuckooHashMap() {

    for (int i = 0; i < m_num_buckets * SLOTS_PER_BUCKET; i++)
        delete m_table[i];
    delete[] m_table;
    delete[] m_key_version_counters;
}

template <typename T>
T OptimisticCuckooHashMap<T>::get(std::string key) {

    // Hash to two buckets.
    uint32_t h1, h2;
    h1 = 0;
    h2 = 0;

    hashlittle2(key.c_str(), key.length(), &h1, &h2);

    h1 = h1 % m_num_buckets;
    h2 = h2 % m_num_buckets;

    if (h1 < 0){
      h1 = m_num_buckets - abs(h1);
    }

    if (h2 < 0){
        h2 = m_num_buckets - abs(h2);
    }

    h1 *= SLOTS_PER_BUCKET;
    h2 *= SLOTS_PER_BUCKET;

    int key_version_index = h1 % NUM_KEY_VERSION_COUNTERS;

    Try:
    uint32_t key_version_start = __sync_fetch_and_add(&key_version_index, 0);

    if (key.compare("744304") == 0) {
        std::cout << "Getting key 744304 in bucket 1: " << h1 << " and bucket 2: " << h2 << std::endl;
    }

    // Look at the first bucket.
    for (int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {
        HashEntry* hash_entry = m_table[i];
        if (hash_entry != NULL) {
            if (key == hash_entry->key) {
                uint32_t key_version_end = __sync_add_and_fetch(&key_version_index, 0);
                if (key_version_start & 0x1 || key_version_start != key_version_end) {
                    std::cout << "Had to retry with key: " << key << std::endl;
                    goto Try;
                }
                return hash_entry->val;
            }
        }
    }

    // Look at the second bucket.
    for (int i = h2; i < h2 + SLOTS_PER_BUCKET; i++) {
        HashEntry* hash_entry = m_table[i];
        if (hash_entry != NULL) {
            if (key == hash_entry->key) {
                uint32_t key_version_end = __sync_add_and_fetch(&key_version_index, 0);
                if (key_version_start & 0x1 || key_version_start != key_version_end) {
                    std::cout << "Had to retry with key: " << key << std::endl;
                    goto Try;
                }
                return hash_entry->val;
            }
        }
    }

    uint32_t key_version_end = __sync_add_and_fetch(&key_version_index, 0);
    if (key_version_start & 0x1 || key_version_start != key_version_end) {
        std::cout << "Had to retry with key: " << key << std::endl;
        goto Try;
    }
    throw KeyNotFoundError(key.c_str());
}

template <typename T>
void OptimisticCuckooHashMap<T>::put(std::string key, T val) {

    int num_iters = 0;
    std::string curr_key = key;
    T curr_val = val;

    uint32_t h1, h2;
    h1 = 0;
    h2 = 0;

    hashlittle2(curr_key.c_str(), curr_key.length(), &h1, &h2);

    h1 = h1 % m_num_buckets;
    h2 = h2 % m_num_buckets;

    if (h1 < 0)
        h1 = m_num_buckets - abs(h1);

    if (h2 < 0)
        h2 = m_num_buckets - abs(h2);

    h1 *= SLOTS_PER_BUCKET;
    h2 *= SLOTS_PER_BUCKET;

    int key_version_index = h1 % NUM_KEY_VERSION_COUNTERS;

    while (num_iters < MAX_ITERS) {
        num_iters++;


        if (curr_key.compare("744304") == 0) {
            std::cout << "Put Key: " << curr_key << ", Bucket 1: " << h1 << ", Bucket 2: " << h2 << std::endl;
        }

        // Look at the first bucket.
        for (int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {

            HashEntry* hash_entry = m_table[i];

            if (hash_entry == NULL) {
                HashEntry* hash_entry = new HashEntry();
                hash_entry->key = curr_key;
                hash_entry->val = curr_val;
                m_table[i] = hash_entry;
                if (curr_key.compare("744304") == 0) {
                    std::cout <<"Put Key: " << curr_key << " in slot index: " << i << std::endl;
                }
                return;
            } else if (hash_entry->key == curr_key) {
                __sync_fetch_and_add(&key_version_index, 1);
                hash_entry->val = curr_val;
                __sync_fetch_and_add(&key_version_index, 1);
                return;
            }
        }

        // Look at the second bucket.
        for (int i = (int)h2; i < (int)h2 + SLOTS_PER_BUCKET; i++) {
            HashEntry* hash_entry = m_table[i];
            if (hash_entry == NULL) {
                HashEntry* hash_entry = new HashEntry();
                hash_entry->key = curr_key;
                hash_entry->val = curr_val;
                m_table[i] = hash_entry;
                if (curr_key.compare("744304") == 0) {
                    std::cout <<"Put Key: " << curr_key << " in slot index: " << i << std::endl;
                }
                return;
            } else if (hash_entry->key == curr_key) {
                __sync_fetch_and_add(&key_version_index, 1);
                hash_entry->val = curr_val;
                __sync_fetch_and_add(&key_version_index, 1);
                return;
            }
        }

        int index = rand() % (2 * SLOTS_PER_BUCKET);
        HashEntry* temp_hash_entry;
        if (0 <= index && index < SLOTS_PER_BUCKET)
            temp_hash_entry = m_table[h1 + index];
        else
            temp_hash_entry = m_table[h2 + index - SLOTS_PER_BUCKET];

        std::string temp_key = temp_hash_entry->key;
        T temp_val = temp_hash_entry->val;

        // ***** BEGIN TODO: DO THIS IN A HELPER METHOD *****
        hashlittle2(temp_key.c_str(), temp_key.length(), &h1, &h2);

        h1 = h1 % m_num_buckets;
        h2 = h2 % m_num_buckets;

        if (h1 < 0)
            h1 = m_num_buckets - abs(h1);

        if (h2 < 0)
            h2 = m_num_buckets - abs(h2);

        h1 *= SLOTS_PER_BUCKET;
        h2 *= SLOTS_PER_BUCKET;

        key_version_index = h1 % NUM_KEY_VERSION_COUNTERS;

        // ***** END TODO: DO THIS IN A HELPER METHOD *****
        __sync_fetch_and_add(&key_version_index, 1);
        temp_hash_entry->key = curr_key;
        temp_hash_entry->val = curr_val;
        __sync_fetch_and_add(&key_version_index, 1);

        std::cout << "Swap " << curr_key << " with " << temp_key << std::endl;

        curr_key = temp_key;
        curr_val = temp_val;

    }

    std::cout << "Abort" << std::endl;
}
