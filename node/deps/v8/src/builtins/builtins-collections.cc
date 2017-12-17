// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/builtins/builtins-utils.h"
#include "src/builtins/builtins.h"
#include "src/objects-inl.h"

namespace v8 {
namespace internal {

BUILTIN(MapClear) {
  HandleScope scope(isolate);
  const char* const kMethodName = "Map.prototype.clear";
  CHECK_RECEIVER(JSMap, map, kMethodName);
  JSMap::Clear(map);
  return isolate->heap()->undefined_value();
}

BUILTIN(SetClear) {
  HandleScope scope(isolate);
  const char* const kMethodName = "Set.prototype.clear";
  CHECK_RECEIVER(JSSet, set, kMethodName);
  JSSet::Clear(set);
  return isolate->heap()->undefined_value();
}

}  // namespace internal
}  // namespace v8