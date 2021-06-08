/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#pragma once

#include <rocksdb/slice.h>

namespace pegasus {
namespace server {
/** compaction_filter_rule represents the compaction rule to filter the keys which are stored in
 * rocksdb. */
class compaction_filter_rule
{
public:
    virtual ~compaction_filter_rule() = default;

    // TODO(zhaoliwei): we can use `value_filed` to replace existing_value in the later,
    // after the refactor of value schema
    virtual bool match(const std::string &hash_key,
                       const std::string &sort_key,
                       const rocksdb::Slice &existing_value) const = 0;
};
} // namespace server
} // namespace pegasus