/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Microsoft Corporation
 *
 * -=- Robust Distributed System Nucleus (rDSN) -=-
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * Description:
 *     Service API  in rDSN
 *
 * Revision history:
 *     Mar., 2015, @imzhenyu (Zhenyu Guo), first version
 *     Feb., 2016, @imzhenyu (Zhenyu Guo), add comments for V1 release
 *     xxxx-xx-xx, author, fix bug about xxx
 */

#pragma once

#include <dsn/c/api_common.h>
#include <dsn/c/api_task.h>
#include <dsn/tool-api/gpid.h>
#include <dsn/tool-api/rpc_address.h>
#include <dsn/tool-api/task_tracker.h>

/*!
 @defgroup service-api-c Core Service API

 @ingroup service-api

  Core service API for building applications and distributed frameworks, which
  covers the major categories that a server application may use, shown in the modules below.

 @{
 */

/*!
 @defgroup task-common Common Task Operations

 Common Task/Event Operations

 rDSN adopts the event-driven programming model, where all computations (event handlers) are
 represented as individual tasks; each is the execution of a sequential piece of code in one thread.
 Specifically, rDSN categorizes the tasks into four types, as defined in \ref dsn_task_type_t.

Unlike the traditional event-driven programming, rDSN enhances the model in the following ways,
with which they control the application in many aspects in a declarative approach.

- each task is labeled with a task code, with which developers can configure many aspects in config
files.
  Developers can define new task code using \ref DEFINE_TASK_CODE, or \ref dsn_task_code_register.

  <PRE>
  [task..default]
  ; allow task executed in other thread pools or tasks
  ; for TASK_TYPE_COMPUTE - allow-inline allows a task being executed in its caller site
  ; for other tasks - allow-inline allows a task being execution in io-thread
  allow_inline = false

  ; group rpc mode with group address: GRPC_TO_LEADER, GRPC_TO_ALL, GRPC_TO_ANY
  grpc_mode = GRPC_TO_LEADER

  ; when toollet profiler is enabled
  is_profile = true

  ; when toollet tracer is enabled
  is_trace = true

  ; thread pool to execute the task
  pool_code = THREAD_POOL_DEFAULT

  ; task priority
  priority = TASK_PRIORITY_COMMON

  ; whether to randomize the timer delay to random(0, timer_interval),
  ; if the initial delay is zero, to avoid multiple timers executing at the same time (e.g.,
checkpointing)
  randomize_timer_delay_if_zero = false

  ; what kind of network channel for this kind of rpc calls
  rpc_call_channel = RPC_CHANNEL_TCP

  ; what kind of header format for this kind of rpc calls
  rpc_call_header_format = NET_HDR_DSN

  ; how many milliseconds to delay recving rpc session for
  ; when queue length ~= [1.0, 1.2, 1.4, 1.6, 1.8, >=2.0] x pool.queue_length_throttling_threshold,
  ; e.g., 0, 0, 1, 2, 5, 10
  rpc_request_delays_milliseconds = 0, 0, 1, 2, 5, 10

  ; whether to drop a request right before execution when its queueing time
  ; is already greater than its timeout value
  rpc_request_dropped_before_execution_when_timeout = false

  ; for how long (ms) the request will be resent if no response
  ; is received yet, 0 for disable this feature
  rpc_request_resend_timeout_milliseconds = 0

  ; throttling mode for rpc requets: TM_NONE, TM_REJECT, TM_DELAY when
  ; queue length > pool.queue_length_throttling_threshold
  rpc_request_throttling_mode = TM_NONE

  ; what is the default timeout (ms) for this kind of rpc calls
  rpc_timeout_milliseconds = 5000

  [task.LPC_AIO_IMMEDIATE_CALLBACK]
  ; override the option in [task..default]
  allow_inline = true
  </PRE>

- each task code is bound to a thread pool, which can be customized as follows.
  Developers can define new thread pools using \ref DEFINE_THREAD_POOL_CODE, or \ref
dsn_threadpool_code_register.

  <PRE>
  [threadpool..default]

  ; how many tasks (if available) should be returned for
  ; one dequeue call for best batching performance
  dequeue_batch_size = 5

  ; throttling: whether to enable throttling with virtual queues
  enable_virtual_queue_throttling = false

  ; thread pool name
  name = THREAD_POOL_INVALID

  ; whethe the threads share a single queue(partitioned=false) or not;
  ; the latter is usually for workload hash partitioning for avoiding locking
  partitioned = false

  ; task queue aspects names, usually for tooling purpose
  queue_aspects =

  ; task queue provider name
  queue_factory_name = dsn::tools::hpc_concurrent_task_queue

  ; throttling: throttling threshold above which rpc requests will be dropped
  queue_length_throttling_threshold = 1000000

  ; what CPU cores are assigned to this pool, 0 for all
  worker_affinity_mask = 0

  ; task aspects names, usually for tooling purpose
  worker_aspects =

  ; thread/worker count
  worker_count = 2

  ; task worker provider name
  worker_factory_name =

  ; thread priority
  worker_priority = THREAD_xPRIORITY_NORMAL

  ; whether the threads share all assigned cores
  worker_share_core = true

  [threadpool.THREAD_POOL_DEFAULT]
  ; override default options in [threadpool..default]
  dequeue_batch_size = 5

  </PRE>
-

 @{
 */
