/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <cstdlib>
#include <memory>

#include "android-base/macros.h"

namespace android {
namespace util {

/**
 * Makes a std::unique_ptr<> with the template parameter inferred by the
 * compiler.
 * This will be present in C++14 and can be removed then.
 */
template <typename T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T{std::forward<Args>(args)...});
}

// Based on std::unique_ptr, but uses free() to release malloc'ed memory
// without incurring the size increase of holding on to a custom deleter.
template <typename T>
class unique_cptr {
 public:
  using pointer = typename std::add_pointer<T>::type;

  constexpr unique_cptr() : ptr_(nullptr) {}
  constexpr unique_cptr(std::nullptr_t) : ptr_(nullptr) {}
  explicit unique_cptr(pointer ptr) : ptr_(ptr) {}
  unique_cptr(unique_cptr&& o) : ptr_(o.ptr_) { o.ptr_ = nullptr; }

  ~unique_cptr() { std::free(reinterpret_cast<void*>(ptr_)); }

  inline unique_cptr& operator=(unique_cptr&& o) {
    if (&o == this) {
      return *this;
    }

    std::free(reinterpret_cast<void*>(ptr_));
    ptr_ = o.ptr_;
    o.ptr_ = nullptr;
    return *this;
  }

  inline unique_cptr& operator=(std::nullptr_t) {
    std::free(reinterpret_cast<void*>(ptr_));
    ptr_ = nullptr;
    return *this;
  }

  pointer release() {
    pointer result = ptr_;
    ptr_ = nullptr;
    return result;
  }

  inline pointer get() const { return ptr_; }

  void reset(pointer ptr = pointer()) {
    if (ptr == ptr_) {
      return;
    }

    pointer old_ptr = ptr_;
    ptr_ = ptr;
    std::free(reinterpret_cast<void*>(old_ptr));
  }

  inline void swap(unique_cptr& o) { std::swap(ptr_, o.ptr_); }

  inline explicit operator bool() const { return ptr_ != nullptr; }

  inline typename std::add_lvalue_reference<T>::type operator*() const { return *ptr_; }

  inline pointer operator->() const { return ptr_; }

  inline bool operator==(const unique_cptr& o) const { return ptr_ == o.ptr_; }

  inline bool operator!=(const unique_cptr& o) const { return ptr_ != o.ptr_; }

  inline bool operator==(std::nullptr_t) const { return ptr_ == nullptr; }

  inline bool operator!=(std::nullptr_t) const { return ptr_ != nullptr; }

 private:
  DISALLOW_COPY_AND_ASSIGN(unique_cptr);

  pointer ptr_;
};

inline uint32_t fix_package_id(uint32_t resid, uint8_t package_id) {
  return resid | (static_cast<uint32_t>(package_id) << 24);
}

inline uint8_t get_package_id(uint32_t resid) {
  return static_cast<uint8_t>((resid >> 24) & 0x000000ffu);
}

// The type ID is 1-based, so if the returned value is 0 it is invalid.
inline uint8_t get_type_id(uint32_t resid) {
  return static_cast<uint8_t>((resid >> 16) & 0x000000ffu);
}

inline uint16_t get_entry_id(uint32_t resid) { return static_cast<uint16_t>(resid & 0x0000ffffu); }

inline bool is_internal_resid(uint32_t resid) {
  return (resid & 0xffff0000u) != 0 && (resid & 0x00ff0000u) == 0;
}

inline bool is_valid_resid(uint32_t resid) {
  return (resid & 0x00ff0000u) != 0 && (resid & 0xff000000u) != 0;
}

void ReadUtf16StringFromDevice(const uint16_t* src, size_t len, std::string* out);

}  // namespace util
}  // namespace android

#endif /* UTIL_H_ */
