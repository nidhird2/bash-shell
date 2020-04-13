/**
 * Finding Filesystems
 * CS 241 - Spring 2020
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//ssize_t minixfs_write_inode(file_system*, inode*, const void*, size_t, off_t*);
/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string, "Free blocks: %zd\n"
                            "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    inode* current = get_inode(fs, path);
    // if path/file DNE
    if(current == NULL){
        errno = ENOENT;
        return -1;
    }
    int type = current->mode >> RWX_BITS_NUMBER;
    current->mode = (type << RWX_BITS_NUMBER) | new_permissions;

    //update time
    clock_gettime(CLOCK_REALTIME, &current->ctim);
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* current = get_inode(fs, path);
    // if path/file DNE
    if(current == NULL){
        errno = ENOENT;
        return -1;
    }
    if(owner != (uid_t) - 1){
        current->uid = owner;
    }
    if(group != (uid_t) - 1){
        current->gid = group;
    }
    clock_gettime(CLOCK_REALTIME, &current->ctim);
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    //if inode already exists
    //printf("create path: %s\n", path);
    if(get_inode(fs, path) != NULL){
        //printf("inode already exits!\n");
        return NULL;
    }
    const char* filename;
    //if parent dir DNE
    inode* parent = parent_directory(fs, path, &filename);
    if(parent == NULL){
        //printf("parent DNE\n");
        return NULL;
    }
    if(!valid_filename(filename)){
        //printf("invalid filename\n");
        return NULL;
    }
    inode_number new_num = first_unused_inode(fs);
    inode* new = fs->inode_root + new_num;
    init_inode(parent, new);
    //char* dirstr = (char*)malloc(FILE_NAME_ENTRY);
    //make_string_from_dirent(dirstr, loc);
    minixfs_dirent loc;
    loc.name = strdup(filename);
    loc.inode_num = new_num;
    char* use = malloc(FILE_NAME_ENTRY);
    make_string_from_dirent(use, loc);
    // printf("use: %s\n", use);
    // printf("use len: %lu\n", strlen(use));

    //find parent path
    size_t ppath_len = strlen(path) - strlen(filename) + 1;
    char* parent_path = (char*)malloc(ppath_len);
    memcpy(parent_path, path, ppath_len - 1);
    parent_path[ppath_len - 1] = '\0';
    // printf("filename: .%s.\n", filename);
    // printf("path: .%s.\n", path);
    // printf("parent path: .%s.\n", parent_path);

    off_t s = (long)parent->size;
    minixfs_write(fs, parent_path, use, FILE_NAME_ENTRY, &s);
    free(use);
    free(loc.name);
    return new;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        //mchar result[300];
        unsigned long used = 0;
        char* map = GET_DATA_MAP(fs->meta);
        for(uint64_t i=0; i<fs->meta->dblock_count;i++){
          if(map[i]==1){
            used++;
          }
        }
        char *info = block_info_string(used); 
        if((unsigned long)*off > strlen(info)){
            return 0;
        }
        size_t amount_copy = count;
        //if trying to copy past string, only copy til end of string
        if((unsigned long)*off + count > strlen(info)){
            amount_copy = strlen(info) - *off;
        }
        memcpy(buf, info + *off, amount_copy);
        free(info);
        *off += amount_copy;
        return amount_copy;
    }
    // TODO implement your own virtual file here
    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    inode* current = get_inode(fs, path);
    //printf("current inode: %p\n", current);
    //printf("write path: %s\n", path);
    if(current == NULL){
        current = minixfs_create_inode_for_path(fs, path);
    }

    size_t blocks_needed = ((count + *off) / sizeof(data_block)) + 1;
    //filesize cannot handle req
    if(blocks_needed > (NUM_DIRECT_BLOCKS+NUM_INDIRECT_BLOCKS)){
        errno = ENOSPC;
        return -1;
    }
    //cannot allocate enough blocks
    if(minixfs_min_blockcount(fs, path, blocks_needed) == -1){
        errno = ENOSPC;
        //printf("cannot allocate blocks!\n");
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &(current->atim));
    clock_gettime(CLOCK_REALTIME, &(current->mtim));
    size_t idx = *off / sizeof(data_block);
    size_t buf_offset = 0;
    size_t buf_left = count;
    size_t data_offset =  *off % sizeof(data_block);;
    data_block_number a;
    size_t amount_to_copy = 0;
    for(size_t i = idx; i < NUM_DIRECT_BLOCKS; i++){
        a = current->direct[i];
        char* p = fs->data_root[a].data + data_offset;
        // if buffer end occurs in this block
        if(data_offset + buf_left < sizeof(data_block)){
            amount_to_copy = buf_left;
        }
        // else copy rest of block
        else{
            amount_to_copy = sizeof(data_block) - data_offset;
        }
        //char* p = temp.data + data_offset;
        //printf("p: %p\n", p);
        memcpy(p, buf + buf_offset, amount_to_copy);
        buf_offset += amount_to_copy;
        buf_left -= amount_to_copy;
        //check if done writing
        if(buf_left == 0){
            //update new size!
            // printf("*off: %lu", (unsigned long)*off);
            // printf("count: %lu", count);
            // unsigned long sum = *off + count;
            // printf("sum: %lu", sum);
            // printf("sum not casted: %lu", (*off + count));
            if(current->size < (*off + count)){
                current->size = *off + count;
            }
            // printf("new size: %lu\n", current->size);
            *off += count;
            return count;
        }
        data_offset = 0;
    }
    int indirect_idx = 0;
    if(idx >= NUM_DIRECT_BLOCKS){
        indirect_idx = idx - NUM_DIRECT_BLOCKS;
    }
    for(size_t i = indirect_idx; i < NUM_INDIRECT_BLOCKS; i++){
        data_block* ind = &(fs->data_root[current->indirect]);
        data_block_number* ref_table = (data_block_number*)ind;
        data_block_number a = ref_table[i];
        char* p = fs->data_root[a].data + data_offset;
        // if end occurs in this block
        if(data_offset + buf_left < sizeof(data_block)){
            amount_to_copy = buf_left;
        }
        // else copy rest of block
        else{
            amount_to_copy = sizeof(data_block) - data_offset;
        }
        //printf("buf: %s\n", buf + buf_offset);
        memcpy(p, buf + buf_offset, amount_to_copy);
        buf_offset += amount_to_copy;
        buf_left -= amount_to_copy;
        //check if done writing
        if(buf_left == 0){
            //update new size!
            if(current->size < (*off + count)){
                    current->size = *off + count;
            }
            *off += count;
            return count;
        }
        data_offset = 0;
    }
    //printf("yikes idk\n");
    return -1;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    inode* current = get_inode(fs, path);
    //path DNE
    if(current == NULL){
        errno = ENOENT;
        //printf("Can't find the file!\n");
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &current->atim);
    //off is greater than end of file
    if((unsigned long)*off >= current->size){
        return 0;
    }
    //printf("current size: %lu\n", current->size);
    size_t amount_read = 0;
    size_t bytes_left_to_read = current->size;
    size_t buff_off = 0;
    size_t buff_left = count;
    size_t data_offset = *off % (16 * KILOBYTE);
    size_t idx = *off / (16 * KILOBYTE);
    size_t amount_to_copy = 0;
    data_block_number a;
    // printf("read path: %s\n", path);
    // printf("bytes left to read: %lu\n", bytes_left_to_read);
    // printf("offset: %lu\n", (unsigned long)*off);
    for(size_t i = idx; i < NUM_DIRECT_BLOCKS; i++){
        a = current->direct[i];
        char* p = fs->data_root[a].data + data_offset;
        //temp = fs->data_root[a];
        //if not the last block, copy til end of block
        if((bytes_left_to_read > sizeof(data_block)) && (buff_left > sizeof(data_block))){
            amount_to_copy = sizeof(data_block) - data_offset;
        }
        //EOF occurs before desired count
        else if (bytes_left_to_read <= (data_offset + buff_left)){
            amount_to_copy = bytes_left_to_read - data_offset;
        }
        //desired count occurs in block
        else{
            amount_to_copy = buff_left;
        }
        //char* p = temp.data + data_offset;
        //printf("p: %p\n", p);
        memcpy(buf + buff_off, p, amount_to_copy);
        amount_read += (amount_to_copy + data_offset);
        bytes_left_to_read -= (amount_to_copy + data_offset);
        buff_off += amount_to_copy;
        buff_left -= amount_to_copy;
        //if you reached EOF or copied over count bytes
        if(bytes_left_to_read == 0 ||buff_left == 0){
            *off = amount_read;
            return amount_read - *off;
        }
        data_offset = 0;
    }
    int indirect_idx = 0;
    if(idx >= NUM_DIRECT_BLOCKS){
        indirect_idx = idx - NUM_DIRECT_BLOCKS;
    }
    //go through indirect blocks
    for(size_t i = indirect_idx; i < NUM_INDIRECT_BLOCKS; i++){
        data_block* ind = &(fs->data_root[current->indirect]);
        data_block_number* ref_table = (data_block_number*)ind;
        data_block_number a = ref_table[i];
        char* p = fs->data_root[a].data + data_offset;
        //if not the last block, copy til end of block
        if((bytes_left_to_read > sizeof(data_block)) && (buff_left > sizeof(data_block))){
            amount_to_copy = sizeof(data_block) - data_offset;
        }
        //EOF occurs before desired count
        else if (bytes_left_to_read <= (data_offset + buff_left)){
            amount_to_copy = bytes_left_to_read - data_offset;
        }
        //desired count occurs in block
        else{
            amount_to_copy = buff_left;
        }
        memcpy(buf + buff_off, p, amount_to_copy);
        amount_read += (amount_to_copy + data_offset);
        bytes_left_to_read -= (amount_to_copy + data_offset);
        buff_off += amount_to_copy;
        buff_left -= amount_to_copy;
        //if you reached EOF or copied over count bytes
        if(bytes_left_to_read == 0 || buff_left == 0){
            *off = amount_read;
            return amount_read - *off;
        }
        data_offset = 0;
    }
    printf("YIKES, HERE!");
    return -1;
}

// ssize_t minixfs_write_inode(file_system* fs, inode* current, const void* buf, 
//                             size_t count, off_t *off){
//     return -1;
// }

// ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
//                       size_t count, off_t *off)
