#ifndef BETTER_CUCKOO_HASHMAP_H
#define BETTER_CUCKOO_HASHMAP_H


template <typename T>
class BetterCuckooHashMap {

    public:
        const int SLOTS_PER_BUCKET = 4;
        const int MAX_ITERS = 500;

        BetterCuckooHashMap(int num_buckets=64);
        ~BetterCuckooHashMap();

        T get(std::string key);
        void put(std::string key, T val);
        bool remove(std::string key);

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

        unsigned char tag_hash(const uint32_t hv);

};

#include "better_cuckoo_hash_map.cpp"

#endif
