#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#define HEAP_CAPACITY 64000
#define CHUNK_LIST_CAPACITY 1024


typedef struct {
    char* start;
    size_t size;
}Heap_Chunck;

typedef struct {
    Heap_Chunck chuncks[CHUNK_LIST_CAPACITY];
    size_t count;
} Chunck_List;

char heap[HEAP_CAPACITY] = {0};
Chunck_List heap_alloced = {0};
Chunck_List heap_freed = {.count = 1,
.chuncks = {[0] = {.start = heap, .size = sizeof(heap)}}};
Chunck_List tmp_chuncks = {0};
size_t heap_alloced_size = 0;
size_t heap_freed_size = 0;

int chunck_start_compar(const void* a, const void* b) {
    const Heap_Chunck* a_chunck = a;
    const Heap_Chunck* b_chunck = b;
    return a_chunck->start - b_chunck->start;
}

int chunck_list_find(const Chunck_List* list, void* ptr) {

    for(size_t i = 0; i < list->count; i++) {
        if(list->chuncks[i].start == ptr)
            return (int)i;
    }
    return -1;
}

void chunck_list_insert(Chunck_List* list, void* ptr, size_t size) {
    assert(list->count < CHUNK_LIST_CAPACITY);

    list->chuncks[list->count].start = ptr;
    list->chuncks[list->count].size = size;

    for(size_t i = list->count; i > 0 && list->chuncks[i].start < list->chuncks[i - 1].start; i--) {
        const Heap_Chunck tmp =  list->chuncks[i];
        list->chuncks[i] = list->chuncks[i - 1];
        list->chuncks[i - 1] = tmp;
    }
    list->count++;
}

void chunck_list_remove(Chunck_List* list, size_t index) {
    assert(index < list->count);
    for(size_t i = index; i < list->count - 1; i++) {
        list->chuncks[i] = list->chuncks[i + 1];
    }
    list->count--;
}
void chunck_list_merge(const Chunck_List* list, Chunck_List* tmp) {
    tmp->count = 0;
    for(size_t i = 0; i < list->count; i++) {
        const Heap_Chunck chunck = list->chuncks[i];
        if(tmp->count > 0) {
            Heap_Chunck* top_chunck = &tmp->chuncks[tmp->count - 1];
            if(top_chunck->start + top_chunck->size == chunck.start) {
                top_chunck->size += chunck.size;
            }else {
                chunck_list_insert(tmp, chunck.start, chunck.size);
            }
        }else {
            chunck_list_insert(tmp, chunck.start, chunck.size);
        }
    }
}
void* heap_alloc(size_t size) {

    if(size > 0){
        chunck_list_merge(&heap_freed, &tmp_chuncks);
        heap_freed = tmp_chuncks;
        for(size_t i = 0; i < heap_freed.count; i++) {
            const Heap_Chunck chunck = heap_freed.chuncks[i];
            if(chunck.size >= size) {
                chunck_list_remove(&heap_freed, i);
                size_t tail_size = chunck.size - size;
                chunck_list_insert(&heap_alloced, chunck.start, size);
                if(tail_size > 0) {
                    chunck_list_insert(&heap_freed, chunck.start + size, tail_size);
                }
                return chunck.start;
            }
        }
    }
        return NULL;
}

void chunk_list_dump(const Chunck_List* list) {
    printf("Heap alloced chuncks: %zu", list->count);
    for(size_t i = 0; i < list->count; i++) {
        printf("start: %p, size: %zu ",list->chuncks[i].start, list->chuncks[i].size);
        puts("\n");
    }

}
void heap_free(void* ptr) {

    if(ptr != NULL) {
        const int index = chunck_list_find(&heap_alloced, ptr);
        assert(index >= 0);
        assert(ptr == heap_alloced.chuncks[index].start);
        chunck_list_insert(&heap_freed, heap_alloced.chuncks->start, heap_alloced.chuncks->size);
        chunck_list_remove(&heap_alloced, (size_t)index);
    }
}

int main() {

    for(int i = 0; i < 10; i++) {
        void* p = heap_alloc(i);
            heap_free(p);
    }
    chunk_list_dump(&heap_alloced);
    chunk_list_dump(&heap_freed);
    return 0;
}
