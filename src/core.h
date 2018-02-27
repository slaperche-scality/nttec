/*
 * Copyright 2017-2018 the NTTEC authors
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
#ifndef __NTTEC_CORE_H__
#define __NTTEC_CORE_H__

#include <cstdint>

#include <gmpxx.h>

#include "big_int.h"

namespace nttec {

template <typename Type>
struct DoubleSize {
};
template <>
struct DoubleSize<uint32_t> {
    typedef uint64_t T;
};
template <>
struct DoubleSize<uint64_t> {
    typedef __uint128_t T;
};
template <>
struct DoubleSize<__uint128_t> {
    typedef UInt256 T;
};
template <>
struct DoubleSize<mpz_class> {
    typedef mpz_class T;
};

template <typename Type>
struct SignedDoubleSize {
};
template <>
struct SignedDoubleSize<uint32_t> {
    typedef int64_t T;
};
template <>
struct SignedDoubleSize<uint64_t> {
    typedef __int128_t T;
};
template <>
struct SignedDoubleSize<__uint128_t> {
    typedef Int256 T;
};
template <>
struct SignedDoubleSize<mpz_class> {
    typedef mpz_class T;
};

/** A group of values stored as one.
 *
 * This allows faster processing, as the values can be processed as one.
 */
template <typename T>
struct GroupedValues {
    // A group of several values.
    T values;

    /** Per-value flags.
     *
     * For now, only the first n bits (n being the number of values stored) are
     * used.
     * When the bit is set, the corresponding value should be 0 and that means
     * that the real value is Fn-1.
     */
    uint32_t flag;
};

/** Return the version string of NTTEC.
 *
 * The version string has the form MAJOR.MINOR.PATCH-REVISION, where '-REVISION'
 * is optional (only present for development version).
 *
 * @return the version string.
 */
const char* get_version();

} // namespace nttec

#endif
