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

#include <dsn/cpp/message_utils.h>

namespace dsn {

/*extern*/ dsn::message_ex *from_blob_to_received_msg(dsn::task_code rpc_code,
                                                      const blob &bb,
                                                      int thread_hash,
                                                      uint64_t partition_hash,
                                                      dsn_msg_serialize_format serialization_type)
{
    auto msg = ::dsn::message_ex::create_receive_message_with_standalone_header(bb);
    msg->local_rpc_code = rpc_code;
    const char *name = rpc_code.to_string();
    strncpy(msg->header->rpc_name, name, strlen(name));

    msg->header->client.thread_hash = thread_hash;
    msg->header->client.partition_hash = partition_hash;
    msg->header->context.u.serialize_format = serialization_type;
    msg->add_ref(); // released by callers explicitly using dsn_msg_release
    return msg;
}

} // namespace dsn
