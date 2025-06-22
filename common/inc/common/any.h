#pragma once
// std::any can't store noncopyable types

#include <memory>
#include <utility>

namespace common {

class Any {
 public:
  template<typename T, typename... Args>
  static Any Make(Args&&... args);

  template<typename T>
  T& GetUnchecked();

 private:
  struct Holder {
    virtual ~Holder() = default;
  };

  template<typename T>
  struct HolderImpl : public Holder {
    T value;

    template<typename... Args>
    HolderImpl(Args&&... args);
  };

  std::unique_ptr<Holder> data_;
};

template<typename T, typename... Args>
Any Any::Make(Args&&... args) {
  Any any;
  any.data_ = std::make_unique<HolderImpl<T>>(std::forward<Args>(args)...);
  return any;
}

template<typename T>
T& Any::GetUnchecked() {
  return reinterpret_cast<HolderImpl<T>*>(data_.get())->value;
}

template<typename T>
template<typename... Args>
Any::HolderImpl<T>::HolderImpl(Args&&... args)
  : value{std::forward<Args>(args)...}
  {}

} // namespace common
