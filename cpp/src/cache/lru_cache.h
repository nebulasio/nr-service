#pragma once

#include "common.h"
#include <list>
#include <mutex>

namespace neb {

template <typename K, typename V> struct key_value_pair_t {
public:
  K m_key;
  V m_value;
  key_value_pair_t(const K &k, const V &v) : m_key(k), m_value(v) {}
};

template <class Key, class Value, class Lock = std::mutex,
          class Map = std::unordered_map<
              Key, typename std::list<key_value_pair_t<Key, Value>>::iterator>>
class lru_cache {
public:
  typedef key_value_pair_t<Key, Value> node_t;
  typedef std::list<node_t> list_t;
  typedef Map map_t;
  typedef Lock lock_t;
  using guard_t = std::lock_guard<lock_t>;

  explicit lru_cache(size_t size) : m_cache_max_size(size) {}
  virtual ~lru_cache() = default;

  size_t size() const {
    guard_t g(m_lock);
    return m_cache_map.size();
  }

  bool empty() const {
    guard_t g(m_lock);
    return m_cache_map.empty();
  }

  void clear() {
    guard_t g(m_lock);
    m_cache_map.clear();
    m_cache_list.clear();
  }

  void set(const Key &k, const Value &v) {
    guard_t g(m_lock);
    const auto iter = m_cache_map.find(k);
    if (iter != m_cache_map.end()) {
      return;
    }

    m_cache_list.emplace_front(k, v);
    m_cache_map[k] = m_cache_list.begin();

    if (m_cache_map.size() > m_cache_max_size) {
      m_cache_map.erase(m_cache_list.back().m_key);
      m_cache_list.pop_back();
    }
  }

  bool get(const Key &k_in, Value &v_out) {
    guard_t g(m_lock);
    const auto iter = m_cache_map.find(k_in);
    if (iter == m_cache_map.end()) {
      return false;
    }
    m_cache_list.splice(m_cache_list.begin(), m_cache_list, iter->second);
    v_out = iter->second->m_value;
    return true;
  }

  bool exists(const Key &k) const {
    guard_t g(m_lock);
    return m_cache_map.find(k) != m_cache_map.end();
  }

  template <typename F> void watch(F &f) const {
    guard_t g(m_lock);
    std::for_each(m_cache_list.begin(), m_cache_list.end(), f);
  }

private:
  mutable Lock m_lock;
  Map m_cache_map;
  list_t m_cache_list;
  size_t m_cache_max_size;
};

} // namespace neb
