#pragma once

#include <utility>

namespace common {

template<typename T>
class Uninit {
 public:
  template<typename... Args>
  void Construct(Args&&... args);
  void Destruct();

  T& value();
  const T& value() const;
 
 private:
  alignas(T) char _raw[sizeof(T)];
};  // struct Uninit

template<typename T>
template<typename ...Args>
void Uninit<T>::Construct(Args&&... args) {
  new (_raw) T(std::forward<Args>(args)...);
}

template<typename T>
void Uninit<T>::Destruct() {
  value().~T();
}

template<typename T>
T& Uninit<T>::value() {
  return *reinterpret_cast<T*>(_raw);
}

template<typename T>
const T& Uninit<T>::value() const {
  return *reinterpret_cast<const T*>(_raw);
}

}  // namespace common
