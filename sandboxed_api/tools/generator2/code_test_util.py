# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Contains golden outputs for tests."""


CODE_GOLD = """// AUTO-GENERATED by the Sandboxed API generator.
// Edits will be discarded when regenerating this file.

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "sandboxed_api/sandbox.h"
#include "sandboxed_api/util/status_macros.h"
#include "sandboxed_api/vars.h"

namespace sapi {
namespace Tests {

class TestApi {
 public:
  explicit TestApi(::sapi::Sandbox* sandbox) : sandbox_(sandbox) {}
  // Deprecated
  ::sapi::Sandbox* GetSandbox() const { return sandbox(); }
  ::sapi::Sandbox* sandbox() const { return sandbox_; }

  // int function_a(int, int)
  absl::StatusOr<int> function_a(int x, int y) {
    ::sapi::v::Int ret;
    ::sapi::v::Int x_((x));
    ::sapi::v::Int y_((y));

    SAPI_RETURN_IF_ERROR(sandbox_->Call("function_a", &ret, &x_, &y_));
    return ret.GetValue();
  }

  // int types_1(bool, unsigned char, char, unsigned short, short)
  absl::StatusOr<int> types_1(bool a0, unsigned char a1, char a2, unsigned short a3, short a4) {
    ::sapi::v::Int ret;
    ::sapi::v::Bool a0_((a0));
    ::sapi::v::UChar a1_((a1));
    ::sapi::v::Char a2_((a2));
    ::sapi::v::UShort a3_((a3));
    ::sapi::v::Short a4_((a4));

    SAPI_RETURN_IF_ERROR(sandbox_->Call("types_1", &ret, &a0_, &a1_, &a2_, &a3_, &a4_));
    return ret.GetValue();
  }

  // int types_2(int, unsigned int, long, unsigned long)
  absl::StatusOr<int> types_2(int a0, unsigned int a1, long a2, unsigned long a3) {
    ::sapi::v::Int ret;
    ::sapi::v::Int a0_((a0));
    ::sapi::v::UInt a1_((a1));
    ::sapi::v::Long a2_((a2));
    ::sapi::v::ULong a3_((a3));

    SAPI_RETURN_IF_ERROR(sandbox_->Call("types_2", &ret, &a0_, &a1_, &a2_, &a3_));
    return ret.GetValue();
  }

  // int types_3(long long, unsigned long long, float, double)
  absl::StatusOr<int> types_3(long long a0, unsigned long long a1, float a2, double a3) {
    ::sapi::v::Int ret;
    ::sapi::v::LLong a0_((a0));
    ::sapi::v::ULLong a1_((a1));
    ::sapi::v::Reg<float> a2_((a2));
    ::sapi::v::Reg<double> a3_((a3));

    SAPI_RETURN_IF_ERROR(sandbox_->Call("types_3", &ret, &a0_, &a1_, &a2_, &a3_));
    return ret.GetValue();
  }

  // int types_4(signed char, short, int, long)
  absl::StatusOr<int> types_4(signed char a0, short a1, int a2, long a3) {
    ::sapi::v::Int ret;
    ::sapi::v::SChar a0_((a0));
    ::sapi::v::Short a1_((a1));
    ::sapi::v::Int a2_((a2));
    ::sapi::v::Long a3_((a3));

    SAPI_RETURN_IF_ERROR(sandbox_->Call("types_4", &ret, &a0_, &a1_, &a2_, &a3_));
    return ret.GetValue();
  }

  // int types_5(long long, long double)
  absl::StatusOr<int> types_5(long long a0, long double a1) {
    ::sapi::v::Int ret;
    ::sapi::v::LLong a0_((a0));
    ::sapi::v::Reg<long double> a1_((a1));

    SAPI_RETURN_IF_ERROR(sandbox_->Call("types_5", &ret, &a0_, &a1_));
    return ret.GetValue();
  }

  // void types_6(char *)
  absl::Status types_6(::sapi::v::Ptr* a0) {
    ::sapi::v::Void ret;

    SAPI_RETURN_IF_ERROR(sandbox_->Call("types_6", &ret, a0));
    return absl::OkStatus();
  }

 private:
  ::sapi::Sandbox* sandbox_;
};

}  // namespace Tests
}  // namespace sapi
"""

CODE_GOLD_MAPPED = """// AUTO-GENERATED by the Sandboxed API generator.
// Edits will be discarded when regenerating this file.

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "sandboxed_api/sandbox.h"
#include "sandboxed_api/util/status_macros.h"
#include "sandboxed_api/vars.h"

namespace sapi {
namespace Tests {

typedef unsigned int uint;
typedef uint * uintp;

class TestApi {
 public:
  explicit TestApi(::sapi::Sandbox* sandbox) : sandbox_(sandbox) {}
  // Deprecated
  ::sapi::Sandbox* GetSandbox() const { return sandbox(); }
  ::sapi::Sandbox* sandbox() const { return sandbox_; }

  // uint function(uintp)
  absl::StatusOr<uint> function(::sapi::v::Ptr* a) {
    ::sapi::v::UInt ret;

    SAPI_RETURN_IF_ERROR(sandbox_->Call("function", &ret, a));
    return ret.GetValue();
  }

 private:
  ::sapi::Sandbox* sandbox_;
};

}  // namespace Tests
}  // namespace sapi
"""

CODE_ENUM_GOLD = """// AUTO-GENERATED by the Sandboxed API generator.
// Edits will be discarded when regenerating this file.

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "sandboxed_api/sandbox.h"
#include "sandboxed_api/util/status_macros.h"
#include "sandboxed_api/vars.h"

namespace sapi {
namespace Tests {

enum ProcessStatus {
\tOK = 0 ,
\tERROR = 1 ,
};

class TestApi {
 public:
  explicit TestApi(::sapi::Sandbox* sandbox) : sandbox_(sandbox) {}
  // Deprecated
  ::sapi::Sandbox* GetSandbox() const { return sandbox(); }
  ::sapi::Sandbox* sandbox() const { return sandbox_; }

  // ProcessStatus ProcessDatapoint(ProcessStatus)
  absl::StatusOr<ProcessStatus> ProcessDatapoint(ProcessStatus status) {
    ::sapi::v::IntBase<ProcessStatus> ret;
    ::sapi::v::IntBase<ProcessStatus> status_((status));

    SAPI_RETURN_IF_ERROR(sandbox_->Call("ProcessDatapoint", &ret, &status_));
    return static_cast<ProcessStatus>(ret.GetValue());
  }

 private:
  ::sapi::Sandbox* sandbox_;
};

}  // namespace Tests
}  // namespace sapi
"""
