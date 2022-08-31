#include "pparser.h"
#include "kernel.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "config.h"

static int pathparser_path_valid_format(const char* filename) {
    int len = strnlen(filename, PEACHOS_MAX_PATH);

    // like "0:/..."
    return (len >= 3 && 
            is_digit(filename[0]) && 
            memcmp((void*)&filename[1], ":/", 2) == 0);
}

static int pathparser_get_drive_from_path(const char** path) {
    if (!pathparser_path_valid_format(*path)) {
        return -ERROR_BAD_PATH;
    }

    int drive_no = to_numeric_digit(*path[0]);

    // Add 3 bytes to skip drive number (0:/)
    *path += 3;
    return drive_no;
}

static struct path_root* pathparser_create_root(int drive_number) {
    struct path_root* path_r = kzalloc(sizeof(struct path_root));
    path_r->drive_no = drive_number;
    path_r->first = 0;

    return path_r;
}

/* Returns the first path part (stops at '/' or '/0') 
 *path is incremented and will point to the next part */
static const char* pathparser_get_path_part(const char** path) {
    char* result_path_part = kzalloc(PEACHOS_MAX_PATH);
    int i = 0;

    while (**path != '/' && **path != 0x00) {
        result_path_part[i] = **path;
        *path += 1;
        i++;
    } 

    if (**path == '/') {
        *path += 1;
    }

    if (i==0) {
        kfree(result_path_part);
        result_path_part = 0;
    }

    return result_path_part;
}

// Create a struct path_part which will be pointed by last_part (When it's not NULL) and will contain the first path part from path
static struct path_part* pathparser_parse_path_part(struct path_part* last_part, const char** path) {
    const char* path_part_str = pathparser_get_path_part(path);
    if (!path_part_str) {
        return 0;
    }

    struct path_part* path_part = kzalloc(sizeof(struct path_part));
    path_part->part = path_part_str;
    path_part->next = 0x00;
    
    if (last_part) {
        last_part->next = path_part;
    }
    
    return path_part;
}

void pathparser_free(struct path_root* path_root) {
    struct path_part* part = path_root->first;
    while (part) {
        struct path_part* next_part = part->next;
        kfree((void*) part->part);
        kfree((void*) part);
        part = next_part;
    }

    kfree(path_root);
}

struct path_root* pathparser_parse(const char* path, const char* current_dir_path) {
    int res = 0;
    const char* tmp_path = path;
    struct path_root* path_root = 0;

    if (strlen(path) > PEACHOS_MAX_PATH) {
        goto out;
    }

    // get drive number and advance the pointer after "0:/"
    res = pathparser_get_drive_from_path(&tmp_path);
    if (res < 0) {
        goto out;
    }

    path_root = pathparser_create_root(res);
    if (!path_root) {
        goto out;
    }

    struct path_part* first_part = pathparser_parse_path_part(NULL, &tmp_path);
    if (!first_part) {
        goto out;
    }

    path_root->first = first_part;

    struct path_part* part = pathparser_parse_path_part(first_part, &tmp_path);
    while (part) {
        part = pathparser_parse_path_part(part, &tmp_path);
    }

out:
    return path_root;

}