/*
 * Copyright 2017-2018 Scality
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __QUAD_ARRAY_VECTOR_H__
#define __QUAD_ARRAY_VECTOR_H__

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

#include "exceptions.h"
#include "simd/simd.h"

namespace quadiron {
namespace array {

/** A runtime-sized vector for storing a fixed size sequence of elements.
 *
 * Its size can be defined at runtime (unlike std::array) but cannot changes
 * after its creation (unlike std::vector).
 *
 * It is like a variable-length array (VLA) from C99, but allocated on the heap.
 *
 * @tparam T Type of element.
 */
template <typename T>
class Vector final {
  public:
    // Member types {{{

    using value_type = T;

    using pointer = value_type*;
    using const_pointer = const value_type*;

    using reference = value_type&;
    using const_reference = const value_type&;

    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using allocator_type = simd::AlignedAllocator<value_type>;

    // Ensure that our iterator types meet the requirements of
    // `RandomAccessIterator`.
    static_assert(
        std::is_same<
            typename std::iterator_traits<iterator>::iterator_category,
            std::random_access_iterator_tag>::value,
        "iterator is random access");
    static_assert(
        std::is_same<
            typename std::iterator_traits<const_iterator>::iterator_category,
            std::random_access_iterator_tag>::value,
        "const_iterator is random access");
    static_assert(
        std::is_same<
            typename std::iterator_traits<reverse_iterator>::iterator_category,
            std::random_access_iterator_tag>::value,
        "reverse_iterator is random access");
    static_assert(
        std::is_same<
            typename std::iterator_traits<
                const_reverse_iterator>::iterator_category,
            std::random_access_iterator_tag>::value,
        "const_reverse_iterator is random access");

    // }}}
    // Constructors/Destructor/Assignments {{{

    /** Construct an vector with enough space for `n` elements.
     *
     * @warning the content of the vector is uninitialized.
     *
     * @param[in] n vector size
     *
     * @par Complexity
     *
     * Linear in the number of elements `n`.
     */
    explicit Vector(size_type n)
        : m_data(n ? allocator.allocate(n) : nullptr), m_size(n)
    {
    }

    /** Construct an vector with `n` elements.
     *
     * @param[in] n vector size
     * @param[in] val value to fill the vector with
     *
     * @par Complexity
     *
     * Linear in the number of elements `n`.
     */
    Vector(size_type n, const value_type& val) : Vector(n)
    {
        std::uninitialized_fill_n(m_data, m_size, val);
    }

    /** Construct an vector with the elements from range [first,last[.
     *
     * The vector size will be the number of elements in the range [first,
     * last[.
     *
     * Each element is emplace-constructed from its corresponding element in
     * that range, in the same order.
     *
     * @param[in] first iterator to the initial position of the range to copy
     * @param[in] last iterator to the final position of the range to copy
     *
     * @note The third parameter is unused: its only purpose it to avoid
     * ambiguity with the overload Vector(size_type, const value_type&).
     *
     *
     * @par Complexity
     *
     * Linear in the number of elements in [first, last[.
     *
     * Note that the complexity is lower (`O(n)` instead of `O(2n)`) if
     * `InputIt` meets the requirements of `RandomAccessIterator`.
     */
    template <class InputIt>
    Vector(
        InputIt first,
        InputIt last,
        typename std::iterator_traits<InputIt>::iterator_category* = nullptr)
        : Vector(std::distance(first, last))
    {
        std::uninitialized_copy(first, last, begin());
    }

    /** Construct a vector by copying the elements from `vec`.
     *
     * The elements from `vec` are copied and their order is preserved.
     *
     * @param[in] vec another Vector object of the same type
     *
     * @par Complexity
     *
     * Linear in the number of elements in `vec`.
     */
    Vector(const Vector& vec) : Vector(vec.begin(), vec.end()) {}

    /** Construct a vector by acquiring the elements from `vec`.
     *
     * The ownership of the elements from `vec` is transfered: no copy.
     *
     * @note `vec` is left in an unspecified but valid state.
     *
     * @param[in,out] vec another Vector object of the same type
     *
     * @par Complexity
     *
     * Constant.
     */
    Vector(Vector&& vec) noexcept
        : m_data(std::exchange(vec.m_data, nullptr)),
          m_size(std::exchange(vec.m_size, 0))
    {
    }

