#pragma once

#include <map>
#include <tuple>
#include <vector>

// Ordered `std::map` with limited functionality. Suffices for our purposes.

namespace anitomy::detail::json {

template <typename K, typename V>
class Object final {
public:
  V& operator[](const K& key) {
    return data_[key];
  }

  bool contains(const K& key) const {
    return data_.contains(key);
  }

  bool empty() const {
    return data_.empty();
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    data_.emplace(std::forward<Args>(args)...);
    order_.emplace_back(std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)));
  }

  auto begin() {
    return order_.begin();
  }

  auto end() {
    return order_.end();
  }

private:
  std::map<K, V> data_;
  std::vector<K> order_;
};

}  // namespace anitomy::detail::json
