// Copyright 2019 Bill Ticehurst. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/v8config.h"
#if defined(V8_OS_WIN)

#include "src/base/once.h"
#include "src/flags/flags.h"
#include "src/tracing/etw-provider.h"
#include "src/tracing/v8-etw-provider.h"

#include <string>

namespace v8 {

using etw::EtwProvider;
using etw::EventDescriptor;
using etw::EventMetadata;
using etw::Field;

V8_DECLARE_ONCE(once_init);
static ChakraEtwProvider* etw_provider = nullptr;

/*
Note: Below should be run from an admin prompt.

For simple testing, use "logman" to create a trace for this provider via:

  logman create trace -n chakra -o chakra.etl -p {57277741-3638-4A4B-BDBA-0AC6E45DA56C}

After the provider GUID, you can optionally specificy keywords and level, e.g.

  -p {57277741-3638-4A4B-BDBA-0AC6E45DA56C} 0xBEEF 0x05

To capture events, start/stop the trace via:

  logman start chakra
  logman stop chakra

When finished recording, remove the configured trace via:

  logman delete chakra

Alternatively, use a tool such as PerfView or WPR to configure and record
traces.
*/

// {57277741-3638-4A4B-BDBA-0AC6E45DA56C}
constexpr GUID chakra_provider_guid = {0x57277741, 0x3638, 0x4A4B,
  {0xBD, 0xBA, 0x0A, 0xC6, 0xE4, 0x5D, 0xA5, 0x6C}};
constexpr char chakra_provider_name[] = "Microsoft-JScript";

ChakraEtwProvider::ChakraEtwProvider()
    : EtwProvider(chakra_provider_guid, chakra_provider_name) {}

void ChakraEtwProvider::SourceLoad(
    uint64_t source_id,
    void *script_context_id,
    uint32_t source_flags,
    const std::wstring& url) {
  constexpr static auto event_desc = EventDescriptor(
    41,  // EventId
    etw::kLevelInfo,
    1,   // JScriptRuntimeKeyword
    12,  // SourceLoadOpcode
    2);  // ScriptContextRuntimeTask
  constexpr static auto event_meta = EventMetadata("SourceLoad",
      Field("SourceID", etw::kTypeUInt64),
      Field("ScriptContextID", etw::kTypePointer),
      Field("SourceFlags", etw::kTypeUInt32),
      Field("Url", etw::kTypeUnicodeStr));

  LogEventData(&event_desc, &event_meta,
      source_id, script_context_id, source_flags, url);
}

void ChakraEtwProvider::MethodLoad(
    void *script_context_id,
    void *method_start_address,
    uint64_t method_size,
    uint32_t method_id,
    uint16_t method_flags,
    uint16_t method_address_range_id,
    uint64_t source_id,
    uint32_t line,
    uint32_t column,
    const std::wstring& method_name) {
  constexpr static auto event_desc = EventDescriptor(
    9,   // EventId
    etw::kLevelInfo,
    1,   // JScriptRuntimeKeyword
    10,  // MethodLoadOpcode
    1);  // MethodRuntimeTask
  constexpr static auto event_meta = EventMetadata("MethodLoad",
      Field("ScriptContextID", etw::kTypePointer),
      Field("MethodStartAddress", etw::kTypePointer),
      Field("MethodSize", etw::kTypeUInt64),
      Field("MethodID", etw::kTypeUInt32),
      Field("MethodFlags", etw::kTypeUInt16),
      Field("MethodAddressRangeID", etw::kTypeUInt16),
      Field("SourceID", etw::kTypeUInt64),
      Field("Line", etw::kTypeUInt32),
      Field("Column", etw::kTypeUInt32),
      Field("MethodName", etw::kTypeUnicodeStr));

  LogEventData(&event_desc, &event_meta,
      script_context_id, method_start_address, method_size, method_id,
      method_flags, method_address_range_id, source_id,
      line, column, method_name);
}

// Assigns the static on the first run. A no-op after that.
// This is a optimization to avoid using a static local in EtwEventHandler
// which would add some instructions on every tracing call.
void InitializeEtw() {
  v8::base::CallOnce(&once_init, []() {
    // Allocating a static pointer means the destructor will not run at process
    // exit. This is fine for a process wide registered provider.
    // See "Static and Global Variables" in https://google.github.io/styleguide/cppguide.html

    // TODO(billti): Only emulate Chakra if FLAG_etw_tracing_chakra == true
    etw_provider = new ChakraEtwProvider();
  });
}

void EtwEventHandler(const JitCodeEvent* event) {
  // All events current are Info level events. Bail if this level isn't enabled.
  if (V8_LIKELY(!etw_provider->IsEventEnabled(etw::kLevelInfo, 0))) return;

  // TODO(billti): Support/test interpreted code, RegExp, Wasm, etc.
  if (event->code_type != v8::JitCodeEvent::CodeType::JIT_CODE) return;

  // TODO(billti): There are events for CODE_ADD_LINE_POS_INFO and CODE_MOVED
  // Note: There is no event (currently) for code being removed.
  if (event->type == v8::JitCodeEvent::EventType::CODE_ADDED) {
    int name_len = static_cast<int>(event->name.len);
    // Note: event->name.str is not null terminated.
    std::wstring method_name(name_len + 1, '\0');
    int name_chars = MultiByteToWideChar(CP_UTF8, 0, event->name.str, name_len,
        method_name.data(), method_name.size());

    if(!event->script.IsEmpty()){
      int scriptId = event->script->GetId();
      auto scriptName = event->script->GetScriptName();
      if (scriptName->IsString() ) {
          char buff[1024];
          auto str = scriptName.As<v8::String>();
          str->WriteUtf8(event->isolate, buff, 1024);
          // TODO(billti): Maintain a script map to write the SourceLoad events
          // if (scriptMap.find(scriptId) != scriptMap.end()) {
          //
          // }
      }
    }

    // TODO(billti): Can there be more than one context per isolate to handle?
    void* script_context = static_cast<void*>(event->isolate);
    int64_t script_id = event->script.IsEmpty() ? 0 : event->script->GetId();
    void* start_address = event->code_start;
    int64_t length = static_cast<int64_t>(event->code_len);
    etw_provider->MethodLoad(script_context, start_address, length,
        0,  // MethodId
        0,  // MethodFlags
        0,  // MethodAddressRangeId
        script_id,
        0, 0,  // Line & Column
        method_name);
  }
}

}  // namespace v8

#endif  // defined(V8_OS_WIN)
