#include "streamer.h"
#include "memory/heap/kheap.h"
#include "config.h"

#include <stdbool.h>

struct disk_stream* disk_streamer_new(int disk_id) {
    struct disk* disk = disk_get(disk_id);
    if (!disk) {
        return 0;
    }
    struct disk_stream* stream = kzalloc(sizeof(struct disk_stream));
    stream->pos = 0;
    stream->disk = disk;
    return stream;
}

int disk_streamer_seek(struct disk_stream* stream, int pos) {
    stream->pos = pos;
    return 0;
}

int disk_stremer_read(struct disk_stream* stream, void* out, int total) {
    int sector = stream->pos / PEACHOS_SECTOR_SIZE;
    int offset = stream->pos % PEACHOS_SECTOR_SIZE;
    char buf[PEACHOS_SECTOR_SIZE];

    int res = disk_read_block(stream->disk, sector, 1, buf);
    if (res < 0) {
        goto out;
    }

    int bytes_to_read_in_sector = PEACHOS_SECTOR_SIZE - offset;
    int total_to_read = (total > bytes_to_read_in_sector) ? (bytes_to_read_in_sector) : total;
    for (int i = 0; i < total_to_read; i++)
    {
        *(char*)out++ = buf[offset + i];
    }

    // Adjust the stream
    stream->pos += total_to_read;
    if (total > bytes_to_read_in_sector) {
        res = disk_stremer_read(stream, out, total - bytes_to_read_in_sector);
    }

out:
    return res;
}

void disk_streamer_close(struct disk_stream* stream) {
    kfree(stream);
}