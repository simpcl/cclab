#ifndef _RESOURCE_CACHE_H_
#define _RESOURCE_CACHE_H_

#include <cassert>
#include <stdint.h>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <iostream>

using namespace std;

template<typename T>
class ResourceCache {
  const int kCacheExpireSeconds = 30;

  struct ResourceCacheItem {
    explicit ResourceCacheItem(const std::string& k, std::shared_ptr<T> r) :
      key(k), res(r), prev(nullptr), next(nullptr) {
      last_modified = std::chrono::system_clock::now();
    }

    std::string key;
    std::shared_ptr<T> res;
    std::chrono::system_clock::time_point last_modified;
    ResourceCacheItem* prev;
    ResourceCacheItem* next;
  };

 public:
  ResourceCache(int capacity, int expire_seconds);
  ResourceCache(int capacity);
  ~ResourceCache();

  std::shared_ptr<T> Get(const std::string& key);
  bool Set(const std::string& key, std::shared_ptr<T> res);

 protected:
  void InsertToLRU(ResourceCacheItem* item);
  void RemoveFromLRU(ResourceCacheItem* item);
  void MoveToLRUHead(ResourceCacheItem* item);

 private:
  int capacity_;
  std::unordered_map<std::string, ResourceCacheItem*> cache_map_;
  std::mutex cache_mutex_;
  ResourceCacheItem lru_head_;
  int expire_seconds_;
};

template<typename T>
ResourceCache<T>::ResourceCache(int capacity, int expire_seconds) :
  capacity_(capacity), lru_head_("", nullptr), expire_seconds_(expire_seconds) {
  lru_head_.next = &lru_head_;
  lru_head_.prev = &lru_head_;
}

template<typename T>
ResourceCache<T>::ResourceCache(int capacity) : capacity_(capacity),
  lru_head_("", nullptr), expire_seconds_(kCacheExpireSeconds) {
  lru_head_.next = &lru_head_;
  lru_head_.prev = &lru_head_;
}

template<typename T>
ResourceCache<T>::~ResourceCache() {
  std::lock_guard<std::mutex> guard(cache_mutex_);
  for (auto& item : cache_map_) {
    delete item.second;
  }
  cache_map_.clear();
}

template<typename T>
std::shared_ptr<T> ResourceCache<T>::Get(const std::string& key) {
  std::lock_guard<std::mutex> guard(cache_mutex_);
  auto iter = cache_map_.find(key);
  if (iter != cache_map_.end()) {
    std::cout << key << " cache found!" << std::endl;
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - iter->second->last_modified);
    std::cout << "diff: " << diff.count() << ", expire: " << expire_seconds_ << std::endl;
    if (diff.count() < expire_seconds_) {
      return iter->second->res;
    }
  }
  return nullptr;
}

template<typename T>
bool ResourceCache<T>::Set(const std::string& key, std::shared_ptr<T> res) {
  std::lock_guard<std::mutex> guard(cache_mutex_);
  auto iter = cache_map_.find(key);
  if (iter != cache_map_.end()) {
    ResourceCacheItem* item = iter->second;
    item->res = res;
    item->last_modified = std::chrono::system_clock::now();
    MoveToLRUHead(item);
    return true;
  }

  if (cache_map_.size() == capacity_) {
    ResourceCacheItem* rear = lru_head_.prev;
    cache_map_.erase(rear->key);
    RemoveFromLRU(rear);
  }

  ResourceCacheItem* item = new ResourceCacheItem(key, res);
  if (!item) {
    return false;
  }
  cache_map_[key] = item;
  InsertToLRU(item);

  return true;
}

template<typename T>
void ResourceCache<T>::InsertToLRU(ResourceCacheItem* item) {
  assert(item != nullptr);

  item->next = lru_head_.next;
  item->prev = &lru_head_;

  lru_head_.next->prev = item;
  lru_head_.next = item;
}

template<typename T>
void ResourceCache<T>::RemoveFromLRU(ResourceCacheItem* item) {
  assert(item != nullptr &&
         item->next != nullptr &&
         item->prev != nullptr);
  item->prev->next = item->next;
  item->next->prev = item->prev;
  delete item;
}

template<typename T>
void ResourceCache<T>::MoveToLRUHead(ResourceCacheItem* item) {
  assert(item != nullptr &&
         item->next != nullptr &&
         item->prev != nullptr);
  item->prev->next = item->next;
  item->next->prev = item->prev;

  item->next = lru_head_.next;
  item->prev = &lru_head_;
  lru_head_.next->prev = item;
  lru_head_.next = item;
}

#endif // _RESOURCE_CACHE_H_
