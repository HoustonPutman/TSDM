#ifndef TSDM_METADATA_CACHE_H_
#define TSDM_METADATA_CACHE_H_

#include <stdint.h>
#include <unordered_map>

using namespace std;

template <typename data_t>
class TSDM_Metadata_Cache {
    private:
        unsigned long long max_size;
        unsigned long long size;

        // Linked List stuff
        struct node {
            data_t data;
            struct node* next;
            struct node* prev;
        };

        struct node* start;
        struct node* end;

        unordered_map< data_t, struct node* > data_map;

    public:
        TSDM_Metadata_Cache(unsigned long long max_size = 1) :
                max_size(max_size),
                size(0) {
            start = NULL;
            end = NULL;
        }

        void set_max(unsigned long long max_size);

        bool add(data_t data);

        bool remove(data_t data);

        bool contains(data_t data);
};

template <typename data_t>
bool TSDM_Metadata_Cache<data_t>::add(data_t data) {
    struct node* new_node = new(struct node);
    ++size;
    bool removed_node = false;

    new_node->data = data;
    new_node->prev = NULL;
    if (start == NULL) {
        new_node->next = NULL;
        start = new_node;
        end = new_node;
    }
    else {
        new_node->next = start;
        start->prev = new_node;
        start = new_node;
        remove(data);

        // Remove old data if cache is full
        if (size == max_size+1) {
            removed_node = true;
            --size;

            struct node* old_node = end;
            end = old_node->prev;
            end->next = NULL;
            data_map.erase(old_node->data);
            free(old_node);
        }
    }
    data_map[data] = new_node;
    return removed_node;
}

template <typename data_t>
bool TSDM_Metadata_Cache<data_t>::remove(data_t data) {
    auto data_pnt = data_map.find(data);
    if (data_pnt == data_map.end()) {
        return false;
    }
    struct node* remove_node = data_pnt->second;

    if (remove_node == start) {
        start = remove_node->next;
    }
    else {
        remove_node->prev->next = remove_node->next;
    }

    if (remove_node == end) {
        end = remove_node->prev;
    }
    else {
        remove_node->next->prev = remove_node->prev;
    }

    free(remove_node);
    data_map.erase(data_pnt);
    --size;

    return true;
}

template <typename data_t>
bool TSDM_Metadata_Cache<data_t>::contains(data_t data) {
    return data_map.count(data) == 1;
}

template <typename data_t>
void TSDM_Metadata_Cache<data_t>::set_max(unsigned long long max_size) {
    this->max_size = max_size;
}
#endif // __TSDM_METADATA_CACHE_H
