# Handshake Protocol Compatibility

## Issue Identified

The Z-Monitor's `SharedMemoryControlChannel` receives the ControlMessage and file descriptor in two separate calls:
1. First `recv()` to get the ControlMessage structure
2. Then `recvmsg()` to get the file descriptor via SCM_RIGHTS

However, when a message is sent with `sendmsg()` containing both data and ancillary data (SCM_RIGHTS), both should be received together using `recvmsg()`. Using `recv()` first may consume the data portion but lose the ancillary data.

## Current Implementation

The simulator's `ControlServer` sends the ControlMessage and file descriptor together in one `sendmsg()` call:
- ControlMessage structure in the data portion (msg_iov)
- File descriptor in ancillary data (SCM_RIGHTS)

## Compatibility Status

**Status:** ⚠️ **Potential Issue**

The Z-Monitor code may need to be updated to use `recvmsg()` for the first receive to get both the ControlMessage and FD together. Alternatively, the simulator could send them in two separate messages, but this is less efficient.

## Recommended Fix

Update Z-Monitor's `SharedMemoryControlChannel::onSocketDataAvailable()` to use `recvmsg()` for the first receive:

```cpp
void SharedMemoryControlChannel::onSocketDataAvailable() {
    // Use recvmsg() to get both ControlMessage and FD together
    struct msghdr msg = {0};
    struct iovec iov;
    ControlMessage message;
    char cmsg_buffer[CMSG_SPACE(sizeof(int))];
    
    iov.iov_base = &message;
    iov.iov_len = sizeof(message);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg_buffer;
    msg.msg_controllen = sizeof(cmsg_buffer);
    
    ssize_t received = ::recvmsg(m_socketFd, &msg, 0);
    // ... extract both ControlMessage and FD from the same message
}
```

## Testing

To verify compatibility:
1. Run the simulator
2. Connect Z-Monitor's SharedMemorySensorDataSource
3. Verify the handshake completes successfully
4. Verify data flows through shared memory

If the handshake fails, update Z-Monitor's code to use `recvmsg()` for the first receive.

