
#ifndef __SERIALIZER_HPP__
#define __SERIALIZER_HPP__

#include "utils2.hpp"
#include "utils.hpp"

/* serializer_t is an abstract interface that describes how each serializer should
behave. It is implemented by log_serializer_t, semantic_checking_serializer_t, and
others. */

typedef uint32_t ser_block_id_t;
#define NULL_SER_BLOCK_ID (ser_block_id_t(-1))

typedef uint64_t ser_transaction_id_t;
#define NULL_SER_TRANSACTION_ID (ser_transaction_id_t(0))
#define FIRST_SER_TRANSACTION_ID (ser_transaction_id_t(1))


struct serializer_t :
    /* Except as otherwise noted, the serializer's methods should only be called from the
    thread it was created on, and it should be destroyed on that same thread. */
    public home_cpu_mixin_t
{
    /* The buffers that are used with do_read() and do_write() must be allocated using
    these functions. They can be safely called from any thread. */
    
    virtual void *malloc() = 0;
    virtual void *clone(void*) = 0; // clones a buf
    virtual void free(void*) = 0;
    
    /* Reading a block from the serializer */
    
    struct read_callback_t {
        virtual void on_serializer_read() = 0;
    };
    virtual bool do_read(ser_block_id_t block_id, void *buf, read_callback_t *callback) = 0;
    
    /* do_write() updates or deletes a group of bufs.
    
    Each write_t passed to do_write() identifies an update or deletion. If 'buf' is NULL, then it
    represents a deletion. If 'buf' is non-NULL, then it identifies an update, and the given
    callback will be called as soon as the data has been copied out of 'buf'. If the entire
    transaction completes immediately, it will return 'true'; otherwise, it will return 'false' and
    call the given callback at a later date.
    
    'writes' can be freed as soon as do_write() returns. */
    
    struct write_txn_callback_t {
        virtual void on_serializer_write_txn() = 0;
    };
    struct write_block_callback_t {
        virtual void on_serializer_write_block() = 0;
    };
    struct write_t {
        ser_block_id_t block_id;
        const void *buf;   /* If NULL, a deletion */
        write_block_callback_t *callback;
    };
    virtual bool do_write(write_t *writes, int num_writes, write_txn_callback_t *callback) = 0;
    
    /* The size, in bytes, of each serializer block */
    
    virtual size_t get_block_size() = 0;
    
    /* max_block_id() and block_in_use() are used by the buffer cache to reconstruct
    the free list of unused block IDs. */
    
    /* Returns a block ID such that every existing block has an ID less than
    that ID. Note that block_in_use(max_block_id() - 1) is not guaranteed. */
    virtual ser_block_id_t max_block_id() = 0;
    
    /* Checks whether a given block ID exists */
    virtual bool block_in_use(ser_block_id_t id) = 0;
};

#endif /* __SERIALIZER_HPP__ */
