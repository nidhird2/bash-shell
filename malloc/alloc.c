/**
 * Malloc
 * CS 241 - Spring 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

typedef struct _metadata_entry_t{
    size_t size;
    unsigned int free : 1;
    struct _metadata_entry_t* next;
    struct _metadata_entry_t* prev;
    char data[0];
} meta_t;

typedef struct _boundary_tag{
    size_t size;
} btag;

void* add_more_malloc_space(size_t);

static void* limit = NULL;
static void* lower_limit = NULL;
static meta_t* head_available = NULL;
static int first = 1;
static size_t bytes_malloced = 0;

//change to alter behavior:
static size_t split_threshold = 50;
static size_t coalesce_threshold = 100;
static size_t double_threshold = 1;


/**
 * Returns ptr to last space in meta_t, none
*/
meta_t* get_last_meta(){
    if(first){
        return NULL;
    }
    btag* last_tag = ((btag*)lower_limit) - 1;
    //(meta_t*)(((char*)prev_tag) - (prev_tag->size) - sizeof(meta_t)); 
    meta_t* last_created =  (meta_t*)(((char*)last_tag) - (last_tag->size) - sizeof(meta_t)); 
    return last_created;
}


void remove_free(meta_t* to_remove){
    if(to_remove->free == 0){
        return;
    }
    if(to_remove->prev != NULL){
        to_remove->prev->next = to_remove->next;
    } else {
        head_available = to_remove->next;
    }
    if(to_remove->next != NULL){
        to_remove->next->prev = to_remove->prev;
    }
    to_remove->next = NULL;
    to_remove->prev = NULL;
    to_remove->free = 0;
    return;
}

void coalesce_two(meta_t* prev, meta_t* current){
    if(prev->data + prev->size +sizeof(btag) != (char*)current){
        return;
    }
    if(prev->free != 1 || current->free != 1){
        return;
    }
    prev->size += current->size + sizeof(meta_t) + sizeof(btag);
    btag* tag = (btag*)(prev->data + prev->size); 
    tag->size = prev->size;
    remove_free(current);
    //prev->next = NULL;
    //prev->prev = NULL;
    return;
}

void coalesce_me(meta_t* me){
    //printf("me: %p\n", me);
    //printf("meta_t size: %lu\n", sizeof(meta_t));
    //printf("ptr: %p\n", (void*)(me + 1));
    btag* prev_tag;
    meta_t* next = (meta_t*)(me->data + me->size +sizeof(btag));
    //printf("next: %p\n", next);
    if(((void*)next) >= lower_limit){
        next = NULL;
    }
    if((void*)me <= limit){
        prev_tag = NULL;
    }else {
        prev_tag = ((void*)me)- sizeof(btag);
    }
    if(prev_tag == NULL && next == NULL){
        return;
    }
    if(prev_tag == NULL){
        if(next->free == 1){
            return coalesce_two(me, next);
        }
        return;
    }
    meta_t* prev = (meta_t*)(((char*)prev_tag) - (prev_tag->size) - sizeof(meta_t)); 
    if(next == NULL){
        if(prev->free == 1){
            return coalesce_two(prev, me);
        }
        return;
    }
    //neither are free
    if(prev -> free == 0 && next->free == 0){
        return;
    }
    //only next is free
    if(prev->free == 0 && next->free == 1){
        return coalesce_two(me, next);
    }
    //only prev is free
    else if(prev->free == 1 && next->free == 0){
        return coalesce_two(prev, me);
    }
    else{
        coalesce_two(me, next);
        return coalesce_two(prev,me);
    }
}

