/* -*- mode: c++ -*- */
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
#ifndef __QUAD_GF_RING_H__
#define __QUAD_GF_RING_H__

#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include <unistd.h>

#include "arith.h"
#include "core.h"
#include "exceptions.h"
#include "vec_buffers.h"

namespace quadiron {

namespace vec {

template <typename T>
class Vector;

} // namespace vec

namespace gf {

template <typename Class, typename... Args>
Class create(Args... args)
{
    Class obj(args...);
    obj.init();
    return obj;
}

template <typename Base, typename Class, typename... Args>
std::unique_ptr<Base> alloc(Args... args)
{
    std::unique_ptr<Base> obj = std::unique_ptr<Class>(new Class(args...));
    obj->init();
    return obj;
}

/** A ring of integers modulo N.
 */
template <typename T>
class RingModN {
  public:
    virtual ~RingModN() = default;
    bool isNF4 = false;
    void compute_factors_of_order();
    void find_primitive_root();
    virtual T card(void) const;
    virtual T card_minus_one(void) const;
    virtual bool check(T a) const;
    virtual T neg(T a) const;
    virtual T add(T a, T b) const;
    virtual T sub(T a, T b) const;
    virtual T mul(T a, T b) const;
    virtual T div(T a, T b) const;
    T inv_bezout(T a) const;
    virtual T inv(T a) const;
    virtual T exp(T a, T b) const;
    virtual T log(T a, T b) const;
    T exp_naive(T base, T exponent) const;
    T exp_quick(T base, T exponent) const;
    T log_naive(T base, T exponent) const;
    virtual T replicate(T a) const;
    virtual void mul_coef_to_buf(T a, T* src, T* dest, size_t len) const;
    virtual void mul_vec_to_vecp(
        vec::Vector<T>& u,
        vec::Buffers<T>& src,
        vec::Buffers<T>& dest) const;
    virtual void add_two_bufs(T* src, T* dest, size_t len) const;
    virtual void
    add_vecp_to_vecp(vec::Buffers<T>& src, vec::Buffers<T>& dest) const;
    virtual void sub_two_bufs(T* bufa, T* bufb, T* res, size_t len) const;
    virtual void sub_vecp_to_vecp(
        vec::Buffers<T>& veca,
        vec::Buffers<T>& vecb,
        vec::Buffers<T>& res) const;
    virtual void butterfly_ct(T coef, T* buf1, T* buf2, size_t len) const;
    virtual void butterfly_gs(T coef, T* buf1, T* buf2, size_t len) const;
    bool is_quadratic_residue(T q) const;
    virtual void compute_omegas(vec::Vector<T>& W, int n, T w) const;
    void compute_omegas_cached(vec::Vector<T>& W, int n, T w) const;
    virtual T weak_rand(void) const;
    bool is_primitive_root(T nb) const;
    T get_root() const;
    T get_primitive_root() const;
    bool check_primitive_root(T nb) const;
    bool check_order_naive(T nb, T order) const;
    T do_step_get_order(
        T x,
        T h,
        std::vector<T>& primes,
        std::vector<int>& exponents) const;
    T get_order(T x) const;
    virtual T get_nth_root(T n) const;
    T get_code_len(T n) const;
    T get_code_len_high_compo(T n) const;
    virtual void hadamard_mul(int n, T* x, T* y) const;
    virtual void hadamard_mul_doubled(int n, T* x, T* y) const;
    virtual void add_doubled(int n, T* x, T* y) const;
    virtual void neg(size_t n, T* x) const;
    virtual void neg(vec::Buffers<T>& buf) const;

    RingModN(RingModN&&) = default;

  protected:
    explicit RingModN(T card);
    virtual void init();

    template <typename Class, typename... Args>
    friend Class create(Args... args);

    template <typename Base, typename Class, typename... Args>
    friend std::unique_ptr<Base> alloc(Args... args);

