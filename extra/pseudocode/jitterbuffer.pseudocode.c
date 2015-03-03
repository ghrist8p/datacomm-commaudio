// jitter buffer

/**
 * the jitter buffer is used to mitigate the effect of jitter from the network.
 *
 * data can arrive out of order, as long as they specify the order that they're
 *   supposed to be in.
 *
 * @param maxElements maximum number of elements that the buffer can contain.
 * @param elementSize the size in bytes of a single element.
 * @param consumeTimeout milliseconds to wait before enabling it to be consumed.
 */
JitterBuffer::constructor(int maxElements, int elementSize, int consumeTimeout)
{
/*
    initialize instance variables...
    allocate maxElements*elementSize bytes of memory
 */
}

/**
 * inserts the passed data into the jitter buffer. if the passed data is too
 *   late (has already been consumed by the consumer), then he function returns
 *   right away, with an error code. if the data arrives too early, then the
 *   function blocks until it can be put into the buffer.
 *
 * elements can be consumed (even if they're not produced yet) after
 *   {consumeTimeout} milliseconds of elements following it has been produced
 *   ... or then the element being consumed is produced.
 *
 * @param      index index in the jitter buffer that the data should be copied
 *   into. the index is independent of the element size.
 * @param      src pointer to the beginning of the data to be copied into the
 *   jitter buffer.
 *
 * @return     1 if the data could not be inserted, 0 otherwise.
 */
int JitterBuffer::insert(int index, void* src)
{
/*
    if index is smaller than the last consumed element...
        return 1

    copy {elementSize} bytes from {src} into index {index} of buffer
    update book keeping variables that keep track of last produced and such

    return 0
 */
}

/**
 * removes an element from the jitter buffer, and places it into the {dest}
 *   pointer.
 *
 * this function my block until something can be dequeued.
 *
 * @param  dest pointer to a buffer that is large enough to hold an element
 *   because an element from the jitter buffer will be copied to the {dest}
 *   pointer.
 */
void JitterBuffer::remove(void* dest)
{
/*
    waits until there are elements to remove from the buffer

    copy the data at the end of the queue into the {dest} pointer
    update book keeping variables
 */
}

/**
 * deletes all the elements from the jitter buffer.
 */
void JitterBuffer::clear()
{
/*
    deletes all elements from the jitter buffer
 */
}
