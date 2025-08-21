#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ruby-comm-channel.h"

// ==== Communication Channel Implementation ====
int create_comm_channel(CommChannel* channel) {
    if (!channel) return -1;
    
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        return -1;
    }
    channel->read_fd = sv[0];
    channel->write_fd = sv[1];

    return 0;
}

void close_comm_channel(CommChannel* channel) {
    if (!channel) return;
    
    if (channel->read_fd >= 0) {
        close(channel->read_fd);
        channel->read_fd = -1;
    }
    if (channel->write_fd >= 0) {
        close(channel->write_fd);
        channel->write_fd = -1;
    }
}

int write_to_comm_channel(CommChannel* channel, const char* data, size_t len) {
    if (!channel || !data || channel->write_fd < 0) return -1;
    
    ssize_t bytes_written = write(channel->write_fd, data, len);
    return (bytes_written == (ssize_t)len) ? 0 : -1;
}

int read_from_comm_channel(CommChannel* channel, char* buffer, size_t buffer_size) {
    if (!channel || !buffer || channel->read_fd < 0) return -1;
    
    ssize_t bytes_read = read(channel->read_fd, buffer, buffer_size - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        return (int)bytes_read;
    }
    return -1;
}