void split_me(meta_t* me, size_t size){
    size_t current_size = me->size;
    if(current_size <= size){
        return;
    }
    if(current_size < size + sizeof(meta_t) + sizeof(btag) + split_threshold){
        return;
    }
    size_t new_size = (me->size) - size - sizeof(meta_t) - sizeof(btag);
    me->size = size;
    btag* me_tag = (btag*)(me->data + me->size);
    me_tag->size = size;

    meta_t* split = ((void*)me_tag) + sizeof(btag);
    split->free=1;
    split->size= new_size;
    btag* split_tag = (btag*)(split->data + new_size);
    split_tag->size= split->size;
    split->prev = NULL;
    split->next= head_available;
    if(head_available){
        head_available->prev = split;
    }
    head_available = split;
}

void* get_double_new_space(size_t size){
    bytes_malloced += (2*size);
    meta_t* chosen = sbrk(0);
    if(sbrk((size + sizeof(meta_t) + sizeof(btag))*2) == (void*)-1){
        return NULL;
    }
    chosen->size = size;
    chosen->free = 0;
    chosen->next = NULL;
    chosen->prev= NULL;
    btag* chosen_tag = (btag*)(chosen->data+ size);
    chosen_tag->size = size;
    meta_t* new_entry = ((void*)chosen_tag) + sizeof(btag);
    new_entry->size = size;
    new_entry->free = 1;
    new_entry->next = head_available;
    new_entry->prev = NULL;
    btag* new_tag = ((void*)(new_entry+ 1)) + size;
    new_tag->size = size;
    head_available = new_entry;

    if(first){
        first = 0;
        limit = chosen;
    }
    lower_limit = (void*)(chosen_tag+1);
    return chosen->data;
}

