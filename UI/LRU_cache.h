#ifndef _LRUCACHE_HPP_INCLUDED_
#define	_LRUCACHE_HPP_INCLUDED_

#include <unordered_map>
#include <list>
#include <stdexcept>

#include <memory_resource>

template<typename T>
class PoolList : private std::pmr::monotonic_buffer_resource, // 1. 内存池作为第一个基类（最先初始化）
                 public std::pmr::list<T>                     // 2. list 作为第二个基类（后初始化）
{
private:
    // 定义一块专属的、固定大小的物理内存缓冲区（例如 4 KB）
    alignas(alignof(T)) std::byte buffer[4096]{};

public:
    // 构造函数初始化列表：
    // 先用物理缓冲区初始化内存池基类，然后将内存池指针（this）传给 list 基类
    PoolList()
        : std::pmr::monotonic_buffer_resource(buffer, sizeof(buffer)),
          std::pmr::list<T>(static_cast<std::pmr::monotonic_buffer_resource *>(this)) {
    }

    // 禁用拷贝构造和赋值，防止内部内存池指针错乱
    PoolList(const PoolList &) = delete;

    PoolList &operator=(const PoolList &) = delete;

    // 支持移动构造
    PoolList(PoolList &&) noexcept = default;
};


template<typename key_t, typename value_t>
class LRU_cache {
public:
    using key_value_pair_t = std::pair<key_t, value_t>;
    using list_iterator_t  = typename std::list<key_value_pair_t>::iterator;

    explicit LRU_cache(const size_t max_size) : _max_size(max_size) {
        // _cache_items_list.reserve(_max_size);
    }

    LRU_cache() = delete;


    std::optional<std::pair<key_t, value_t> > put(const key_t &key, const value_t &value) {
        auto it = _cache_items_map.find(key);
        _cache_items_list.push_front(key_value_pair_t(key, value));
        if (it != _cache_items_map.end()) {
            _cache_items_list.erase(it->second);
            _cache_items_map.erase(it);
        }
        _cache_items_map[key] = _cache_items_list.begin();

        if (_cache_items_map.size() > _max_size) {
            auto last = _cache_items_list.end();
            // last 其实是指向了哨兵
            --last;
            std::pair<key_t, value_t> evicted_kv;
            evicted_kv = *last;
            _cache_items_map.erase(last->first);
            _cache_items_list.pop_back();
            return evicted_kv;
        }
        return {};
    }

    const value_t &get(const key_t &key) {
        auto it = _cache_items_map.find(key);
        if (it == _cache_items_map.end()) {
            throw std::range_error("There is no such key in cache");
        } else {
            _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
            return it->second->second;
        }
    }

    bool exists(const key_t &key) const {
        return _cache_items_map.contains(key);
    }

    size_t size() const {
        return _cache_items_map.size();
    }

private:
    PoolList<key_value_pair_t> _cache_items_list;
    std::unordered_map<key_t, list_iterator_t> _cache_items_map;
    size_t _max_size;
};

#endif	/* _LRUCACHE_HPP_INCLUDED_ */
