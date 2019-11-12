// Copyright 2019 Bill Ticehurst. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "include/v8config.h"
#if defined(V8_OS_WIN)

#include "include/v8.h"
#include "src/base/macros.h"

#include <string>
#include <unordered_map>

#include "src/tracing/etw-provider.h"

namespace v8 {

class ChakraEtwProvider : public etw::EtwProvider {
 public:
  ChakraEtwProvider();

  void SourceLoad(
    uint64_t source_id,
    void *script_context_id,
    uint32_t source_flags,
    const std::wstring& url);

  void MethodLoad(
    void *script_context_id,
    void *method_start_address,
    uint64_t method_size,
    uint32_t method_id,
    uint16_t method_flags,
    uint16_t method_address_range_id,
    uint64_t source_id,
    uint32_t line,
    uint32_t column,
    const std::wstring& method_name);

  // TODO(billti) SourceUnload & MethodUnload

  std::unordered_map<int, std::wstring>& GetScriptMapForIsolate(void *);

 private:
  // For each isolate, holds a map of script_id to script_names registered.
  std::unordered_map<void*, std::unordered_map<int, std::wstring>> script_map;
};

V8_EXPORT_PRIVATE void EtwEventHandler(const JitCodeEvent* event);
V8_EXPORT_PRIVATE void InitializeEtw();

}  // namespace v8

#endif  // defined(V8_OS_WIN)