void* get_new_space(size_t size){
    if(size <= double_threshold){
        return get_double_new_space(size);
    }
    bytes_malloced += (1*size);
    //fprintf(output, "making space\n");
    meta_t* chosen = sbrk(0);
    //sbrk didn't give us more space :(
    if(sbrk(size + sizeof(meta_t) + sizeof(btag)) == (void*)-1){
        return NULL;
    }
    chosen->size = size;
    chosen->free = 0;
    chosen->next = NULL;
    chosen->prev= NULL;
    btag* chosen_tag = (btag*)(chosen->data+ size);
    chosen_tag->size = size;
    // meta_t* new_entry = ((void*)chosen_tag) + sizeof(btag);
    // new_entry->size = size;
    // new_entry->free = 1;
    // new_entry->next = head_available;
    // btag* new_tag = ((void*)(new_entry+ 1)) + size;
    // new_tag->size = size;
    // head_available = new_entry;
    if(first){
        first = 0;
        limit = chosen;
    }
    lower_limit = (void*)(chosen_tag+1);
    //head_used = chosen;
    return chosen->data;
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    void* res = malloc(num*size);
    if(res == NULL){
        return res;
    }
    memset(res, 0, num * size);
    return res;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */

void *malloc(size_t size) {
    // implement malloc!
    // if(output == NULL){
    //     output = fopen("result.txt", "a");
    //     fprintf(output, "making space\n");
    // }
    meta_t* p = head_available;
    meta_t* chosen = NULL;
    //printf("head available: %p\n", head_available);
    
    while(p != NULL){
        if(p->free && p->size >= size){
            if(chosen == NULL || p->size < chosen->size){
                chosen =  p;
                //break;
            }
            if(p->size == chosen->size){
                break;
            }
        }
        p = p->next;
    }
    if(chosen != NULL){
        //check for block splitting
        if(chosen->size > size){
            split_me(chosen, size);
        }
        remove_free(chosen);
        //remove_frees(chosen);
        return (void*)(chosen->data);
    }
    void* res = add_more_malloc_space(size);
    if(res == NULL){
        //no space found: make space!!
        return get_new_space(size);
    }
    return res;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if(ptr == NULL){
        return;
    }
    meta_t* target = ptr - sizeof(meta_t);
    // meta_t* current = head_used;
    // meta_t* current_prev = NULL;
    // while(current != NULL){
    //     if(target == current){
    //         // if(current_prev != NULL){
    //         //     current_prev->next = target->next;
    //         // } else{
    //         //     head_used = target->next;
    //         // }
    //         target->free = 1;
    //         target->next = head_available;
    //         head_available = target;
    //         coalesce_me(ptr);
    //         return;
    //     }
    //     current_prev = current;
    //     current = current->next;
    // }

    target->free = 1;
    if(head_available != NULL){
        head_available->prev = target;
    }
    target->next = head_available;
    target->prev = NULL;
    head_available = target;
    if(bytes_malloced >= coalesce_threshold){
        coalesce_me(target);
    }
    return;
}

// void* find_used_match(void *ptr){
//     meta_t* target = (meta_t*)ptr - 1;
//     meta_t* current = head_used;
//     while(current != NULL){
//         if(current == target){
//             return target;
//         }
//     }
//     return NULL;
// }

void* check_right_space(meta_t* me, size_t target){
    meta_t* next = (meta_t*)(me->data + me->size +sizeof(btag));
    // printf("next: %p\n", next);
    // printf("next ptr: %p\n", ((void*)(next + 1)));
    if(((void*)next) >= lower_limit){
        return NULL;
    }
    // printf("checking right space...\n");
    // printf("ptr: %p\n", ((void*)(me + 1)));
    // printf("next: %p\n", next);
    // printf("next ptr: %p\n", ((void*)(next + 1)));

    if(next->free == 0){
        return NULL;
    }
    size_t new_size = me->size + sizeof(meta_t) + sizeof(btag) + next->size;
    //printf("new size: %lu\n", new_size);
    //printf("target size: %lu\n", target);
    if(new_size < target){
        return NULL;
    }
    // printf("attempts to use neighbor\n");
    me->size += sizeof(meta_t) + sizeof(btag) + next->size;
    btag* me_tag = (btag*)(me->data + me->size);
    me_tag->size = me->size;
    remove_free(next);
    split_me(me, target);
    return me->data; 
}

void* add_more_malloc_space(size_t target){
    meta_t* last_created = get_last_meta();
    if(last_created == NULL){
        return NULL;
    }
    if(last_created->free == 0){
        return NULL;
    }
    size_t needed = target - last_created->size;
    if(sbrk(needed) == ((void*)-1)){
        return NULL;
    }
    last_created->size = target;
    btag* last_tag = (btag*)(last_created->data + target);
    last_tag->size = target;
    lower_limit = (void*)(last_tag+1);
    remove_free(last_created);
    return last_created->data;
}

void* add_more_realloc_space(meta_t* me, size_t target){
    meta_t* next = (meta_t*)(me->data + me->size +sizeof(btag));
    //printf("next: %p\n", next);
    if(((void*)next) < lower_limit){
        return NULL;
    }
    size_t needed = target - me->size;
    if(sbrk(needed) == ((void*)-1)){
        return NULL;
    }
    me->size = target;
    btag* me_tag = (btag*)(me->data + target);
    me_tag->size = target;
    lower_limit = (void*)(me_tag+1);
    return me->data;   
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if(ptr == NULL){
        return malloc(size);
    }
    if(size == 0){
        free(ptr);
        return NULL;
    }
    meta_t * met = ((meta_t*)ptr)-1;
    //printf("met size: %lu\n", met->size);
    //printf("ptr: %p\n", ((void*)(met + 1)));
    //meta_t* met = find_used_match(ptr); 
    //if you already enough space in current ptr
    // if(find_used_match(ptr) == NULL){
    //     return malloc(size);
    // }
    if(met->size >= size){
        //split_me(met, size);
        return ptr;
    }
    //void* res = NULL;
    //printf("ptr:%p\n", ptr);
    //void* res = NULL;
    void* res = check_right_space(met, size);
    if(res != NULL){
        return res;
    }
    res = add_more_realloc_space(met, size);
    if(res != NULL){
        split_me(met, size);
        return res;
    }
    res = malloc(size);
    memcpy(res,ptr,(int)fmin(size, met->size));
    free(ptr);
    return res;
}