    /** Construct an vector with the elements from an initializer list.
     *
     * The order of the elements is preserved.
     *
     * @param[in] init_list an initializer_list object
     *
     * @par Complexity
     *
     * Linear in the number of elements in `init_list`.
     */
    Vector(std::initializer_list<value_type> init_list)
        : Vector(init_list.begin(), init_list.end())
    {
    }

    /** Destroy each element, and deallocates the storage allocated.
     *
     * @par Complexity
     *
     * Linear in the number of elements in the vector.
     */
    ~Vector()
    {
        allocator.deallocate(m_data, m_size);
    }

    /** Assigns new contents to the vector, by copying elements from `vec`.
     *
     * @pre the size of `vec` must be the same as the current size.
     *
     * @param[in] vec another Vector object of the same type
     * @return *this
     *
     * @throw quadiron::InvalidArgument when `this->size() != vec.size()`
     *
     * @par Complexity
     *
     * Linear in the number of elements in `vec`.
     */
    Vector& operator=(const Vector& vec)
    {
        if (vec.m_size != m_size) {
            throw InvalidArgument("size mismatch in copy assignment");
        }

        Vector tmp(vec);
        swap(*this, tmp);
        return *this;
    }

    /** Assigns new contents to the vector, by acquiring elements from `vec`.
     *
     * The ownership of the elements from `vec` is transfered: no copy.
     *
     * @note `vec` is left in an unspecified but valid state.
     *
     * @pre the size of `vec` must be the same as the current size.
     *
     * @warning assigning two vectors of different size is undefined behavior
     *
     * @param[in,out] vec another Vector object of the same type
     * @return *this
     *
     * @par Complexity
     *
     * Constant.
     */
    Vector& operator=(Vector&& vec) noexcept
    {
        assert(vec.m_size == m_size);

        Vector tmp(std::move(vec));
        swap(tmp, *this);
        return *this;
    }

    /** Assign new contents to the vector, by copying elements from `init_list`.
     *
     * @pre the size of `init_list` must be the same as the current size.
     *
     * @param[in] init_list an initializer_list object
     * @return *this
     *
     * @throw quadiron::InvalidArgument when `this->size() != init_list.size()`
     *
     * @par Complexity
     *
     * Linear in the number of elements in `init_list`.
     */
    Vector& operator=(std::initializer_list<value_type> init_list)
    {
        if (init_list.size() != m_size) {
            throw InvalidArgument("size mismatch with the initializer list");
        }
        std::copy(init_list.begin(), init_list.end(), begin());
        return *this;
    }

    // }}}
    // Iterators {{{

    /** Return an iterator pointing to the first element.
     *
     * @warning if the vector is empty, the returned iterator value shall not be
     * dereferenced
     *
     * @return an iterator to the beginning of the vector
     *
     * @par Complexity
     *
     * Constant.
     */
    iterator begin() noexcept
    {
        return m_data;
    }

    /// @see Vector::begin
    const_iterator begin() const noexcept
    {
        return m_data;
    }

    /** Return an iterator pointing to the past-the-end element.
     *
     * @warning the returned iterator value shall not be dereferenced
     *
     * @return an iterator to the element past the end of the vector
     *
     * @par Complexity
     *
     * Constant.
     */
    iterator end() noexcept
    {
        return m_data + m_size;
    }

    /// @see Vector::end
    const_iterator end() const noexcept
    {
        return m_data + m_size;
    }

    /** Return a reverse iterator pointing to the last element.
     *
     * rbegin points to the element right before the one that would be pointed
     * to by end.
     *
     * @warning if the vector is empty, the returned iterator value shall not be
     * dereferenced
     *
     * @return a reverse iterator to the end of the vector
     *
     * @par Complexity
     *
     * Constant.
     */
    reverse_iterator rbegin() noexcept
    {
        return std::make_reverse_iterator(end());
    }

