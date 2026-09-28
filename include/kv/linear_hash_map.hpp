#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

template <typename K, typename V>
class LinearHashMap {
public:
    explicit LinearHashMap(std::size_t capacity)
        : map_(capacity), capacity_(capacity), size_(0) {}

    void insert(const K& key, const V& value) {
        std::size_t idx = probe(key);

        if (map_[idx].state == State::OCCUPIED) {
            map_[idx].val = value;
            return;
        }

        map_[idx] = Entry{key, value, State::OCCUPIED};
        ++size_;
    }

    V& find(const K& key) {
        std::size_t idx = std::hash<K>{}(key) % capacity_;

        for (std::size_t i = 0; i < capacity_; ++i) {
            if (map_[idx].state == State::EMPTY) {
                throw std::out_of_range("key not found");
            }

            if (map_[idx].state == State::OCCUPIED &&
                map_[idx].key == key) {
                return map_[idx].val;
            }

            idx = (idx + 1) % capacity_;
        }

        throw std::out_of_range("key not found");
    }

    void erase(const K& key) {
        std::size_t idx = std::hash<K>{}(key) % capacity_;

        for (std::size_t i = 0; i < capacity_; ++i) {
            if (map_[idx].state == State::EMPTY) {
                return;
            }

            if (map_[idx].state == State::OCCUPIED &&
                map_[idx].key == key) {
                map_[idx].state = State::DELETED;
                --size_;
                return;
            }

            idx = (idx + 1) % capacity_;
        }
    }

private:
    enum class State {
        EMPTY,
        OCCUPIED,
        DELETED
    };

    struct Entry {
        K key{};
        V val{};
        State state = State::EMPTY;
    };

    std::size_t probe(const K& key) {
        std::size_t idx = std::hash<K>{}(key) % capacity_;
        std::optional<std::size_t> first_deleted;

        for (std::size_t i = 0; i < capacity_; ++i) {
            if (map_[idx].state == State::EMPTY) {
                return first_deleted.value_or(idx);
            }

            if (map_[idx].state == State::DELETED) {
                if (!first_deleted) {
                    first_deleted = idx;
                }
            } else if (map_[idx].key == key) {
                return idx;
            }

            idx = (idx + 1) % capacity_;
        }

        if (first_deleted) {
            return *first_deleted;
        }

        throw std::overflow_error("hash map full");
    }

    std::vector<Entry> map_;
    std::size_t capacity_;
    std::size_t size_;
};
