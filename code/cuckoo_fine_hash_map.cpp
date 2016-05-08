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

    if (!(0 <= h1 && h1 < m_num_buckets)) {
        std::cout << "Lock Helper Fail for h1: " << h1 << std::endl;
    }
    if (!(0 <= h2 && h2 < m_num_buckets)) {
        std::cout << "Lock Helper Fail for h2: " << h2 << std::endl;
    }

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

    if (!(0 <= h1 && h1 < m_num_buckets)) {
        std::cout << "Unlock Helper Fail for h1: " << h1 << std::endl;
    }
    if (!(0 <= h2 && h2 < m_num_buckets)) {
        std::cout << "Unlock Helper Fail for h2: " << h2 << std::endl;
    }

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

        //std::cout << "Put Key: " << curr_key << ", Bucket 1: " << h1 << ", Bucket 2: " << h2 << std::endl;

        //std::cout << "Put 01" << std::endl;
        lock_helper(h1, h2);
        //std::cout << "Put 02" << std::endl;

        // Look at the first bucket.
        for (unsigned int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {

            //std::cout << "Bucket 1 about to access i: " << i << std::endl;
            HashEntry* hash_entry = m_table[i];
            //std::cout << "Bucket 1 done to access i: " << i << std::endl;

            if (hash_entry == NULL) {
                HashEntry* hash_entry = new HashEntry();
                hash_entry->key = curr_key;
                hash_entry->val = curr_val;

                //std::cout << "Bucket 1 about to set m_table[i]: " << i << std::endl;
                m_table[i] = hash_entry;
                //std::cout << "Bucket 1 done to set m_table[i]: " << i << std::endl;

                //std::cout << "Put 03" << std::endl;
                unlock_helper(h1, h2);
                //std::cout << "Put 04" << std::endl;

                return;
            } else if (hash_entry->key == curr_key) {

                //std::cout << "Bucket 1 about to set hash_enttry->val: " << i << std::endl;
                hash_entry->val = curr_val;
                //std::cout << "Bucket 1 about to set hash_enttry->val: " << i << std::endl;

                //std::cout << "Put 05" << std::endl;
                unlock_helper(h1, h2);
                //std::cout << "Put 06" << std::endl;
                return;
            }
        }

        //std::cout << "Now looking at the second bucket..." << std::endl;

        // Look at the second bucket.
        for (unsigned int i = h2; i < h2 + SLOTS_PER_BUCKET; i++) {

            //std::cout << "Bucket 2 about to access i: " << i << std::endl;
            HashEntry* hash_entry = m_table[i];
            //std::cout << "Bucket 2 done to access i: " << i << std::endl;

            if (hash_entry == NULL) {
                HashEntry* hash_entry = new HashEntry();
                hash_entry->key = curr_key;
                hash_entry->val = curr_val;

                //std::cout << "Bucket 2 about to set m_table[i]: " << i << std::endl;
                m_table[i] = hash_entry;
                //std::cout << "Bucket 2 done to set m_table[i]: " << i << std::endl;

                //std::cout << "Put 07" << std::endl;
                unlock_helper(h1, h2);
                //std::cout << "Put 08" << std::endl;
                return;
            } else if (hash_entry->key == curr_key) {

                //std::cout << "Bucket 1 about to set hash_enttry->val: " << i << std::endl;
                hash_entry->val = curr_val;
                //std::cout << "Bucket 1 about to set hash_enttry->val: " << i << std::endl;

                //std::cout << "Put 09" << std::endl;
                unlock_helper(h1, h2);
                //std::cout << "Put 10" << std::endl;
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

        // //std::cout << "Swap " << curr_key << " with " << temp_key << std::endl;

        curr_key = temp_key;
        curr_val = temp_val;

        //std::cout << "Put 11" << std::endl;
        unlock_helper(h1, h2);
        //std::cout << "Put 12" << std::endl;
    }

    std::cout << "Abort" << std::endl;
}
