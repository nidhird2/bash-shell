/**
 * Finding Filesystems
 * CS 241 - Spring 2020
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

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
    if(current!= NULL){
        errno = ENOENT;
        return -1;
    }
    current->mode |= new_permissions << RWX_BITS_NUMBER;

    //update time
    clock_gettime(CLOCK_REALTIME, &current->ctim);
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* current = get_inode(fs, path);
    // if path/file DNE
    if(current!= NULL){
        errno = ENOENT;
        return -1;
    }
    if(owner != (current->uid - 1)){
        current->uid = owner;
    }
    if(group != (current->gid - 1)){
        current->gid = group;
    }
    clock_gettime(CLOCK_REALTIME, &current->ctim);
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    //if inode already exists
    if(get_inode(fs, path) != NULL){
        return NULL;
    }
    const char* filename;
    inode* parent = parent_directory(fs, path, &filename);
    //if not valid filename or parent dir DNE
    if(!valid_filename(filename) || parent == NULL){
        return NULL;
    }
    inode* new = fs->inode_root + first_unused_inode(fs);
    //char* dirstr = (char*)malloc(FILE_NAME_ENTRY);
    minixfs_dirent loc;
    loc.inode_num = 1;
    loc.name = strdup(filename);
    //make_string_from_dirent(dirstr, loc);
    init_inode(parent, new);
    unsigned long temp = parent->size / sizeof(data_block_number);
    size_t offset = parent->size - (temp * sizeof(data_block_number));
    minixfs_min_blockcount(fs, path, FILE_NAME_ENTRY / sizeof(data_block));

    char* use;

    if(temp < NUM_DIRECT_BLOCKS){
        use = fs->data_root[parent->direct[temp]].data + offset;
    }
    else if (temp < NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS){
        temp -= NUM_DIRECT_BLOCKS;
        data_block ind = fs->data_root[parent->indirect];
        data_block_number* ref = (data_block_number*)(&ind) + temp;
        use = fs->data_root[parent->direct[*ref]].data + offset;
    }
    else{
        return NULL;
    }
    make_string_from_dirent(use, loc);
    return new;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
    }
    // TODO implement your own virtual file here
    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    inode* current = get_inode(fs, path);
    if(current == NULL){
        minixfs_create_inode_for_path(fs, path);
        current = get_inode(fs, path);
    }
    //filesize cannot handle req
    if(current->size + count + *off > ((NUM_DIRECT_BLOCKS+NUM_INDIRECT_BLOCKS) * sizeof(data_block))){
        errno = ENOSPC;
        return -1;
    }
    //off is greater than end of file
    if((unsigned long)*off >= current->size){
        return 0;
    }
    size_t need = count + *off + current->size;
    int blocks_needed = need / (sizeof(data_block));
    if(minixfs_min_blockcount(fs, path, blocks_needed) == -1){
        errno = ENOSPC;
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &current->atim);
    clock_gettime(CLOCK_REALTIME, &current->mtim);
    size_t idx = *off / (16 * KILOBYTE);
    size_t buf_offset = 0;
    size_t buf_left = count;
    size_t ind_off = 0;
    data_block temp;
    //update new size
    if(*off + count > current->size){
        current->size = *off + count;
    }
    if(idx < NUM_DIRECT_BLOCKS){
        temp = fs->data_root[current->direct[idx]];
    }
    else{
        ind_off = idx - NUM_DIRECT_BLOCKS;
        data_block ind = fs->data_root[current->indirect];
        data_block_number* ref = (data_block_number*)(&ind) + ind_off;
        temp = fs->data_root[current->direct[*ref]];
        ind_off++;
    }
    if(*off + count < (16 * KILOBYTE)){
        memcpy(temp.data + *off, buf, count);
        *off += count;
        return count;
    }
    size_t amount_copy = (16 * KILOBYTE) - *off;
    memcpy(temp.data + *off, buf, amount_copy);
    buf_offset += amount_copy;
    buf_left -= amount_copy;

    if(idx < NUM_DIRECT_BLOCKS){
        for(int i = idx + 1; i < NUM_DIRECT_BLOCKS; i++){
            amount_copy = (16 * KILOBYTE);
            if(buf_left < amount_copy){
                amount_copy = buf_left;
            }
            memcpy(temp.data, buf, amount_copy);
            buf_offset += amount_copy;
            buf_left -= amount_copy;
            if(buf_left == 0){
                *off += count;
                return count;
            }
        }
    }
    data_block ind = fs->data_root[current->indirect];
    for(size_t i = ind_off; i < NUM_INDIRECT_BLOCKS; i++){
        data_block_number* ref = (data_block_number*)(&ind) + ind_off;
        temp = fs->data_root[current->direct[*ref]];
        amount_copy = (16 * KILOBYTE);
        if(buf_left < amount_copy){
            amount_copy = buf_left;
        }
        memcpy(temp.data, buf, amount_copy);
        buf_offset += amount_copy;
        buf_left -= amount_copy;
        if(buf_left == 0){
            *off += count;
            return count;
        }
    }

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
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &current->atim);
    //off is greater than end of file
    if((unsigned long)*off >= current->size){
        return 0;
    }
    size_t amount_read = 0;
    size_t bytes_left_to_read = current->size;
    size_t buff_off = 0;
    size_t buff_left = count;

    //go through all direct blocks
    for(int block_i = 0; block_i < NUM_DIRECT_BLOCKS; block_i++){
        data_block temp = fs->data_root[current->direct[block_i]];
        //offset is not in this block
        if((unsigned long)*off > (amount_read + (16 * KILOBYTE))){
            amount_read += (16 * KILOBYTE);
            bytes_left_to_read -= (16 * KILOBYTE);
        }
        //offset was in previous block
        else if(buff_off != 0){ 
            size_t amount_to_copy;
            if(buff_left > (16 * KILOBYTE) && bytes_left_to_read >(16 * KILOBYTE)){
                amount_to_copy = (16 * KILOBYTE);
            }
            //check if end occurs in this block
            else{
                if(buff_left < bytes_left_to_read){
                    amount_to_copy = buff_left;
                }
                else{
                    amount_to_copy = bytes_left_to_read;
                }
            }
            memcpy(buf + buff_off, temp.data, amount_to_copy);
            amount_read += amount_to_copy;
            bytes_left_to_read -= amount_to_copy;
            buff_off += amount_to_copy;
            buff_left -= amount_to_copy;
        }
        //offset is in this block
        else{
            size_t amount_to_copy;
            //EOF reached!
            if (bytes_left_to_read <= (*off +count)){
                amount_to_copy = bytes_left_to_read - *off;
            }
            //count is also in this block!
            else if(*off + count < (16*KILOBYTE)){
                amount_to_copy = count;
            }
            //copy rest of block
            else{
                amount_to_copy = (16 * KILOBYTE) - *off;
            }
            memcpy(buf + buff_off, temp.data + *off, amount_to_copy);
            amount_read += (amount_to_copy + *off);
            bytes_left_to_read -= (amount_to_copy + *off);
            buff_off += amount_to_copy;
            buff_left -= amount_to_copy;
        }
        //if you reached EOF or copied over count bytes
        if(bytes_left_to_read == 0 ||buff_left == 0){
            *off = amount_read;
            return buff_off;
        }
    }
    //go through indirect blocks
    data_block ind = fs->data_root[current->indirect];
    for(size_t i = 0; i < NUM_INDIRECT_BLOCKS; i++){
        data_block_number* ref = (data_block_number*)(&ind) + i;
        data_block temp = fs->data_root[current->direct[*ref]];

        //offset is not in this block
        if((unsigned long)*off > (amount_read + (16 * KILOBYTE))){
            amount_read += (16 * KILOBYTE);
            bytes_left_to_read -= (16 * KILOBYTE);
        }
        //offset was in previous block
        else if(buff_off != 0){ 
            size_t amount_to_copy;
            if(buff_left > (16 * KILOBYTE) && bytes_left_to_read >(16 * KILOBYTE)){
                amount_to_copy = (16 * KILOBYTE);
            }
            //check if end occurs in this block
            else{
                if(buff_left < bytes_left_to_read){
                    amount_to_copy = buff_left;
                }
                else{
                    amount_to_copy = bytes_left_to_read;
                }
            }
            memcpy(buf + buff_off, temp.data, amount_to_copy);
            amount_read += amount_to_copy;
            bytes_left_to_read -= amount_to_copy;
            buff_off += amount_to_copy;
            buff_left -= amount_to_copy;
        }
        //offset is in this block
        else{
            size_t amount_to_copy;
            //EOF reached!
            if (bytes_left_to_read <= (*off +count)){
                amount_to_copy = bytes_left_to_read - *off;
            }
            //count is also in this block!
            else if(*off + count < (16*KILOBYTE)){
                amount_to_copy = count;
            }
            //copy rest of block
            else{
                amount_to_copy = (16 * KILOBYTE) - *off;
            }
            memcpy(buf + buff_off, temp.data + *off, amount_to_copy);
            amount_read += (amount_to_copy + *off);
            bytes_left_to_read -= (amount_to_copy + *off);
            buff_off += amount_to_copy;
            buff_left -= amount_to_copy;
        }
        //if you reached EOF or copied over count bytes
        if(bytes_left_to_read == 0 ||buff_left == 0){
            *off = amount_read;
            return buff_off;
        }
    }
    return -1;
}
