#ifndef FILE_H
#define FILE_H

#include "pparser.h"
typedef unsigned int FILE_SEEK_MODE;

enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;

enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

struct disk;
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int(*FS_RESOLVE_FUNCTION)(struct disk* disk);


struct filesystem
{
    // FS should return 0 from resolve if the provided disk is using its fs
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION    open;

    char name[20];
};


struct file_descriptor
{
    int index;
    struct filesystem* filesystem;

    // Private data for internal file descriptor (each FS can cast this to its relevant data)
    void* private;

    // THe disk that the fd should be used on
    struct disk* disk;
};

void fs_init();
int fopen(const char* filename, const char* mode_str);
void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);
#endif