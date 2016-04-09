#ifndef BETTER_CUCKOO_HASHMAP_H
#define BETTER_CUCKOO_HASHMAP_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include <boost/functional/hash.hpp>

#include "common/hash.c"
#include "common/errors.h"


template <typename T>
class BetterCuckooHashMap {

    public:
        const int SLOTS_PER_BUCKET = 4;
        const int MAX_ITERS = 500;

        BetterCuckooHashMap(int num_buckets=64) {
            m_table = new HashPointer[num_buckets * SLOTS_PER_BUCKET]();
            m_num_buckets = num_buckets;
        };


        ~BetterCuckooHashMap() {
            for (int i = 0; i < m_num_buckets * SLOTS_PER_BUCKET; i++)
                delete m_table[i].ptr;
            delete[] m_table;
        }

        T get(std::string key) {
            // Hash to two buckets.
            uint32_t h1, h2;

            h1 = hashlittle(key.c_str(), key.length(), 0);    
            unsigned char tag = tag_hash(h1);

            h1 = h1 % m_num_buckets;
            if (h1 < 0){
              h1 = m_num_buckets - abs(h1);
            }
            h1 *= SLOTS_PER_BUCKET;

            // Look at the first bucket.
            for (int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {
                HashPointer hash_pointer = m_table[i];
                if (hash_pointer.tag == tag && hash_pointer.ptr != NULL) {
                    HashEntry* hash_entry = hash_pointer.ptr;
                    if (key == hash_entry->key)
                        return hash_entry->val;
                }
            }

            h2 = (h1 ^ (tag * 0x5bd1e995)) % m_num_buckets;
            if (h2 < 0){
                h2 = m_num_buckets - abs(h2);
            }
            h2 *= SLOTS_PER_BUCKET;

            // Look at the second bucket.
            for (int i = h2; i < h2 + SLOTS_PER_BUCKET; i++) {
                HashPointer hash_pointer = m_table[i];
                if (hash_pointer.tag == tag && hash_pointer.ptr != NULL) {
                    HashEntry* hash_entry = hash_pointer.ptr;
                    if (key == hash_entry->key)
                        return hash_entry->val;
                }
            }
            return "NOT FOUND";
            // throw KeyNotFoundError(key.c_str());
        }


        void put(std::string key, T val) {
            // Hash to two buckets.
            int num_iters = 0;
            std::string curr_key = key;
            T curr_val = val;

            while (num_iters < MAX_ITERS) {
                num_iters++;
                uint32_t h1, h2;

                h1 = hashlittle(key.c_str(), key.length(), 0); 
                unsigned char tag = tag_hash(h1);
                h2 = h1 ^ (tag * 0x5bd1e995);

                h1 = h1 % m_num_buckets;
                h2 = h2 % m_num_buckets;

                if (h1 < 0)
                    h1 = m_num_buckets - abs(h1);

                if (h2 < 0)
                    h2 = m_num_buckets - abs(h2);

                h1 *= SLOTS_PER_BUCKET;
                h2 *= SLOTS_PER_BUCKET;

                // std::cout << "Put Key: " << curr_key << ", Bucket 1: " << h1 << ", Bucket 2: " << h2 << std::endl;
                // std::cout << "Put Tag: " << (int) tag << std::endl;

                // Look at the first bucket.
                for (int i = h1; i < h1 + SLOTS_PER_BUCKET; i++) {
                    HashPointer hash_pointer = m_table[i];

                    if (hash_pointer.ptr == NULL) {
                        HashEntry* hash_entry = new HashEntry();
                        hash_entry->key = curr_key;
                        hash_entry->val = curr_val;

                        m_table[i].tag = tag;
                        m_table[i].ptr = hash_entry;
                        // std::cout <<"Put Key: " << curr_key << " in slot index: " << i << std::endl;
                        return;
                    } else if (m_table[i].ptr->key == curr_key) {
                        m_table[i].ptr->val = curr_val;
                        return;
                    }
                }

                // Look at the second bucket.
                for (int i = (int)h2; i < (int)h2 + SLOTS_PER_BUCKET; i++) {
                    
                    HashPointer hash_pointer = m_table[i];

                    if (hash_pointer.ptr == NULL) {
                        HashEntry* hash_entry = new HashEntry();
                        hash_entry->key = curr_key;
                        hash_entry->val = curr_val;

                        m_table[i].tag = tag;
                        m_table[i].ptr = hash_entry;
                        // std::cout <<"Put Key: " << curr_key << " in slot index: " << i << std::endl;
                        return;
                    } else if (m_table[i].ptr->key == curr_key) {
                        m_table[i].ptr->val = curr_val;
                        return;
                    }
                }

                int index = rand() % (2 * SLOTS_PER_BUCKET);
                HashPointer* temp_hash_pointer;
                if (0 <= index && index < SLOTS_PER_BUCKET)
                    temp_hash_pointer = &m_table[h1 + index];
                else
                    temp_hash_pointer = &m_table[h2 + index - SLOTS_PER_BUCKET];

                std::string temp_key = temp_hash_pointer->ptr->key;
                T temp_val = temp_hash_pointer->ptr->val;

                temp_hash_pointer->tag = tag;
                temp_hash_pointer->ptr->key = curr_key;
                temp_hash_pointer->ptr->val = curr_val;

                std::cout << "Swap " << curr_key << " with " << temp_key << std::endl;

                curr_key = temp_key;
                curr_val = temp_val;

            }

            std::cout << "Abort" << std::endl;
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
         //     HashPointer* hash_pointer = m_table[i];
         //     if (hash_pointer != NULL && tag == hash_pointer->tag) {
         //         HashEntry* hash_entry = hash_pointer->ptr;
         //         if (key == hash_entry->key) {
         //             delete hash_entry;
         //             delete hash_pointer;
         //             m_table[i] = NULL;
         //             return true;
         //         }
         //     }
         //    }

         //    // Look at the second bucket.
            // for (int i = (int)h2; i < (int)h2 + SLOTS_PER_BUCKET; i++) {
         //     HashPointer* hash_pointer = m_table[i];
         //     if (hash_pointer != NULL && tag == hash_pointer->tag) {
         //         HashEntry* hash_entry = hash_pointer->ptr;
         //         if (key == hash_entry->key) {
         //             delete hash_entry;
         //             delete hash_pointer;
         //             m_table[i] = NULL;
         //             return true;
         //         }
         //     }
         //    }
         //    return false;
        }

    private:
        struct HashEntry {
            std::string key;
            T val;
        };
        struct HashPointer {
            unsigned char tag;
            HashEntry* ptr;
        };
        HashPointer *m_table;
        int m_num_buckets;

        int m_tagmask = 0xff;

        unsigned char tag_hash(const uint32_t hv) {
            uint32_t r =  hv & m_tagmask;
            return (unsigned char) r + (r == 0);
        }

        // T search_for_key(bool delete_entry=false) {

        // }



};

#endif
