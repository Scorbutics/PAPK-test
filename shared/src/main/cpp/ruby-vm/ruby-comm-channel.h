#ifndef RUBY_COMM_CHANNEL_H
#define RUBY_COMM_CHANNEL_H

#ifdef __cplusplus
extern "C" {
#endif

// Communication channel structure for cross-platform compatibility
typedef struct {
    int read_fd;
    int write_fd;
} CommChannel;


// Helper functions for communication channels
int create_comm_channel(CommChannel* channel);
void close_comm_channel(CommChannel* channel);
int write_to_comm_channel(CommChannel* channel, const char* data, size_t len);
int read_from_comm_channel(CommChannel* channel, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif //RUBY_COMM_CHANNEL_H
