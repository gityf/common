#pragma once
//#include <map>
#include <unordered_map>
#include <limits>

namespace common {
    /**
       Implements a lru cache
       */

    template <typename Key, typename Value>
    class LruCache
    {
        struct Data
        {
            unsigned serial;
            Value value;
            Data() { }
            Data(unsigned serial_, const Value& value_)
                : serial(serial_),
                value(value_)
            { }
        };

        //typedef std::map<Key, Data> DataType;
        typedef std::unordered_map<Key, Data> DataType;
        DataType data;

        typename DataType::size_type maxElements;
        unsigned serial;
        unsigned hits;
        unsigned misses;

        unsigned _nextSerial()
        {
            if (serial == std::numeric_limits<unsigned>::max())
            {
                for (typename DataType::iterator it = data.begin(); it != data.end(); ++it)
                    it->second.serial = 0;
                serial = 1;
            }

            return serial++;
        }

        typename DataType::iterator _getOldest()
        {
            typename DataType::iterator foundElement = data.begin();

            typename DataType::iterator it = data.begin();

            for (++it; it != data.end(); ++it)
            if (it->second.serial < foundElement->second.serial)
                foundElement = it;

            return foundElement;
        }

    public:
        typedef typename DataType::size_type size_type;
        typedef Value value_type;

        explicit LruCache(size_type maxElements_)
            : maxElements(maxElements_),
            serial(0),
            hits(0),
            misses(0)
        { }

        /// returns the number of elements currently in the cache
        size_type size() const        { return data.size(); }

        /// returns the maximum number of elements in the cache
        size_type getMaxElements() const      { return maxElements; }

        void setMaxElements(size_type maxElements_)
        {
            maxElements = maxElements_;
            while (data.size() > maxElements)
                data.erase(_getOldest());
        }

        /// removes a element from the cache and returns true, if found
        bool erase(const Key& key)
        {
            typename DataType::iterator it = data.find(key);
            if (it == data.end())
                return false;

            data.erase(it);
            return true;
        }

        /// clears the cache.
        void clear(bool stats = false)
        {
            data.clear();
            if (stats)
                hits = misses = 0;
        }

        /// puts a new element in the cache. If the element is already found in
        /// the cache, it is considered a cache hit and pushed to the top of the
        /// list.
        Value& put(const Key& key, const Value& value)
        {
            typename DataType::iterator it = data.find(key);
            if (it == data.end())
            {
                if (data.size() >= maxElements)
                    data.erase(_getOldest());

                it = data.insert(data.begin(),
                    typename DataType::value_type(key,
                    Data(_nextSerial(), value)));
            }
            else
            {
                // element found
                it->second.serial = _nextSerial();
            }

            return it->second.value;
        }

        Value* getptr(const Key& key)
        {
            typename DataType::iterator it = data.find(key);
            if (it == data.end())
            {
                ++misses;
                return 0;
            }

            it->second.serial = _nextSerial();

            ++hits;
            return &it->second.value;
        }

        /// returns a pair of values - a flag, if the value was found and the
        /// value if found or the passed default otherwise. If the value is
        /// found it is a cache hit and pushed to the top of the list.
        std::pair<bool, Value> getx(const Key& key, Value def = Value())
        {
            Value* v = getptr(key);
            return v ? std::pair<bool, Value>(true, *v)
                : std::pair<bool, Value>(false, def);
        }

        /// returns the value to a key or the passed default value if not found.
        /// If the value is found it is a cahce hit and pushed to the top of the
        /// list.
        Value get(const Key& key, Value def = Value())
        {
            return getx(key, def).second;
        }

        /// returns the number of hits.
        unsigned getHits() const    { return hits; }
        /// returns the number of misses.
        unsigned getMisses() const  { return misses; }
        /// returns the cache hit ratio between 0 and 1.
        double hitRatio() const     { return hits + misses > 0 ? static_cast<double>(hits) / static_cast<double>(hits + misses) : 0; }
        /// returns the ratio, between held elements and maximum elements.
        double fillfactor() const   { return static_cast<double>(data.size()) / static_cast<double>(maxElements); }

    };

}
