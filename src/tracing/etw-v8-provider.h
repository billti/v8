// Copyright 2020 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_TRACING_ETW_V8_PROVIDER_H_
#define V8_TRACING_ETW_V8_PROVIDER_H_

/*
Provide name and GUID generated from it are:

    "V8.js",
    // {ca4c76aa-e822-589e-8f5d-9fdca8bad813}
    {0xca4c76aa,0xe822,0x589e,{0x8f,0x5d,0x9f,0xdc,0xa8,0xba,0xd8,0x13}};

Note: Below should be run from an admin prompt.

For simple testing, use "logman" to create a trace for this provider via:

  logman create trace -n v8js -o v8js.etl -p {ca4c76aa-e822-589e-8f5d-9fdca8bad813}

After the provider GUID, you can optionally specificy keywords and level, e.g.

  -p {ca4c76aa-e822-589e-8f5d-9fdca8bad813} 0xBEEF 0x05

To capture events, start/stop the trace via:
  logman start example
  logman stop example

When finished recording, remove the configured trace via:

  logman delete example

Alternatively, use a tool such as PerfView or WPR to configure and record traces.
*/

#include <unordered_map>

#include "etw-provider.h"
#include "include/v8.h"

namespace v8 {
namespace internal {
namespace etw {

constexpr char kProviderName[] = "V8.js";
// Below is generated from the "V8.js"name, which allows the "*V8.js" provider
// specifier in some tools (e.g. PerfView)
constexpr GUID kProviderGuid = {0xca4c76aa, 0xe822, 0x589e, {0x8f, 0x5d, 0x9f, 0xdc, 0xa8, 0xba, 0xd8, 0x13}};

// Using the below GUID enables tools that understand stack walking via Chakra events
// {57277741-3638-4A4B-BDBA-0AC6E45DA56C}
constexpr GUID kChakraGuid = {0x57277741, 0x3638, 0x4A4B, {0xBD, 0xBA, 0x0A, 0xC6, 0xE4, 0x5D, 0xA5, 0x6C}};
constexpr uint64_t kJScriptRuntimeKeyword = 1;
// Below 2 events are needed to mimic Chakra events needed for stack-walking
constexpr EventInfo kMethodLoadEvent {9, kLevelInfo, 10, 1, kJScriptRuntimeKeyword};
constexpr EventInfo kSourceLoadEvent {41, kLevelInfo, 12, 2, kJScriptRuntimeKeyword};

// Define the event descriptor data for each event
// Note: Order of fields is: eventId, level, opcode, task, keyword
constexpr EventInfo kMsgEvent                {100, kLevelInfo, 0, 0, 0};
constexpr EventInfo kInitializePlatformEvent {101, kLevelInfo, 0, 0, 0};
constexpr EventInfo kShutdownPlatformEvent   {102, kLevelInfo, 0, 0, 0};
constexpr EventInfo kInitializeV8Event       {103, kLevelInfo, 0, 0, 0};
constexpr EventInfo kTearDownV8Event         {104, kLevelInfo, 0, 0, 0};
constexpr EventInfo kIsolateStartEvent       {105, kLevelInfo, kOpCodeStart, 0, 0};
constexpr EventInfo kIsolateStopEvent        {106, kLevelInfo, kOpCodeStop,  0, 0};
constexpr EventInfo kSnapshotInitStartEvent  {107, kLevelInfo, kOpCodeStart, 0, 0};
constexpr EventInfo kSnapshotInitStopEvent   {108, kLevelInfo, kOpCodeStop,  0, 0};
constexpr EventInfo kParsingStartEvent       {109, kLevelVerbose, kOpCodeStart, 0, 0};
constexpr EventInfo kParsingStopEvent        {110, kLevelVerbose, kOpCodeStop,  0, 0};
constexpr EventInfo kGenerateUnoptimizedCodeStartEvent {111, kLevelVerbose, kOpCodeStart, 0, 0};
constexpr EventInfo kGenerateUnoptimizedCodeStopEvent  {112, kLevelVerbose, kOpCodeStop,  0, 0};
constexpr EventInfo kJitExecuteStartEvent    {113, kLevelVerbose, kOpCodeStart, 0, 0};
constexpr EventInfo kJitExecuteStopEvent     {114, kLevelVerbose, kOpCodeStop,  0, 0};
constexpr EventInfo kJitFinalizeStartEvent   {115, kLevelVerbose, kOpCodeStart, 0, 0};
constexpr EventInfo kJitFinalizeStopEvent    {116, kLevelVerbose, kOpCodeStop,  0, 0};
constexpr EventInfo kConcurrentMarkingStartEvent       {117, kLevelVerbose, kOpCodeStart, 0, 0};
constexpr EventInfo kConcurrentMarkingStopEvent        {118, kLevelVerbose, kOpCodeStop,  0, 0};
constexpr EventInfo kDeoptEvent              {119, kLevelVerbose, 0, 0, 0};
constexpr EventInfo kDisableOptEvent         {120, kLevelVerbose, 0, 0, 0};
// TODO: OSR, sweep, compaction, maybe optimization stages...

using ScriptMapType = std::unordered_map<void*, std::unordered_map<int, std::wstring>>;

class V8Provider : public EtwProvider {

#if defined(V8_ETW) && defined(V8_TARGET_OS_WIN)

 public:
  void RegisterEtwProvider() {
    // TODO(billti@microsoft.com): Update once tools understand the "V8.js" provider events.
    isolate_script_map = new ScriptMapType();
    Register(/* ProviderGuid */ kChakraGuid, kProviderName);
  }