    /// @see Vector::rbegin
    const_reverse_iterator rbegin() const noexcept
    {
        return std::make_reverse_iterator(end());
    }

    /** Return a reverse iterator pointing to the theoretical element preceding
     * the first element.
     *
     * rend points to the element right before the one that would be pointed
     * to by begin.
     *
     * @warning the returned iterator value shall not be dereferenced
     *
     * @return a reverse iterator to the element before the beginning of the
     * vector
     *
     * @par Complexity
     *
     * Constant.
     */
    reverse_iterator rend() noexcept
    {
        return std::make_reverse_iterator(begin());
    }

    /// @see Vector::rend
    const_reverse_iterator rend() const noexcept
    {
        return std::make_reverse_iterator(begin());
    }

    /** Return an const iterator pointing to the first element.
     *
     * The returned iterator cannot be used to modify the contents it points to,
     * even if the vector object is not itself const.
     *
     * @warning if the vector is empty, the returned iterator value shall not be
     * dereferenced
     *
     * @return a const iterator to the beginning of the vector
     *
     * @par Complexity
     *
     * Constant.
     */
    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    /** Return an const iterator pointing to the past-the-end element.
     *
     * The returned iterator cannot be used to modify the contents it points to,
     * even if the vector object is not itself const.
     *
     * @warning the returned iterator value shall not be dereferenced
     *
     * @return a const iterator to the element past the end of the vector
     *
     * @par Complexity
     *
     * Constant.
     */
    const_iterator cend() const noexcept
    {
        return end();
    }

    /** Return a reverse iterator pointing to the last element.
     *
     * The returned iterator cannot be used to modify the contents it points to,
     * even if the vector object is not itself const.
     *
     * crbegin points to the element right before the one that would be pointed
     * to by cend.
     *
     * @warning if the vector is empty, the returned iterator value shall not be
     * dereferenced
     *
     * @return a reverse iterator to the end of the vector
     *
     * @par Complexity
     *
     * Constant.
     */
    const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    /** Return a reverse iterator pointing to the theoretical element preceding
     * the first element.
     *
     * The returned iterator cannot be used to modify the contents it points to,
     * even if the vector object is not itself const.
     *
     * crend points to the element right before the one that would be pointed
     * to by cbegin.
     *
     * @warning the returned iterator value shall not be dereferenced
     *
     * @return a reverse iterator to the element before the beginning of the
     * vector
     *
     * @par Complexity
     *
     * Constant.
     */
    const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    // }}}
    // Capacity {{{

    /** Return the number of elements in the vector.
     *
     * @par Complexity
     *
     * Constant.
     */
    size_type size() const noexcept
    {
        return m_size;
    }

    /** Return the maximum number of elements that the vector can hold.
     *
     * @note because the size of an vector is fixed we always have `max_size ==
     * size`.
     *
     * @par Complexity
     *
     * Constant.
     */
    size_type max_size() const noexcept
    {
        return m_size;
    }

    /** Return whether the vector is empty (i.e. whether its size is 0).
     *
     * @return true if the vector cannot contain any element.
     *
     * @par Complexity
     *
     * Constant.
     */
    bool empty() const noexcept
    {
        return m_size == 0;
    }

    // }}}
    // Element access {{{

    /** Return a reference to the element at position `idx` in the vector.
     *
     * @pre `idx < size()`.
     *
     * @warning no bound checking (using an invalid index is undefined
     * behavior).
     *
     * @par Complexity
     *
     * Constant.
     */
    reference operator[](size_type idx)
    {
        assert(idx < m_size);
        return m_data[idx];
    }

    /// @see Vector::operator[]
    const_reference operator[](size_type idx) const
    {
        assert(idx < m_size);
        return m_data[idx];
    }

    /** Return a reference to the element at position `idx` in the vector.
     *
     * @note this function performs bound-checking before accesing the value.
     *
     * @throw quadiron::OutOfRange when `idx >= size()`.
     *
     * @par Complexity
     *
     * Constant.
     */
    reference at(size_type idx)
    {
        if (idx >= m_size) {
            throw OutOfRange("Vector::at");
        }
        return m_data[idx];
    }

