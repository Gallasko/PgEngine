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
        /**
         * @brief An Iterator for iterating over the elements of a sparse set
         * 
         * This iterator is a read only iterator that iterates over the elements ot the sparse set
         * Using the operator* you can obtain 
         */
        class Iterator
        {
        friend class SparseSet;
        public:
            //Pre Increment
            inline Iterator& operator++() { index++; return *this; }
            
            //Post Increment
            inline Iterator operator++(int) { Iterator old = *this; index++; return old; }

            inline bool operator==(const Iterator& rhs) const { return index == rhs.index; } 
            inline bool operator!=(const Iterator& rhs) const { return index != rhs.index; } 

            inline uint64 operator*() const { return (*dense)[index]; }

        protected:
            Iterator(size_t pos, std::vector<uint64> *dense) : index(pos), dense(dense) {}

        private:
            size_t index = 0;
            std::vector<uint64> *dense;
        };

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

        void insert(const uint64& value)
        {
            if(size >= dense.capacity())
                dense.reserve(size * 2);

            if(value >= sparse.capacity())
                sparse.reserve(value * 2);

            dense[size] = value;
            sparse[value] = size;

            size++;
        }

        void remove(const uint64& value)
        {
            if(size < 1 && !has(value))
                return;

            dense[sparse[value]] = dense[sparse[size - 1]];
            sparse[value] = sparse[size - 1];

            size--;
        }

        void removeAt(const size_t& index)
        {
            if(size < 1 && index >= size) 
                return;

            // Put the last value where the removed value is, to keep a contiguous dense array
            // and update the sparse array accordingly.  
            sparse[dense[index]] = sparse[dense[size -1]];
            dense[index] = dense[size - 1];

            size--;
        }

        void clear()
        {
            size = 0;
        }

        inline Iterator begin() { return Iterator(0, &dense); }
        inline Iterator end() { return Iterator(size, &dense); }

    private:
        std::vector<uint64> dense;
        std::vector<uint64> sparse;

        std::size_t size = 0;
        std::size_t sparseCapacity = 1;
    };
}