  void UnregisterEtwProvider() {
    Unregister();
    if (isolate_script_map) delete isolate_script_map;
  }

  // The public APIs to log the events should all be inline wrappers that call
  // to internal implementations. You can check if a session is listening first
  // for optimal efficiency. That state is maintained by the base class.

  void Msg(const char* msg) {
    if (IsEnabled()) LogMsg(msg);
  }

  // The below are infrequent and expensive enough to not worry if enabled first
  void InitializePlatform() { LogInitializePlatform(); }
  void ShutdownPlatform()   { LogShutdownPlatform(); }
  void InitializeV8()       { LogInitializeV8(); }
  void TearDownV8()         { LogTearDownV8(); }
  void IsolateStart(void* isolate)      { LogIsolateStart(isolate); }
  void IsolateStop(void* isolate)       { LogIsolateStop(isolate); }
  void SnapshotInitStart(void* isolate) { LogSnapshotInitStart(isolate); }
  void SnapshotInitStop(void* isolate)  { LogSnapshotInitStop(isolate); }

  void ParsingStart(void* isolate) {
    if (IsEnabled()) LogParsingStart(isolate);
  }
  void ParsingStop(void* isolate) {
    if (IsEnabled()) LogParsingStop(isolate);
  }

  void GenerateUnoptimizedCodeStart(void* isolate) {
    if (IsEnabled()) LogGenerateUnoptimizedCodeStart(isolate);
  }
  void GenerateUnoptimizedCodeStop(void* isolate) {
    if (IsEnabled()) LogGenerateUnoptimizedCodeStop(isolate);
  }
  void JitExecuteStart()  { if (IsEnabled()) LogJitExecuteStart(); }
  void JitExecuteStop()   { if (IsEnabled()) LogJitExecuteStop(); }
  void JitFinalizeStart() { if (IsEnabled()) LogJitFinalizeStart(); }
  void JitFinalizeStop()  { if (IsEnabled()) LogJitFinalizeStop(); }

  void ConcurrentMarkingStart() {}
  void ConcurrentMarkingStop() {}

  void Deopt(const std::string& reason, const std::string& kind, const std::string& src, 
      const std::string& fn, int line, int column) {
    LogDeopt(reason, kind, src, fn, line, column);
  }

  void DisableOpt(const std::string& fn_name, const std::string& reason) {
    LogDisableOpt(fn_name, reason);
  }

  // CodeEventHandler is a special snowflake wire up to an event emitter
  static void CodeEventHandler(const JitCodeEvent* event);

 private:
  // All the implementation complexity lives outside the header (and doesn't
  // exist for NO_ETW builds).
  void LogMsg(const char* msg);
  void LogInitializePlatform();
  void LogShutdownPlatform();
  void LogInitializeV8();
  void LogTearDownV8();
  void LogIsolateStart(void* isolate);
  void LogIsolateStop(void* isolate);
  void LogSnapshotInitStart(void* isolate);
  void LogSnapshotInitStop(void* isolate);
  void LogParsingStart(void* isolate);
  void LogParsingStop(void* isolate);
  void LogGenerateUnoptimizedCodeStart(void* isolate);
  void LogGenerateUnoptimizedCodeStop(void* isolate);
  void LogJitExecuteStart();
  void LogJitExecuteStop();
  void LogJitFinalizeStart();
  void LogJitFinalizeStop();
  void LogCodeEvent(const JitCodeEvent* event);
  void LogDeopt(const std::string& reason, const std::string& kind, const std::string& src, 
      const std::string& fn, int line, int column);
  void LogDisableOpt(const std::string& fn_name, const std::string& reason);

  ScriptMapType* isolate_script_map;

#else  // defined(V8_ETW) && defined(V8_TARGET_OS_WIN)

// For NO_ETW, just provide inlined no-op versions of the public APIs
 public:
  void RegisterEtwProvider() {}
  void UnregisterEtwProvider() {}

  void Msg(const char* msg){}
  void InitializePlatform() {}
  void ShutdownPlatform() {}
  void InitializeV8() {}
  void TearDownV8() {}
  void IsolateStart(void* isolate) {}
  void IsolateStop(void* isolate) {}
  void SnapshotInitStart(void* isolate) {}
  void SnapshotInitStop(void* isolate) {}
  void ParsingStart(const char* filename, int offset) {}
  void ParsingStop() {}
  void GenerateUnoptimizedCodeStart() {}
  void GenerateUnoptimizedCodeStop() {}
  void JitExecuteStart() {}
  void JitExecuteStop() {}
  void JitFinalizeStart() {}
  void JitFinalizeStop() {}
  void ConcurrentMarkingStart() {}
  void ConcurrentMarkingStop() {}
  void Deopt(const std::string& reason, const std::string& kind, const std::string& src, 
      const std::string& fn, int line, int column) {}
  void DisableOpt(const std::string& fn_name, const std::string& reason) {}

  // CodeEventHandler is a special snowflake wire up to an event emitter
  static void CodeEventHandler(const JitCodeEvent* event) {}

#endif  // defined(V8_ETW) && defined(V8_TARGET_OS_WIN)

};

// Declare the global "etw::v8Provider" that is the instance of the provider
extern V8Provider v8Provider;

}  // namespace etw
}  // namespace internal
}  // namespace v8

#endif  // V8_TRACING_ETW_V8_PROVIDER_H_