/*! cancel the later execution of the timer task inside the timer */
extern DSN_API void dsn_task_cancel_current_timer();

/*!
 check whether the task is currently running inside the given task

 \param t the given task handle

 \return true if it is.
 */
extern DSN_API bool dsn_task_is_running_inside(dsn::task *t);

/*@}*/

/*!
 @defgroup tasking Asynchronous Tasks and Timers

 Asynchronous Tasks and Timers

 @{
 */

/*!
@defgroup rpc Remote Procedure Call (RPC)

Remote Procedure Call (RPC)

Note developers can easily plugin their own implementation to
replace the underneath implementation of the network (e.g., RDMA, simulated network)
@{
*/

/*!
@{
*/

extern DSN_API dsn::rpc_address dsn_primary_address();

/*!
@defgroup rpc-server Server-Side RPC Primitives

Server-Side RPC Primitives
@{
 */

/*! register callback to handle RPC request */
extern DSN_API bool dsn_rpc_register_handler(dsn::task_code code,
                                             const char *extra_name,
                                             const dsn::rpc_request_handler &cb);

/*! unregister callback to handle RPC request, returns true if unregister ok, false if no handler
    was registered */
extern DSN_API bool dsn_rpc_unregiser_handler(dsn::task_code code);

/*! reply with a response which is created using dsn::message_ex::create_response */
extern DSN_API void dsn_rpc_reply(dsn::message_ex *response,
                                  dsn::error_code err DEFAULT(dsn::ERR_OK));

/*! forward the request to another server instead */
extern DSN_API void dsn_rpc_forward(dsn::message_ex *request, dsn::rpc_address addr);

/*@}*/

/*!
@defgroup rpc-client Client-Side RPC Primitives

Client-Side RPC Primitives
@{
*/

/*! client invokes the RPC call */
extern DSN_API void dsn_rpc_call(dsn::rpc_address server, dsn::rpc_response_task *rpc_call);

/*!
   client invokes the RPC call and waits for its response, note
   returned msg must be explicitly released using \ref dsn::message_ex::release_ref
 */
extern DSN_API dsn::message_ex *dsn_rpc_call_wait(dsn::rpc_address server,
                                                  dsn::message_ex *request);

/*! one-way RPC from client, no rpc response is expected */
extern DSN_API void dsn_rpc_call_one_way(dsn::rpc_address server, dsn::message_ex *request);

/*! this is to mimic a response is received when no real rpc is called */
extern DSN_API void dsn_rpc_enqueue_response(dsn::rpc_response_task *rpc_call,
                                             dsn::error_code err,
                                             dsn::message_ex *response);

/*@}*/

/*@}*/

/*!
@defgroup file File Operations

File Operations

Note developers can easily plugin their own implementation to
replace the underneath implementation of these primitives.
@{
*/
typedef struct
{
    void *buffer;
    int size;
} dsn_file_buffer_t;

/*! the following ctrl code are used by \ref dsn_file_ctrl. */
typedef enum dsn_ctrl_code_t {
    CTL_BATCH_INVALID = 0,
    CTL_BATCH_WRITE = 1,            ///< (batch) set write batch size
    CTL_MAX_CON_READ_OP_COUNT = 2,  ///< (throttling) maximum concurrent read ops
    CTL_MAX_CON_WRITE_OP_COUNT = 3, ///< (throttling) maximum concurrent write ops
} dsn_ctrl_code_t;

/*!
 open file

 \param file_name filename of the file.
 \param flag      flags such as O_RDONLY | O_BINARY used by ::open
 \param pmode     permission mode used by ::open

 \return file handle
 */
extern DSN_API dsn_handle_t dsn_file_open(const char *file_name, int flag, int pmode);

/*! close the file handle */
extern DSN_API dsn::error_code dsn_file_close(dsn_handle_t file);

