#ifndef _BOOL_MATRIX_H_
#define _BOOL_MATRIX_H_

#include <unordered_map>
#include <memory>
#include <boost/dynamic_bitset.hpp>

using BitSet = boost::dynamic_bitset<>;

template<typename T>
class BoolMatrix
{
public:

    BoolMatrix()
        : columnNum_(0),
          lineNum_(0)
    {}
    ~BoolMatrix() = default;
    BoolMatrix(const BoolMatrix &rhs) = delete;

    void addLine(const T &line);
    void deleteLine(const T &line);
    void addColumn();

    void set(unsigned int column, const T &line);
    void set(unsigned int column);
    void reset(unsigned int column);

    unsigned int columns() const { return columnNum_; }
    unsigned int lines() const { return lineNum_; }

    const BitSet* getBitset(const T &line) const;

private:
    typedef std::unordered_map<T, std::unique_ptr<BitSet>> LineMap;

    void setAll(unsigned int column, bool val);

    unsigned int columnNum_;
    unsigned int lineNum_;

    LineMap lineMap_;
};

template<typename T>
void BoolMatrix<T>::addLine(const T &line)
{
    auto it = lineMap_.find(line);
    if (it != lineMap_.end())
    {
        return;
    }

    std::unique_ptr<BitSet> bitset(new BitSet(columnNum_));
    bitset->reset();
    lineMap_[line] = std::move(bitset);
    lineNum_++;
}

template<typename T>
void BoolMatrix<T>::deleteLine(const T &line)
{
    auto it = lineMap_.find(line);
    if (it == lineMap_.end())
    {
        return;
    }

    lineMap_.erase(it);
    lineNum_--;
}

template<typename T>
const BitSet* BoolMatrix<T>::getBitset(const T &line) const
{
    auto it = lineMap_.find(line);
    if (it == lineMap_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

template<typename T>
void BoolMatrix<T>::addColumn()
{
    for (auto &it : lineMap_)
    {
        it.second->push_back(false);
    }

    columnNum_++;
}

template<typename T>
void BoolMatrix<T>::set(unsigned int column, const T &line)
{
    if (column < 0 || column >= columnNum_)
    {
        return;
    }

    auto it = lineMap_.find(line);
    if (it == lineMap_.end())
    {
        return;
    }

    it->second->set(column, true);
}

template<typename T>
void BoolMatrix<T>::set(unsigned int column)
{
    setAll(column, true);
}

template<typename T>
void BoolMatrix<T>::reset(unsigned int column)
{
    setAll(column, false);
}

template<typename T>
void BoolMatrix<T>::setAll(unsigned int column, bool val)
{
    if (column < 0 || column >= columnNum_)
    {
        return;
    }

    for (auto &it : lineMap_)
    {
        it.second->set(column, val);
    }
}

#endif
