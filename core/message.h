#pragma once

#include <string.h>

#define STATUSTYPE core::Status
#define STATUSRETURN(M) \
  return core::Status(core::Status::Code::INTERNAL, (M))
#define STATUSSUCCESS core::Status::Success
#include "common/json.h"

namespace core {

class Message {
 public:
  explicit Message(const common::Json::Value& msg) {
    json_buffer_.Clear();
    msg.Write(&json_buffer_);
    base_ = json_buffer_.Base();
    byte_size_ = json_buffer_.Size();
    from_json_ = true;
  }

  explicit Message(std::string&& msg) {
    str_buffer_ = std::move(msg);
    base_ = str_buffer_.data();
    byte_size_ = str_buffer_.size();
    from_json_ = false;
  }

  Message(const Message& rhs) {
    from_json_ = rhs.from_json_;
    if (from_json_) {
      json_buffer_ = rhs.json_buffer_;
      base_ = json_buffer_.Base();
      byte_size_ = json_buffer_.Size();
    } else {
      str_buffer_ = rhs.str_buffer_;
      base_ = str_buffer_.data();
      byte_size_ = str_buffer_.size();
    }
  }

  void Serialize(const char** base, size_t* byte_size) const {
    *base = base_;
    *byte_size = byte_size_;
  }

 private:
  const char* base_;
  size_t byte_size_;

  bool from_json_;
  std::string str_buffer_;
  common::Json::WriteBuffer json_buffer_;
};

}  // namespace core