/*! flush the buffer of the given file */
extern DSN_API dsn::error_code dsn_file_flush(dsn_handle_t file);

/*! get native handle: HANDLE for windows, int for non-windows */
extern DSN_API void *dsn_file_native_handle(dsn_handle_t file);

/*!
 read file asynchronously

 \param file   file handle
 \param buffer read buffer
 \param count  byte size of the read buffer
 \param offset offset in the file to start reading
 \param cb     callback aio task to be executed on completion
 */
extern DSN_API void
dsn_file_read(dsn_handle_t file, char *buffer, int count, uint64_t offset, dsn::aio_task *cb);

/*!
 write file asynchronously

 \param file   file handle
 \param buffer write buffer
 \param count  byte size of the to-be-written content
 \param offset offset in the file to start write
 \param cb     callback aio task to be executed on completion
 */
extern DSN_API void dsn_file_write(
    dsn_handle_t file, const char *buffer, int count, uint64_t offset, dsn::aio_task *cb);

/*!
 write file asynchronously with vector buffers

 \param file          file handle
 \param buffers       write buffers
 \param buffer_count  number of write buffers
 \param offset        offset in the file to start write
 \param cb            callback aio task to be executed on completion
 */
extern DSN_API void dsn_file_write_vector(dsn_handle_t file,
                                          const dsn_file_buffer_t *buffers,
                                          int buffer_count,
                                          uint64_t offset,
                                          dsn::aio_task *cb);
/*@}*/

/*!
@defgroup env Environment

Non-deterministic Environment Input

Note developers can easily plugin their own implementation to
replace the underneath implementation of these primitives.
@{
*/
extern DSN_API uint64_t dsn_runtime_init_time_ms();
extern DSN_API uint64_t dsn_now_ns();

/*! return [min, max] */
extern DSN_API uint64_t dsn_random64(uint64_t min, uint64_t max);

__inline uint64_t dsn_now_us() { return dsn_now_ns() / 1000; }
__inline uint64_t dsn_now_ms() { return dsn_now_ns() / 1000000; }

/*! return [min, max] */
__inline uint32_t dsn_random32(uint32_t min, uint32_t max)
{
    return (uint32_t)(dsn_random64(min, max));
}

__inline double dsn_probability() { return (double)(dsn_random64(0, 1000000000)) / 1000000000.0; }

/*@}*/

/*!
@defgroup sync Thread Synchornization

Thread Synchornization Primitives

Note developers can easily plugin their own implementation to
replace the underneath implementation of these primitives.
@{
*/

/*!
@defgroup sync-exlock Exlusive Locks
Exlusive Locks
@{
*/

/*! create a recursive? or not exlusive lock*/
extern DSN_API dsn_handle_t dsn_exlock_create(bool recursive);
extern DSN_API void dsn_exlock_destroy(dsn_handle_t l);
extern DSN_API void dsn_exlock_lock(dsn_handle_t l);
extern DSN_API bool dsn_exlock_try_lock(dsn_handle_t l);
extern DSN_API void dsn_exlock_unlock(dsn_handle_t l);
/*@}*/

/*!
@defgroup sync-rwlock Non-recursive Read-Write Locks
Non-recursive Read-Write Locks
@{
*/
extern DSN_API dsn_handle_t dsn_rwlock_nr_create();
extern DSN_API void dsn_rwlock_nr_destroy(dsn_handle_t l);
extern DSN_API void dsn_rwlock_nr_lock_read(dsn_handle_t l);
extern DSN_API void dsn_rwlock_nr_unlock_read(dsn_handle_t l);
extern DSN_API bool dsn_rwlock_nr_try_lock_read(dsn_handle_t l);
extern DSN_API void dsn_rwlock_nr_lock_write(dsn_handle_t l);
extern DSN_API void dsn_rwlock_nr_unlock_write(dsn_handle_t l);
extern DSN_API bool dsn_rwlock_nr_try_lock_write(dsn_handle_t l);
/*@}*/

/*!
@defgroup sync-sema Semaphore
Semaphore
@{
*/
/*! create a semaphore with initial count equals to inital_count */
extern DSN_API dsn_handle_t dsn_semaphore_create(int initial_count);
extern DSN_API void dsn_semaphore_destroy(dsn_handle_t s);
extern DSN_API void dsn_semaphore_signal(dsn_handle_t s, int count);
extern DSN_API void dsn_semaphore_wait(dsn_handle_t s);
extern DSN_API bool dsn_semaphore_wait_timeout(dsn_handle_t s, int timeout_milliseconds);
/*@}*/

/*@}*/

/*@}*/
