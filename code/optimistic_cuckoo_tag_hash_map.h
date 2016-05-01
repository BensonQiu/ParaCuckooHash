#ifndef OPTIMISTIC_CUCKOO_TAG_HASHMAP_H
#define OPTIMISTIC_CUCKOO_TAG_HASHMAP_H


template <typename T>
class OptimisticCuckooTagHashMap {

    public:
        const int SLOTS_PER_BUCKET = 4;
        const int MAX_ITERS = 500;
        const int NUM_KEY_VERSION_COUNTERS = 8192;

        OptimisticCuckooTagHashMap(int num_buckets=64);
        ~OptimisticCuckooTagHashMap();

        T get(std::string key);
        void put(std::string key, T val);

    private:
        struct HashEntry {
            std::string key;
            T val;
        };

        struct HashPointer {
            unsigned char tag;
            HashEntry* ptr;
        };

        // HashEntry **m_table;
        HashPointer *m_table;
        int m_num_buckets;
        uint32_t* m_key_version_counters;
        std::mutex m_write_mutex;
        int* m_visited_bitmap;

        int m_tagmask = 0xff;

        unsigned char tag_hash(const uint32_t hv);
};

#include "optimistic_cuckoo_tag_hash_map.cpp"

#endif