    /// @see Vector::at
    const_reference at(size_type idx) const
    {
        if (idx >= m_size) {
            throw OutOfRange("Vector::at");
        }
        return m_data[idx];
    }

    /** Return a reference to the first element in the vector.
     *
     * @pre the vector is not empty.
     * @warning calling this function on en empty vector us undefied behavior
     */
    reference front()
    {
        assert(!empty());
        return m_data[0];
    }

    /// @see Vector::front
    const_reference front() const
    {
        assert(!empty());
        return m_data[0];
    }

    /** Return a reference to the last element in the vector.
     *
     *
     * @pre the vector is not empty.
     * @warning calling this function on en empty vector us undefied behavior
     */
    reference back()
    {
        assert(!empty());
        return m_data[m_size - 1];
    }

    /// @see Vector::back
    const_reference back() const
    {
        assert(!empty());
        return m_data[m_size - 1];
    }

    /** Get a pointer to the data.
     *
     * @par Complexity
     *
     * Constant.
     */
    value_type* data() noexcept
    {
        return m_data;
    }

    /// @see Vector::data
    const value_type* data() const noexcept
    {
        return m_data;
    }

    // }}}
    // Non-member functions {{{
    // Comparison operators {{{

    /** Return true if `lhs` and `rhs` have the same size and content.
     *
     * @par Complexity
     *
     * Linear in the number of elements in the vectors.
     */
    friend bool operator==(const Vector& lhs, const Vector& rhs)
    {
        return lhs.m_size == rhs.m_size
               && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    /** Return true if `lhs` and `rhs` have different sizes or contents.
     *
     * @sa Vector::operator==
     *
     * @par Complexity
     *
     * Linear in the number of elements in the vectors.
     */
    friend bool operator!=(const Vector& lhs, const Vector& rhs)
    {
        return !(lhs == rhs);
    }

    /** Return true if `lhs` compares lexicographically less than `rhs`.
     *
     * Compare sequentially the elements that have the same position in both
     * vectors against each other until one element is not equivalent to the
     * other.
     * The result of comparing these first non-matching elements is the result
     * of the lexicographical comparison.
     *
     * If both vectors compare equal until one of them ends, the shorter vector
     * is lexicographically less than the longer one.
     *
     * @par Complexity
     *
     * Linear in the number of elements in the vectors.
     */
    friend bool operator<(const Vector& lhs, const Vector& rhs)
    {
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Return true if `lhs` compares lexicographically greater than `rhs`.
     *
     * It is defined as `rhs < lhs`.
     *
     * @sa Vector::operator<
     *
     * @par Complexity
     *
     * Linear in the number of elements in the vectors.
     */
    friend bool operator>(const Vector& lhs, const Vector& rhs)
    {
        return rhs < lhs;
    }

    /** Return true if `lhs` compares lexicographically less than or equal to
     * `rhs`.
     *
     * It is defined as `!(rhs < lhs)`.
     *
     * @sa Vector::operator<
     *
     * @par Complexity
     *
     * Linear in the number of elements in the vectors.
     */
    friend bool operator<=(const Vector& lhs, const Vector& rhs)
    {
        return !(rhs < lhs);
    }

    /** Return true if `lhs` compares lexicographically greater than or equal to
     * `rhs`.
     *
     * It is defined as `!(lhs < rhs)`.
     *
     * @sa Vector::operator<
     *
     * @par Complexity
     *
     * Linear in the number of elements in the vectors.
     */
    friend bool operator>=(const Vector& lhs, const Vector& rhs)
    {
        return !(lhs < rhs);
    }

    // }}}

    friend void swap(Vector<T>& lhs, Vector<T>& rhs) noexcept
    {
        // For ADL.
        using std::swap;

        swap(lhs.m_data, rhs.m_data);
        swap(lhs.m_size, rhs.m_size);
    }

    // }}}
  private:
    static allocator_type allocator;

    pointer m_data;
    size_type m_size;
};

template <typename T>
typename Vector<T>::allocator_type Vector<T>::allocator;

} // namespace array
} // namespace quadiron

#endif