    T _card;
    T root;
    std::vector<T> primes;
    std::vector<int> exponents;
    std::vector<T> all_primes_factors;
    std::vector<T> proper_divisors;
};
template <typename T>
RingModN<T>::RingModN(T card)
{
    this->_card = card;
    this->root = 0;
}

template <typename T>
void RingModN<T>::init()
{
    // compute factors of order
    this->compute_factors_of_order();

    // compute root of unity
    this->find_primitive_root();
}

template <typename T>
inline T RingModN<T>::card(void) const
{
    return this->_card;
}

template <typename T>
inline T RingModN<T>::card_minus_one(void) const
{
    return this->_card - 1;
}

template <typename T>
bool RingModN<T>::check(T a) const
{
    return (a >= 0 && a < this->_card);
}

template <typename T>
inline T RingModN<T>::neg(T a) const
{
    assert(check(a));

    return sub(0, a);
}

template <typename T>
inline T RingModN<T>::add(T a, T b) const
{
    assert(check(a));
    assert(check(b));

    T c = a + b;
    if (c >= this->_card)
        c -= this->_card;
    return c;
}

template <typename T>
inline T RingModN<T>::sub(T a, T b) const
{
    assert(check(a));
    assert(check(b));

    if (a >= b)
        return a - b;
    else
        return this->_card - (b - a);
}

template <typename T>
inline T RingModN<T>::mul(T a, T b) const
{
    assert(check(a));
    assert(check(b));

    return T((DoubleSizeVal<T>(a) * b) % this->_card);
}

template <typename T>
inline T RingModN<T>::div(T a, T b) const
{
    assert(check(a));
    assert(check(b));

    T inv_b = inv(b);

    return mul(a, inv_b);
}

/// Inverse by Bezout.
template <typename T>
inline T RingModN<T>::inv_bezout(T a) const
{
    assert(check(a));

    SignedDoubleSizeVal<T> x = a;
    SignedDoubleSizeVal<T> n = this->_card;
    SignedDoubleSizeVal<T> bezout[2];

    arith::extended_gcd<T>(x, n, bezout, nullptr);
    if (bezout[0] < 0)
        bezout[0] = this->_card + bezout[0];
    return bezout[0];
}

template <typename T>
inline T RingModN<T>::inv(T a) const
{
    return inv_bezout(a);
}

template <typename T>
inline T RingModN<T>::exp(T a, T b) const
{
    assert(check(a));
    assert(check(b));

    return exp_quick(a, b);
}

template <typename T>
inline T RingModN<T>::log(T a, T b) const
{
    assert(check(a));

    return log_naive(a, b);
}

/// Naive exponentiation in the group.
template <typename T>
inline T RingModN<T>::exp_naive(T base, T exponent) const
{
    T result;
    T i;

    if (0 == exponent)
        return 1;

    if (1 == exponent)
        return base;

    result = base;
    for (i = 1; i < exponent; i++)
        result = this->mul(result, base);

    return result;
}

/// Quick exponentiation in the group.
template <typename T>
inline T RingModN<T>::exp_quick(T base, T exponent) const
{
    T result;

    if (0 == exponent)
        return 1;

    if (1 == exponent)
        return base;

    T tmp = this->exp_quick(base, exponent / 2);
    // std::cout << base << "^" << exponent / 2 << "=" << tmp << "\n";
    result = this->mul(tmp, tmp);
    // std::cout << "result = " << tmp << "^2 = " << result << "\n";
    if (exponent % 2 == 1)
        result = this->mul(result, base);
    // std::cout << "final result = " << result << "\n";
    return result;
}

/** Naive brute force algorithm in the group.
 *
 * @throw quadiron::NoSolution if no solution exists.
 */
template <typename T>
inline T RingModN<T>::log_naive(T base, T exponent) const
{
    T result;

    for (result = 1; result < card(); result++) {
        if (exp(base, result) == exponent)
            return result;
    }

    throw NoSolution("solution not found");
}

// This operation is only reserved for NF4.
template <typename T>
inline T RingModN<T>::replicate(T a) const
{
    return a;
}

// For each i, dest[i] = a * src[i]
template <typename T>
inline void RingModN<T>::mul_coef_to_buf(T a, T* src, T* dest, size_t len) const
{
    size_t i;
    DoubleSizeVal<T> coef = DoubleSizeVal<T>(a);
    for (i = 0; i < len; i++) {
        // perform multiplication
        dest[i] = mul(coef, src[i]);
    }
}

template <typename T>
inline void RingModN<T>::mul_vec_to_vecp(
    vec::Vector<T>& u,
    vec::Buffers<T>& src,
    vec::Buffers<T>& dest) const
{
    assert(u.get_n() == src.get_n());
    int i;
    int n = u.get_n();
    size_t len = src.get_size();
    T h = this->card_minus_one();
    const std::vector<T*>& src_mem = src.get_mem();
    const std::vector<T*>& dest_mem = dest.get_mem();
    T* coef_vec = u.get_mem();
    for (i = 0; i < n; i++) {
        T coef = coef_vec[i];
        if (coef > 1 && coef < h) {
            this->mul_coef_to_buf(coef, src_mem[i], dest_mem[i], len);
        } else if (coef == 1) {
            dest.copy(i, src_mem[i]);
        } else if (coef == 0) {
            dest.fill(i, 0);
        } else if (coef == h) {
            dest.copy(i, src_mem[i]);
            this->neg(len, dest_mem[i]);
        }
    }
}

template <typename T>
inline void RingModN<T>::add_two_bufs(T* src, T* dest, size_t len) const
{
    size_t i;
    for (i = 0; i < len; i++) {
        // perform addition
        dest[i] = add(src[i], dest[i]);
    }
}

template <typename T>
inline void
RingModN<T>::add_vecp_to_vecp(vec::Buffers<T>& src, vec::Buffers<T>& dest) const
{
    assert(src.get_n() == dest.get_n());
    assert(src.get_size() == dest.get_size());
    int i;
    int n = src.get_n();
    size_t len = src.get_size();
    for (i = 0; i < n; i++) {
        this->add_two_bufs(src.get(i), dest.get(i), len);
    }
}

template <typename T>
inline void
RingModN<T>::sub_two_bufs(T* bufa, T* bufb, T* res, size_t len) const
{
    size_t i;
    for (i = 0; i < len; i++) {
        res[i] = sub(bufa[i], bufb[i]);
    }
}

template <typename T>
inline void RingModN<T>::sub_vecp_to_vecp(
    vec::Buffers<T>& veca,
    vec::Buffers<T>& vecb,
    vec::Buffers<T>& res) const
{
    assert(veca.get_n() == vecb.get_n());
    assert(veca.get_size() == vecb.get_size());
    int i;
    int n = veca.get_n();
    size_t len = veca.get_size();
    for (i = 0; i < n; i++) {
        this->sub_two_bufs(veca.get(i), vecb.get(i), res.get(i), len);
    }
}

/** Butterfly computation for Cooley-Tukey FFT algorithm
 *
 * Perform in-place oprations on two buffers `P`, `Q` with a coefficient `c`
 *
 * \f{eqnarray*}{
 *  P_i &= P_i + c \times Q_i \\
 *  Q_i &= P_i - c \times Q_i \\
 * \f}
 *
 * @param coef - coefficient, a finite field element
 * @param buf1 - a buffer of `len` elements
 * @param buf2 - a buffer of `len` elements
 * @param len - number of elements per buffer
 */
template <typename T>
inline void
RingModN<T>::butterfly_ct(T coef, T* buf1, T* buf2, size_t len) const
{
    size_t i;
    for (i = 0; i < len; ++i) {
        T a = buf1[i];
        T b = mul(coef, buf2[i]);
        buf1[i] = add(a, b);
        buf2[i] = sub(a, b);
    }
}

/** Butterfly computation for Gentleman-Sande FFT algorithm
 *
 * Perform in-place oprations on two buffers `P`, `Q` with a coefficient `c`
 *
 * \f{eqnarray*}{
 *  P_i &= P_i + Q_i \\
 *  Q_i &= c \times (P_i - Q_i)
 * \f}
 *
 * @param coef - coefficient, a finite field element
 * @param buf1 - a buffer of `len` elements
 * @param buf2 - a buffer of `len` elements
 * @param len - number of elements per buffer
 */
template <typename T>
inline void
RingModN<T>::butterfly_gs(T coef, T* buf1, T* buf2, size_t len) const
{
    size_t i;
    for (i = 0; i < len; ++i) {
        T a = buf1[i];
        T b = buf2[i];
        T c = sub(a, b);
        buf1[i] = add(a, b);
        buf2[i] = mul(coef, c);
    }
}

template <typename T>
void RingModN<T>::compute_factors_of_order()
{
    T h = card_minus_one();
    // prime factorisation of order, i.e. order = p_i^e_i where
    //  p_i, e_i are ith element of this->primes and this->exponents.
    arith::factor_prime<T>(h, &primes, &exponents);
    // Store all primes in a vector.
    // A prime is replicated according to its exponent.
    all_primes_factors = arith::get_prime_factors<T>(primes, exponents);
    // calculate all proper divisor of order. A proper divisor = order/p_i for
    //  each prime divisor of order.
    proper_divisors = arith::get_proper_divisors<T>(h, primes);
}

/** Check if `q` is a quadractic residue.
 *
 * `q` is a quadratic residue, if x<sup>2</sup> == q % n
 *
 * @param q a number
 * @return boolean
 */
template <typename T>
bool RingModN<T>::is_quadratic_residue(T q) const
{
    T i;

    for (i = 0; i < this->card(); i++) {
        if (this->exp(i, 2) == q)
            return true;
    }

    return false;
}

/** Compute the different powers of the root of unity into a vector.
 *
 * @param W output vector (must be of length n)
 * @param n length of the output vector
 * @param w n-th root of unity
 */
template <typename T>
inline void RingModN<T>::compute_omegas(vec::Vector<T>& W, int n, T w) const
{
    for (int i = 0; i < n; i++) {
        W.set(i, this->exp(w, i));
    }
}

/** Compute the different powers of the root of unity into a vector.
 *
 * @note cache the result in a file called W<w>.cache
 * @note XXX not reentrant
 *
 * @param W output vector (must be of length n)
 * @param n length of the output vector
 * @param w n-th root of unity
 */
template <typename T>
void RingModN<T>::compute_omegas_cached(vec::Vector<T>& W, int n, T w) const
{
    std::ostringstream filename;

    filename << "W" << w << ".cache";

    if (-1 == access(filename.str().c_str(), F_OK)) {
        std::ofstream file;
        file.open(filename.str().c_str(), std::ios::out);
        for (int i = 0; i < n; i++) {
            W.set(i, this->exp(w, i));
            file << W.get(i) << "\n";
        }
    } else {
        std::ifstream file;
        int i = 0;
        file.open(filename.str().c_str(), std::ios::in);
        T tmp;
        while (file >> tmp) {
            W.set(i, tmp);
            i++;
        }
        assert(i == n);
    }
}

template <typename T>
T RingModN<T>::weak_rand(void) const
{
    std::uniform_int_distribution<uint32_t> dis(1, this->card() - 1);
    return dis(prng());
}

/** Check if a number is primitive root or not.
 *
 * A number `x` is a primitive root if its order \f$y = q - 1\f$, i.e.
 * \f$x^{i}\neq 1\qquad (i=1, 2, 3, \dots , q-2)\f$
 *
 * Note, order of a number must be a divisor of q-1.
 *
 * Algorithm to checking whether `x` is primitive root or not.
 *
 * Methodology: checking its order is not a proper divisor of (q-1).
 *
 * 1. Prime factorization \f$q - 1 = p_1^{r_1} p_2^{r_2} \dots p_m^{r_m}\f$
 * 2. Get "necessary" proper divisors of `q - 1`:
 *    \f$ D =
 *    \left\{
 *      \frac{q-1}{p_1}, \frac{q-1}{p_2}, \dots, \frac{q-1}{p_m}
 *    \right\}
 *    \f$
 * 3. `x` is a primitive root if for every divisor `d` of `D`: \f$x^d \neq 1\f$
 *
 * Proof:
 *
 *  Suppose that `x` satisfies the algorithm but its order is a proper divisor
 *  `y` of `q-1`.
 *
 *  This order can be expressed as \f$\frac{q-1}{y}\f$ where:
 *
 *  \f[
 *    y = p_1^{s_1} p_2^{s_2} \dots p_m^{s_m} \text{ with } s_i \leq r_i
 *  \f]
 *
 *  and
 *
 *  \f[
 *    x^{\frac{q-1}{y}} = 1
 *  \f]
 *
 *  Withouth loss of generality, we suppose \f$s_1 \geq 1\f$. We have
 *
 *  \f[
 *    x^{\frac{q-1}{y}} = 1
 *    \Rightarrow (x^{\frac{q-1}{y}})^{\frac{y}{p_1}} = 1
 *    \Leftrightarrow x^{\frac{q-1}{p_1}} = 1
 *  \f]
 *
 *  Contradict to Step 3 of the algorithm.
 */
template <typename T>
bool RingModN<T>::is_primitive_root(T nb) const
{
    bool ok = true;
    typename std::vector<T>::size_type i;
    T h = this->card_minus_one();
    // Check nb^divisor == 1.
    for (i = 0; i != this->proper_divisors.size(); ++i) {
        if (this->exp(nb, this->proper_divisors[i]) == 1) {
            ok = false;
            break;
        }
    }
    return ok;
}

/** Find primitive root of finite field.
 *
 * A number `x` is a primitive root if its order \f$y = q - 1\f$, i.e.
 * \f$x^{i}\neq 1\qquad (i=1, 2, 3, \dots , q-2)\f$
 *
 * Use the algorithm of checking if a number is primitive root or not.
 */
template <typename T>
void RingModN<T>::find_primitive_root()
{
    if (this->root) {
        return;
    }

    T h = card_minus_one();
    if (h == 1) {
        this->root = 1;
        return;
    }

    T nb = 2;
    bool ok;
    typename std::vector<T>::size_type i;

    while (nb <= h) {
        ok = true;
        // Check nb^divisor == 1.
        for (i = 0; i != this->proper_divisors.size(); ++i) {
            if (RingModN<T>::exp(nb, this->proper_divisors[i]) == 1) {
                ok = false;
                break;
            }
        }
        if (ok) {
            this->root = nb;
            break;
        }
        nb++;
    }

    if (!this->root) {
        assert(false); // Root not found.
    }
}

template <typename T>
T RingModN<T>::get_root() const
{
    return root;
}

template <typename T>
T RingModN<T>::do_step_get_order(
    T x,
    T h,
    std::vector<T>& primes,
    std::vector<int>& exponents) const
{
    T y, p;
    int r;
    while (!primes.empty()) {
        p = primes.back();
        r = exponents.back();
        primes.pop_back();
        exponents.pop_back();
        y = h / p;
        if (this->exp(x, y) != 1) {
            // remove (p, r)
            while (r > 1) {
                y /= p;
                r--;
            }
            continue;
        }
        // exp(x, y) == 1
        if (r > 1) {
            primes.push_back(p);
            exponents.push_back(r - 1);
        }
        // do next
        return do_step_get_order(x, y, primes, exponents);
    }
    return h;
}

/*
 * Get order of an element x of RingModN(q)
 *  The order d is the smallest divisor of (q-1) such that x^d = 1
 * Pseudocode:
 *  Prime factorisation of (q-1) = p1^r1 * p2^r2 * ... * pm^rm
 *  D = { (p1, r1), (p2, r2), .. , (pm, rm) }
 *  h = q - 1
 *   function do_step_get_order(x, h, D) {
 *     for (pi, ri) in D {
 *       y = h/pi
 *       newD = D \ (pi, ri)
 *       if (x^y != 1) {
 *           y /= pi^(ri - 1)
 *           continue;
 *       }
 *       // x^y == 1
 *       if (ri > 1)
 *         newD = newD + (pi, ri - 1)
 *       return do_step_get_order(x, y, newD)
 *     }
 *     return h;
 *   }
 */
template <typename T>
T RingModN<T>::get_order(T x) const
{
    if (x == 0 || x == 1)
        return 1;
    T h = this->card_minus_one();
    std::vector<T> _primes(primes);
    std::vector<int> _exponent(exponents);
    T order = do_step_get_order(x, h, _primes, _exponent);

    if (order == 1)
        return h;
    return order;
}

/** Check whether a number is a primitive root.
 *
 * @param nb the number to check
 * @return true if `nb` is a primitive root, false otherwise.
 */
template <typename T>
bool RingModN<T>::check_primitive_root(T nb) const
{
    T h = this->card_minus_one();
    return (get_order(nb) == h);
}

/*
 * Check naively order of a number
 */
template <typename T>
bool RingModN<T>::check_order_naive(T nb, T order) const
{
    if (this->exp(nb, order) != 1)
        return false;

    T i = 1;
    T tmp = nb;
    while (i < order - 1) {
        // std::cout << i << ":" << tmp << std::endl;
        if (tmp == 1)
            return false;
        tmp = this->mul(tmp, nb);
        i++;
    }
    return true;
}

/// Compute root of order n: g^((q-1)/d) where d = gcd(n, q-1).
template <typename T>
T RingModN<T>::get_nth_root(T n) const
{
    T q_minus_one = this->card_minus_one();
    T d = arith::gcd<T>(n, q_minus_one);
    T nth_root = this->exp(this->root, q_minus_one / d);
    return nth_root;
}

/** Return primitive root.
 *
 * @return the primitive root.
 */
template <typename T>
T RingModN<T>::get_primitive_root() const
{
    return this->root;
}

/** Find smallest number that is greater than `n` and divisible by `q-1`.
 *
 * @param[in] n the lower bound
 * @return the code length
 */
template <typename T>
T RingModN<T>::get_code_len(T n) const
{
    T nb = this->card_minus_one();
    if (nb < n) {
        assert(false);
    }

    return arith::get_code_len<T>(nb, n);
}

/** Find smallest number that is highly composite, greater than `n` and
 * divisible by `q-1`.
 *
 * @param[in] n the lower bound
 * @return the code length
 */
template <typename T>
T RingModN<T>::get_code_len_high_compo(T n) const
{
    T nb = this->card_minus_one();
    if (nb < n)
        assert(false);

    return arith::get_code_len_high_compo<T>(all_primes_factors, n);
}

template <typename T>
inline void RingModN<T>::hadamard_mul(int n, T* x, T* y) const
{
    for (int i = 0; i < n; i++) {
        x[i] = mul(x[i], y[i]);
    }
}

template <typename T>
inline void RingModN<T>::hadamard_mul_doubled(int n, T* x, T* y) const
{
    const int half = n / 2;
    T* x_next = x + half;

    // multiply y to the first half of `x`
    for (int i = 0; i < half; i++) {
        x[i] = mul(x[i], y[i]);
    }

    // multiply y to the second half of `x`
    for (int i = 0; i < half; i++) {
        x_next[i] = mul(x_next[i], y[i]);
    }
}

template <typename T>
inline void RingModN<T>::add_doubled(int n, T* x, T* y) const
{
    const int half = n / 2;
    T* x_next = x + half;

    // add y to the first half of `x`
    for (int i = 0; i < half; i++) {
        x[i] = add(x[i], y[i]);
    }

    // add y to the second half of `x`
    for (int i = 0; i < half; i++) {
        x_next[i] = add(x_next[i], y[i]);
    }
}

template <typename T>
inline void RingModN<T>::neg(size_t n, T* x) const
{
    // add y to the first half of `x`
    for (size_t i = 0; i < n; i++) {
        x[i] = sub(0, x[i]);
    }
}

template <typename T>
inline void RingModN<T>::neg(vec::Buffers<T>& buf) const
{
    size_t size = buf.get_size();
    for (int i = 0; i < buf.get_n(); i++) {
        neg(size, buf.get(i));
    }
}

#ifdef QUADIRON_USE_SIMD
/* Operations are vectorized by SIMD */

template <>
void RingModN<uint16_t>::neg(size_t n, uint16_t* x) const;

template <>
void RingModN<uint32_t>::neg(size_t n, uint32_t* x) const;

template <>
void RingModN<uint16_t>::mul_coef_to_buf(
    uint16_t a,
    uint16_t* src,
    uint16_t* dest,
    size_t len) const;

template <>
void RingModN<uint32_t>::mul_coef_to_buf(
    uint32_t a,
    uint32_t* src,
    uint32_t* dest,
    size_t len) const;

template <>
void RingModN<uint16_t>::add_two_bufs(uint16_t* src, uint16_t* dest, size_t len)
    const;

template <>
void RingModN<uint32_t>::add_two_bufs(uint32_t* src, uint32_t* dest, size_t len)
    const;

template <>
void RingModN<uint16_t>::sub_two_bufs(
    uint16_t* bufa,
    uint16_t* bufb,
    uint16_t* res,
    size_t len) const;

template <>
void RingModN<uint32_t>::sub_two_bufs(
    uint32_t* bufa,
    uint32_t* bufb,
    uint32_t* res,
    size_t len) const;

template <>
void RingModN<uint16_t>::butterfly_ct(
    uint16_t coef,
    uint16_t* buf1,
    uint16_t* buf2,
    size_t len) const;

template <>
void RingModN<uint32_t>::butterfly_ct(
    uint32_t coef,
    uint32_t* buf1,
    uint32_t* buf2,
    size_t len) const;

template <>
void RingModN<uint16_t>::butterfly_gs(
    uint16_t coef,
    uint16_t* buf1,
    uint16_t* buf2,
    size_t len) const;

template <>
void RingModN<uint32_t>::butterfly_gs(
    uint32_t coef,
    uint32_t* buf1,
    uint32_t* buf2,
    size_t len) const;

template <>
void RingModN<uint16_t>::hadamard_mul(int n, uint16_t* x, uint16_t* y) const;
template <>
void RingModN<uint32_t>::hadamard_mul(int n, uint32_t* x, uint32_t* y) const;
// template <>
// void RingModN<uint64_t>::hadamard_mul(int n, uint64_t* x, uint64_t* y) const;
// template <>
// void RingModN<__uint128_t>::hadamard_mul(int n, __uint128_t* x, __uint128_t*
// y) const;

template <>
void RingModN<uint16_t>::hadamard_mul_doubled(int n, uint16_t* x, uint16_t* y)
    const;
template <>
void RingModN<uint32_t>::hadamard_mul_doubled(int n, uint32_t* x, uint32_t* y)
    const;
// template <>
// void RingModN<uint64_t>::hadamard_mul_doubled(int n, uint64_t* x,
// uint64_t* y) const;
// template <> void RingModN<__uint128_t>::hadamard_mul_doubled(int n,
// __uint128_t* x, __uint128_t* y) const;

template <>
void RingModN<uint16_t>::add_doubled(int n, uint16_t* x, uint16_t* y) const;
template <>
void RingModN<uint32_t>::add_doubled(int n, uint32_t* x, uint32_t* y) const;
// template <>
// void RingModN<uint64_t>::add_doubled(int n, uint64_t* x, uint64_t* y) const;
// template <>
// void RingModN<__uint128_t>::add_doubled(int n, __uint128_t* x, __uint128_t*
// y) const;

#endif // #ifdef QUADIRON_USE_SIMD

} // namespace gf
} // namespace quadiron

#endif
