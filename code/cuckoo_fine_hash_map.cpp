#include <iostream>
#include <string>
#include <stdlib.h>

#include "common/hash.h"
#include "common/errors.h"


template <typename T>
CuckooFineHashMap<T>::CuckooFineHashMap(int num_buckets) {
    m_table = new HashEntry*[num_buckets * SLOTS_PER_BUCKET]();
    m_num_buckets = num_buckets;
    m_bucket_locks = new std::mutex[num_buckets];
};

template <typename T>
CuckooFineHashMap<T>::~CuckooFineHashMap() {
    for (int i = 0; i < m_num_buckets * SLOTS_PER_BUCKET; i++)
        delete m_table[i];
    delete[] m_table;
    delete[] m_bucket_locks;
}

template <typename T>
T CuckooFineHashMap<T>::get(std::string key) {
    // Hash to two buckets.
    uint32_t h1, h2;
    h1 = 0;
    h2 = 0;

    hashlittle2(key.c_str(), key.length(), &h1, &h2);

    h1 = h1 % m_num_buckets;
    h2 = h2 % m_num_buckets;

    h1 *= SLOTS_PER_BUCKET;
    h2 *= SLOTS_PER_BUCKET;

    lock_helper(h1, h2);

    // Look at the first bucket.
    for (unsigned int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {
        HashEntry* hash_entry = m_table[i];
        if (hash_entry != NULL) {
            if (key == hash_entry->key) {

                unlock_helper(h1, h2);
                return hash_entry->val;
            }
        }
    }

    // Look at the second bucket.
    for (unsigned int i = h2; i < h2 + SLOTS_PER_BUCKET; i++) {
        HashEntry* hash_entry = m_table[i];
        if (hash_entry != NULL) {
            if (key == hash_entry->key) {

                unlock_helper(h1, h2);
                return hash_entry->val;
            }
        }
    }

    unlock_helper(h1, h2);
    throw KeyNotFoundError(key.c_str());
}

template <typename T>
void CuckooFineHashMap<T>::lock_helper(uint32_t h1, uint32_t h2) {

    h1 = h1 / SLOTS_PER_BUCKET;
    h2 = h2 / SLOTS_PER_BUCKET;

    if (h1 < h2) {
        m_bucket_locks[h1].lock();
        m_bucket_locks[h2].lock();
    } else if (h2 < h1) {
        m_bucket_locks[h1].lock();
        m_bucket_locks[h2].lock();
    } else if (h1 == h2) {
        m_bucket_locks[h1].lock();
    }
}

template <typename T>
void CuckooFineHashMap<T>::unlock_helper(uint32_t h1, uint32_t h2) {

    h1 = h1 / SLOTS_PER_BUCKET;
    h2 = h2 / SLOTS_PER_BUCKET;

    if (h1 == h2) {
        m_bucket_locks[h1].unlock();
    } else {
        m_bucket_locks[h1].unlock();
        m_bucket_locks[h2].unlock();
    }
}

template <typename T>
void CuckooFineHashMap<T>::put(std::string key, T val) {

    int num_iters = 0;
    std::string curr_key = key;
    T curr_val = val;

    while (num_iters < MAX_ITERS) {
        num_iters++;
        uint32_t h1, h2;
        h1 = 0;
        h2 = 0;

        hashlittle2(curr_key.c_str(), curr_key.length(), &h1, &h2);

        h1 = h1 % m_num_buckets;
        h2 = h2 % m_num_buckets;

        h1 *= SLOTS_PER_BUCKET;
        h2 *= SLOTS_PER_BUCKET;

        lock_helper(h1, h2);

        // Look at the first bucket.
        for (unsigned int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {

            HashEntry* hash_entry = m_table[i];

            if (hash_entry == NULL) {
                HashEntry* hash_entry = new HashEntry();
                hash_entry->key = curr_key;
                hash_entry->val = curr_val;
                m_table[i] = hash_entry;
                unlock_helper(h1, h2);
                return;
            } else if (hash_entry->key == curr_key) {
                hash_entry->val = curr_val;
                unlock_helper(h1, h2);
                return;
            }
        }

        // Look at the second bucket.
        for (unsigned int i = h2; i < h2 + SLOTS_PER_BUCKET; i++) {

            HashEntry* hash_entry = m_table[i];

            if (hash_entry == NULL) {
                HashEntry* hash_entry = new HashEntry();
                hash_entry->key = curr_key;
                hash_entry->val = curr_val;

                m_table[i] = hash_entry;
                unlock_helper(h1, h2);
                return;
            } else if (hash_entry->key == curr_key) {
                hash_entry->val = curr_val;
                unlock_helper(h1, h2);
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

        temp_hash_entry->key = curr_key;
        temp_hash_entry->val = curr_val;

        curr_key = temp_key;
        curr_val = temp_val;

        unlock_helper(h1, h2);
    }

    std::cout << "Abort" << std::endl;
}
