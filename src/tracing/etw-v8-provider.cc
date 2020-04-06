// Copyright 2020 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "etw-v8-provider.h"

#include <string>
#include <type_traits>

#include "etw-metadata.h"

namespace v8 {
namespace internal {
namespace etw {

#if defined(V8_ETW) && defined(V8_TARGET_OS_WIN)

void V8Provider::LogMsg(const char* msg) {
  constexpr static auto event_desc = EventDescriptor(kMsgEvent);
  constexpr static auto event_meta = EventMetadata("Msg",
    Field("Msg", kTypeAnsiStr));

  LogEventData(State(), &event_desc, &event_meta, msg);
}

void V8Provider::LogInitializePlatform() {
  constexpr static auto event_desc = EventDescriptor(kInitializePlatformEvent);
  constexpr static auto event_meta = EventMetadata("InitializePlatform");

  LogEventData(State(), &event_desc, &event_meta);
}

void V8Provider::LogShutdownPlatform() {
  constexpr static auto event_desc = EventDescriptor(kShutdownPlatformEvent);
  constexpr static auto event_meta = EventMetadata("ShutdownPlatform");
  
  LogEventData(State(), &event_desc, &event_meta);
}

void V8Provider::LogInitializeV8() {
  constexpr static auto event_desc = EventDescriptor(kInitializeV8Event);
  constexpr static auto event_meta = EventMetadata("InitializeV8");
  
  LogEventData(State(), &event_desc, &event_meta);
}

void V8Provider::LogTearDownV8() {
  constexpr static auto event_desc = EventDescriptor(kTearDownV8Event);
  constexpr static auto event_meta = EventMetadata("TearDownV8");
  
  LogEventData(State(), &event_desc, &event_meta);
}

void V8Provider::LogIsolateStart(void* isolate) {
  constexpr static auto event_desc = EventDescriptor(kIsolateStartEvent);
  constexpr static auto event_meta = EventMetadata("IsolateStart",
      Field("isolate", kTypePointer));
  
  LogEventData(State(), &event_desc, &event_meta, isolate);
}

void V8Provider::LogIsolateStop(void* isolate) {
  constexpr static auto event_desc = EventDescriptor(kIsolateStopEvent);
  constexpr static auto event_meta = EventMetadata("IsolateStop",
      Field("isolate", kTypePointer));
  
  LogEventData(State(), &event_desc, &event_meta, isolate);
}

void V8Provider::LogSnapshotInitStart(void* isolate) {
  constexpr static auto event_desc = EventDescriptor(kSnapshotInitStartEvent);
  constexpr static auto event_meta = EventMetadata("SnapshotInitStart",
      Field("isolate", kTypePointer));
  
  LogEventData(State(), &event_desc, &event_meta, isolate);
}

void V8Provider::LogSnapshotInitStop(void* isolate) {
  constexpr static auto event_desc = EventDescriptor(kSnapshotInitStopEvent);
  constexpr static auto event_meta = EventMetadata("SnapshotInitStop",
      Field("isolate", kTypePointer));
  
  LogEventData(State(), &event_desc, &event_meta, isolate);
}

void V8Provider::LogParsingStart(void* isolate) {
  constexpr static auto event_desc = EventDescriptor(kParsingStartEvent);
  constexpr static auto event_meta = EventMetadata("ParsingStart", 
      Field("isolate", kTypePointer));
  
  LogEventData(State(), &event_desc, &event_meta, isolate);
}

void V8Provider::LogParsingStop(void* isolate) {
  constexpr static auto event_desc = EventDescriptor(kParsingStopEvent);
  constexpr static auto event_meta = EventMetadata("ParsingStop",
      Field("isolate", kTypePointer));

  LogEventData(State(), &event_desc, &event_meta, isolate);
}

void V8Provider::LogGenerateUnoptimizedCodeStart(void* isolate) {
  constexpr static auto event_desc = EventDescriptor(kGenerateUnoptimizedCodeStartEvent);
  constexpr static auto event_meta = EventMetadata("GenerateUnoptimizedCodeStart",
      Field("isolate", kTypePointer));

  LogEventData(State(), &event_desc, &event_meta, isolate);
}

void V8Provider::LogGenerateUnoptimizedCodeStop(void* isolate) {
  constexpr static auto event_desc = EventDescriptor(kGenerateUnoptimizedCodeStopEvent);
  constexpr static auto event_meta = EventMetadata("GenerateUnoptimizedCodeStop",
      Field("isolate", kTypePointer));

  LogEventData(State(), &event_desc, &event_meta, isolate);
}

void V8Provider::LogJitExecuteStart() {
  constexpr static auto event_desc = EventDescriptor(kJitExecuteStartEvent);
  constexpr static auto event_meta = EventMetadata("JitExecuteStart");
  
  LogEventData(State(), &event_desc, &event_meta);
}

void V8Provider::LogJitExecuteStop() {
  constexpr static auto event_desc = EventDescriptor(kJitExecuteStopEvent);
  constexpr static auto event_meta = EventMetadata("JitExecuteStop");
  
  LogEventData(State(), &event_desc, &event_meta);
}

void V8Provider::LogJitFinalizeStart() {
  constexpr static auto event_desc = EventDescriptor(kJitFinalizeStartEvent);
  constexpr static auto event_meta = EventMetadata("JitFinalizeStart");
  
  LogEventData(State(), &event_desc, &event_meta);
}

void V8Provider::LogJitFinalizeStop() {
  constexpr static auto event_desc = EventDescriptor(kJitFinalizeStopEvent);
  constexpr static auto event_meta = EventMetadata("JitFinalizeStop");
  
  LogEventData(State(), &event_desc, &event_meta);
}

void V8Provider::LogDeopt(const std::string& reason, const std::string& kind, const std::string& src, 
      const std::string& fn, int line, int column) {
  constexpr static auto event_desc = EventDescriptor(kDeoptEvent);
  constexpr static auto event_meta = EventMetadata("Deopt",
      Field("reason", kTypeAnsiStr),
      Field("kind", kTypeAnsiStr),
      Field("src", kTypeAnsiStr),
      Field("fn", kTypeAnsiStr),
      Field("line", kTypeInt32),
      Field("column", kTypeInt32)
  );
  
  LogEventData(State(), &event_desc, &event_meta, reason, kind, src, fn, line, column);
}

void V8Provider::LogDisableOpt(const std::string& fn_name, const std::string& reason) {
  constexpr static auto event_desc = EventDescriptor(kDisableOptEvent);
  constexpr static auto event_meta = EventMetadata("DisableOpt");
  
  LogEventData(State(), &event_desc, &event_meta,
      Field("fn", kTypeAnsiStr),
      Field("reason", kTypeAnsiStr));
}

// Handle code event notifications and log Chakra-like ETW events.
void V8Provider::CodeEventHandler(const JitCodeEvent* event) {
    if (!v8Provider.IsEnabled() || v8Provider.Level() < kLevelInfo) return;
    v8Provider.LogCodeEvent(event);
}

void V8Provider::LogCodeEvent(const JitCodeEvent* event) {
  if (event->code_type != v8::JitCodeEvent::CodeType::JIT_CODE) return;

  // TODO(billti@microsoft.com): Support/test interpreted code, RegExp, Wasm, etc.
  constexpr static auto source_load_event_desc = EventDescriptor(kSourceLoadEvent);
  constexpr static auto source_load_event_meta = EventMetadata("SourceLoad",
      Field("SourceID", etw::kTypeUInt64),
      Field("ScriptContextID", etw::kTypePointer),
      Field("SourceFlags", etw::kTypeUInt32),
      Field("Url", etw::kTypeUnicodeStr));
  
  constexpr static auto method_load_event_desc = EventDescriptor(kMethodLoadEvent);
  constexpr static auto method_load_event_meta = EventMetadata("MethodLoad",
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

  // TODO(billti@microsoft.com): There are events for CODE_ADD_LINE_POS_INFO and CODE_MOVED. Need these?
  // Note: There is no event (currently) for code being removed.
  if (event->type == v8::JitCodeEvent::EventType::CODE_ADDED) {
    int name_len = static_cast<int>(event->name.len);
    // Note: event->name.str is not null terminated.
    std::wstring method_name(name_len + 1, '\0');
    MultiByteToWideChar(CP_UTF8, 0, event->name.str, name_len,
        // Const cast needed as building with C++14 (not const in >= C++17)
        const_cast<LPWSTR>(method_name.data()),
        static_cast<int>(method_name.size()));

    void* script_context = static_cast<void*>(event->isolate);
    int script_id = 0;
    if (!event->script.IsEmpty()) {
      // if the first time seeing this source file, log the SourceLoad event
      script_id = event->script->GetId();
      auto& script_map = (*isolate_script_map)[script_context];
      if (script_map.find(script_id) == script_map.end()) {
        auto script_name = event->script->GetScriptName();
        if (script_name->IsString()) {
          auto v8str_name = script_name.As<v8::String>();
          std::wstring wstr_name(v8str_name->Length(), L'\0');
          // On Windows wchar_t == uint16_t. const_cast needed for C++14.
          uint16_t* wstr_data = const_cast<uint16_t*>(
              reinterpret_cast<const uint16_t*>(wstr_name.data())
          );
          v8str_name->Write(event->isolate, wstr_data);
          script_map.emplace(script_id, std::move(wstr_name));
        } else {
          script_map.emplace(script_id, std::wstring{L"[unknown]"});
        }
        const std::wstring& url = script_map[script_id];
        LogEventData(State(), &source_load_event_desc, &source_load_event_meta,
          (uint64_t)script_id,
          (void*)script_context,
          (uint32_t)0,           // SourceFlags
          url
        );
      }
    }

    // TODO(billti): Can there be more than one context per isolate to handle?
    LogEventData(State(), &method_load_event_desc, &method_load_event_meta,
        (void*)script_context,
        (void*)event->code_start, 
        (uint64_t)event->code_len, 
        (uint32_t)0,  // MethodId
        (uint16_t)0,  // MethodFlags
        (uint16_t)0,  // MethodAddressRangeId
        (uint64_t)script_id,
        (uint32_t)0, (uint32_t)0,  // Line & Column
        method_name);
  }
}

#endif  // defined(V8_ETW) && defined(V8_TARGET_OS_WIN)

// Create the global "etw::v8Provider" that is the instance of the provider
static_assert(std::is_trivial<V8Provider>::value, "V8Provider is not trivial");
V8Provider v8Provider{};

}  // namespace etw
}  // namespace internal
}  // namespace v8
