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

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "array/array.h"
#include "exceptions.h"

namespace array = quadiron::array;

TEST(ArrayVectorTest, TestSimpleInitEmpty) // NOLINT
{
    const array::Vector<int> vec(0);

    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.max_size(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST(ArrayVectorTest, TestSimpleInit) // NOLINT
{
    const array::Vector<int> vec(42);

    EXPECT_EQ(vec.size(), 42);
    EXPECT_EQ(vec.max_size(), 42);
    EXPECT_FALSE(vec.empty());
}

TEST(ArrayVectorTest, TestFillInitEmpty) // NOLINT
{
    const array::Vector<int> vec(0, 42);

    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.max_size(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST(ArrayVectorTest, TestFillInit) // NOLINT
{
    const std::vector<int> expected(10, 42);
    const array::Vector<int> vec(10, 42);
    const std::vector<int> result(vec.begin(), vec.end());

    EXPECT_EQ(vec.size(), 10);
    EXPECT_EQ(vec.max_size(), 10);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(result, expected);
}

TEST(ArrayVectorTest, TestRangeInitEmpty) // NOLINT
{
    const std::vector<int> empty;
    const array::Vector<int> vec(empty.begin(), empty.end());

    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.max_size(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST(ArrayVectorTest, TestRangeInit) // NOLINT
{
    const std::vector<int> expected{1, 3, 5, 7, 10};
    const array::Vector<int> vec(expected.begin(), expected.end());
    const std::vector<int> result(vec.begin(), vec.end());

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.max_size(), 5);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(result, expected);
}

TEST(ArrayVectorTest, TestRangeInitPartial) // NOLINT
{
    const std::vector<int> source{1, 3, 5, 7, 10};
    const std::vector<int> expected{3, 5, 7};
    const array::Vector<int> vec(source.begin() + 1, source.end() - 1);
    const std::vector<int> result(vec.begin(), vec.end());

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.max_size(), 3);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(result, expected);
}

TEST(ArrayVectorTest, TestCopyInitEmpty) // NOLINT
{
    const array::Vector<int> vec1(0);
    const array::Vector<int> vec2(vec1);

    EXPECT_EQ(vec1, vec2);
}

TEST(ArrayVectorTest, TestCopyInit) // NOLINT
{
    const array::Vector<int> vec1(777, 42);
    const array::Vector<int> vec2(vec1);

    EXPECT_EQ(vec1, vec2);
}

TEST(ArrayVectorTest, TestMoveInitEmpty) // NOLINT
{
    array::Vector<int> vec1(0);
    const int* data = vec1.data();
    const array::Vector<int> vec2(std::move(vec1));

    // The buffer moved from `vec1` to `vec2`.
    EXPECT_EQ(vec2.data(), data);
}

TEST(ArrayVectorTest, TestMoveInit) // NOLINT
{
    array::Vector<int> vec1(777, 42);
    const int* data = vec1.data();
    const array::Vector<int> vec2(std::move(vec1));

    // The buffer moved from `vec1` to `vec2`.
    EXPECT_EQ(vec2.data(), data);
}

TEST(ArrayVectorTest, TestInitListInitEmpty) // NOLINT
{
    const std::vector<int> expected{};
    const array::Vector<int> vec{};
    const std::vector<int> result(vec.begin(), vec.end());

    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.max_size(), 0);
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(result, expected);
}

TEST(ArrayVectorTest, TestInitListInit) // NOLINT
{
    const std::vector<int> expected{3, 5, 7};
    const array::Vector<int> vec{3, 5, 7};
    const std::vector<int> result(vec.begin(), vec.end());

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.max_size(), 3);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(result, expected);
}

TEST(ArrayVectorTest, TestCopyAssignEmpty) // NOLINT
{
    array::Vector<int> vec1{};
    const array::Vector<int> vec2(0);

    vec1 = vec2;
    EXPECT_EQ(vec1, vec2);
}

TEST(ArrayVectorTest, TestCopyAssign) // NOLINT
{
    array::Vector<int> vec1{1, 2};
    const array::Vector<int> vec2{3, 4};

    EXPECT_NE(vec1, vec2);
    vec1 = vec2;
    EXPECT_EQ(vec1, vec2);
}

TEST(ArrayVectorTest, TestCopySelfAssign) // NOLINT
{
    array::Vector<int> vec{1, 2};
    array::Vector<int>* pvec = &vec;

    EXPECT_EQ(vec, vec);
    // We have to use pointer here, in order to fool a strict compiler
    // (which warns on simple case such as `vec = vec`).
    vec = *pvec;
    EXPECT_EQ(vec, vec);
}

TEST(ArrayVectorTest, TestCopyAssignSizeMismatch) // NOLINT
{
    array::Vector<int> vec1{1, 2};
    const array::Vector<int> vec2{3};

    ASSERT_THROW(vec1 = vec2, quadiron::InvalidArgument);
}

TEST(ArrayVectorTest, TestMoveAssignEmpty) // NOLINT
{
    array::Vector<int> vec1{};
    array::Vector<int> vec2(0);
    const int* data = vec2.data();

    // The buffer moved from `vec1` to `vec2`.
    EXPECT_EQ(vec2.data(), data);
    vec1 = std::move(vec2);
    EXPECT_EQ(vec1.data(), data);
}

TEST(ArrayVectorTest, TestMoveAssign) // NOLINT
{
    array::Vector<int> vec1{1, 2};
    array::Vector<int> vec2{3, 4};
    const int* data = vec2.data();

    // The buffer moved from `vec1` to `vec2`.
    EXPECT_NE(vec1.data(), data);
    EXPECT_EQ(vec2.data(), data);
    vec1 = std::move(vec2);
    EXPECT_EQ(vec1.data(), data);
}

TEST(ArrayVectorTest, TestMoveSelfAssign) // NOLINT
{
    array::Vector<int> vec{1, 2};
    array::Vector<int>* pvec = &vec;
    const int* data = vec.data();

    EXPECT_EQ(vec.data(), data);
    // We have to use pointer here, in order to fool a strict compiler
    // (which warns on simple case such as `vec = std::move(vec)`).
    vec = std::move(*pvec);
    EXPECT_EQ(vec.data(), data);
}

TEST(ArrayVectorTest, TestInitListAssignEmpty) // NOLINT
{
    array::Vector<int> vec(0);
    const array::Vector<int> expected{};

    vec = {};
    EXPECT_EQ(vec, expected);
}

TEST(ArrayVectorTest, TestInitListAssign) // NOLINT
{
    array::Vector<int> vec{1, 2};
    const array::Vector<int> expected{3, 4};

    vec = {3, 4};
    EXPECT_EQ(vec, expected);
}

TEST(ArrayVectorTest, TestInitListAssignSizeMismatch) // NOLINT
{
    array::Vector<int> vec1{1, 2};

    ASSERT_THROW(vec1 = {3}, quadiron::InvalidArgument);
}

TEST(ArrayVectorTest, TestIterator) // NOLINT
{
    const std::vector<int> expected_cst{1, 3, 5, 7, 9};
    const std::vector<int> expected_mut{0, 2, 4, 6, 8};
    const std::vector<int> expected_rcst{9, 7, 5, 3, 1};
    const std::vector<int> expected_rmut{8, 6, 4, 2, 0};
    const array::Vector<int> cst{1, 3, 5, 7, 9};
    array::Vector<int> mut{0, 2, 4, 6, 8};
    std::vector<int> result(std::max(cst.size(), mut.size()), 0);

    // Test begin/end
    std::copy(mut.begin(), mut.end(), result.begin());
    ASSERT_EQ(result, expected_mut);
    // Test begin/end (const version)
    std::copy(cst.begin(), cst.end(), result.begin());
    ASSERT_EQ(result, expected_cst);
    // Test rbegin/rend
    std::copy(mut.rbegin(), mut.rend(), result.begin());
    ASSERT_EQ(result, expected_rmut);
    // Test rbegin/rend (const version)
    std::copy(cst.rbegin(), cst.rend(), result.begin());
    ASSERT_EQ(result, expected_rcst);
    // Test cbegin/cend
    std::copy(mut.cbegin(), mut.cend(), result.begin());
    ASSERT_EQ(result, expected_mut);
    // Test crbegin/crend
    std::copy(mut.crbegin(), mut.crend(), result.begin());
    ASSERT_EQ(result, expected_rmut);
}

TEST(ArrayVectorTest, TestElementAccess) // NOLINT
{
    const array::Vector<int> cst{1, 3, 5, 7, 9};
    array::Vector<int> mut{0, 2, 4, 6, 8};

    // Test `front`.
    EXPECT_EQ(cst.front(), 1);
    EXPECT_EQ(mut.front(), 0);
    mut.front() = 42;
    EXPECT_EQ(mut.front(), 42);

    // Test `back`.
    EXPECT_EQ(cst.back(), 9);
    EXPECT_EQ(mut.back(), 8);
    mut.back() = 0;
    EXPECT_EQ(mut.back(), 0);

    // Test indexing operator.
    EXPECT_EQ(cst[2], 5);
    EXPECT_EQ(mut[2], 4);
    mut[2] = 666;
    EXPECT_EQ(mut[2], 666);

    // Test `at`.
    EXPECT_EQ(cst.at(3), 7);
    EXPECT_EQ(mut.at(3), 6);
    mut.at(3) = 88;
    EXPECT_EQ(mut.at(3), 88);

    ASSERT_THROW(cst.at(10), quadiron::OutOfRange);
    ASSERT_THROW(mut.at(10), quadiron::OutOfRange);
}

TEST(ArrayVectorTest, TestEqualityOperator) // NOLINT
{
    const array::Vector<int> empty1(0);
    const array::Vector<int> empty2(0);
    const array::Vector<int> vec1(5, 0);
    const array::Vector<int> vec1bis(vec1);
    const array::Vector<int> vec2{1, 3, 5, 7, 10};
    const array::Vector<int> vec3{1, 3, 5, 7};

    // Empty is equal to another empty.
    EXPECT_EQ(empty1, empty2);
    // Self equality.
    EXPECT_EQ(empty1, empty1);
    EXPECT_EQ(vec1, vec1);
    // Same size, same content.
    EXPECT_EQ(vec1, vec1bis);
    // Same size, different content.
    EXPECT_NE(vec1, vec2);
    // Different size.
    EXPECT_NE(empty1, vec1);
    EXPECT_NE(vec2, vec3);
}

TEST(ArrayVectorTest, TestOrderingOperator) // NOLINT
{
    const array::Vector<int> empty1(0);
    const array::Vector<int> empty2(0);
    const array::Vector<int> v1(5, 0);
    const array::Vector<int> v2(v1);
    const array::Vector<int> v3{1, 3, 5, 7};
    const array::Vector<int> v4{1, 3, 8, 7};
    const array::Vector<int> v5{1, 3, 5, 7, 10};

    // Same length, same content (empty or not).
    EXPECT_FALSE(empty1 < empty2);
    EXPECT_FALSE(empty1 > empty2);
    EXPECT_LE(empty1, empty2);
    EXPECT_GE(empty1, empty2);

    EXPECT_FALSE(v1 < v2);
    EXPECT_FALSE(v1 > v2);
    EXPECT_GE(v1, v2);
    EXPECT_LE(v1, v2);

    // Same length, different content.
    EXPECT_LT(v3, v4);
    EXPECT_LE(v3, v4);
    EXPECT_GT(v4, v3);
    EXPECT_GE(v4, v3);
    EXPECT_FALSE(v3 > v4);
    EXPECT_FALSE(v3 >= v4);
    EXPECT_FALSE(v4 < v3);
    EXPECT_FALSE(v4 <= v3);

    // Different size
    EXPECT_LT(v3, v5);
    EXPECT_LE(v3, v5);
    EXPECT_GT(v5, v3);
    EXPECT_GE(v5, v3);
    EXPECT_FALSE(v3 > v5);
    EXPECT_FALSE(v3 >= v5);
    EXPECT_FALSE(v5 < v3);
    EXPECT_FALSE(v5 <= v3);
}

TEST(ArrayVectorTest, TestSwap) // NOLINT
{
    using std::swap;

    const array::Vector<int> vec1{0, 2, 4, 6, 8};
    const array::Vector<int> vec2{1, 3, 5, 7};
    array::Vector<int> vec1bis(vec1);
    array::Vector<int> vec2bis(vec2);

    EXPECT_EQ(vec1bis, vec1);
    EXPECT_NE(vec1bis, vec2);

    EXPECT_EQ(vec2bis, vec2);
    EXPECT_NE(vec2bis, vec1);

    swap(vec1bis, vec2bis);

    EXPECT_NE(vec1bis, vec1);
    EXPECT_EQ(vec1bis, vec2);

    EXPECT_NE(vec2bis, vec2);
    EXPECT_EQ(vec2bis, vec1);
}
