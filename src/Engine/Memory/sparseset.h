#pragma once

#include <cstdint>
#include <vector>

namespace pg
{
    typedef uint_fast16_t uint64; 

    // A value of 0 indicates that nothing was found

    class SparseSet
    {
    public:
        bool has(const uint64& value) const { return value < sparse.capacity() && sparse[value] < dense.capacity() && dense[sparse[value]] == value; };

        uint64 at(const size_t& index) const
        {
            if(index >= size) 
                return 0;
            
            return dense[index];
        }

        uint64 find(const uint64& value) const
        {
            if(has(value))
                return sparse[value];

            return 0;
        }

        void push_back(const uint64& value)
        {
            if(size >= dense.capacity())
                dense.reserve(size * 2);

            if(value >= sparse.capacity())
                sparse.reserve(value * 2);

            dense[size] = value;
            sparse[value] = size;

            size++;
        }

        void removeAt(const size_t& index)
        {
            if(index >= size) 
                return;

            
        }















    private:
        std::vector<uint64> dense;
        std::vector<uint64> sparse;

        std::size_t size = 0;
        std::size_t sparseCapacity = 1;
    };
}