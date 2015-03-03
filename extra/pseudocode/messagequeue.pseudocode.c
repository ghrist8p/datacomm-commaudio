// message queue

/**
 * initializes instance variables. the circular buffer is a fixes-sized FIFO
 *   queue.
 */
MessageQueue::constructor()
{
/*
    initialize instance variables...
    allocate maxElements*elementSize bytes of memory
 */
}

/**
 * copies {len} bytes from {src} into the buffer, and stores the type
 *   information.
 *
 * @param type integer indicating what kind of message this is.
 * @param src pointer to the data to copy into the message queue.
 * @param len number of bytes to copy from {src} into the buffer.
 */
void MessageQueue::enqueue(int type, void* src, int len)
{
/*
    blocks until there is room in the queue
    copy {elementSize} bytes from {src} into an element in the message queue
    record {len}, so we can return it later
    record {type}, so we can record it later
 */
}

/**
 * reads an element from the buffer, and writes it to {dest}, and writes the
 *   number of bytres read from the buffer to {len}, and the type of data copied
 *   into {type}.
 *
 * @param type the type that the data was stored as.
 * @param dest pointer to a buffer where the element from the buffer is written
 *   into.
 * @param len number of bytes copied from the buffer to {dest}.
 */
void MessageQueue::dequeue(int* type, void* dest, int* len)
{
/*
    blocks until there is data in the queue to read
    copy {elementSize} bytes from the queue into {dest}
    write the number of bytes copied from the buffer to {len}
    write the type of data copied to {type}
 */
}

/**
 * removes all the elements from the buffer.
 */
void MessageQueue::clear()
{
/*
    removes all elements from the queue
 */
}
