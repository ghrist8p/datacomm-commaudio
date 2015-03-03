// circular buffer

/**
 * initializes instance variables. the circular buffer is a fixes-sized FIFO
 *   queue.
 *
 * @param maxElements maximum number of elements that the buffer can contain.
 * @param elementSize the size in bytes of a single element.
 */
CircularBuffer::constructor(int maxElements, int elementSize)
{
/*
    initialize instance variables...
    allocate maxElements*elementSize bytes of memory
 */
}

/**
 * places the data at {src}, and puts it in the buffer.
 *
 * @param  src pointer to data.
 */
void CircularBuffer::enqueue(void* src)
{
/*
    blocks until there is room in the queue
    copies {elementSize} bytes from {src} into an element in the message queue
 */
}

/**
 * reads an element from the buffer, and writes it to {dest}.
 *
 * @param dest pointer to a buffer where the element from the buffer is written
 *   into.
 */
void CircularBuffer::dequeue(void* dest)
{
/*
    blocks until there is data in the queue to read
    copies {elementSize} bytes from the queue into {dest}
 */
}

/**
 * removes all the elements from the buffer.
 */
void CircularBuffer::clear()
{
/*
    removes all elements from the queue
 */
}
