#pragma once
#include <algorithm>
#include <cmath>
#include <random>

class UniformDist {
private:
  std::mt19937_64 rng;
  std::uniform_int_distribution<uint64_t> dist;

  void reset(int seed) { rng.seed(seed); }

public:
  uint64_t next() { return dist(rng); }

  UniformDist(uint64_t max_value, int seed = 0) {
    dist = std::uniform_int_distribution<uint64_t>(0, max_value - 1);
    reset(seed);
  }
};

class ZipfianDist {
protected:
  static constexpr double epsilon = 1e-8;

  std::mt19937 rng;
  uint64_t n;                                  ///< Number of elements
  double q;                                    ///< Exponent
  double H_x1;                                 ///< H(x_1)
  double H_n;                                  ///< H(n)
  std::uniform_real_distribution<double> dist; ///< [H(x_1), H(n)]

  void reset(int seed) { rng.seed(seed); }

public:
  ZipfianDist(const uint64_t n = std::numeric_limits<uint64_t>::max(),
              int seed = 0)
      : n(n), q(0.99), H_x1(H(1.5) - 1.0), H_n(H(n + 0.5)), dist(H_x1, H_n) {
    reset(seed);
  }

  uint64_t next() {
    while (true) {
      const double u = dist(rng);
      const double x = H_inv(u);
      const uint64_t k = clamp<uint64_t>(std::round(x), 1, n);
      if (u >= H(k + 0.5) - h(k)) {
        return k;
      }
    }
  }

protected:
  // Clamp x to [min, max].
  template <typename T>
  static T clamp(const T x, const T min, const T max) {
    return std::max(min, std::min(max, x));
  }

  // exp(x) - 1 / x
  static double expxm1bx(const double x) {
    return (std::abs(x) > epsilon)
               ? std::expm1(x) / x
               : (1.0 + x / 2.0 * (1.0 + x / 3.0 * (1.0 + x / 4.0)));
  }

  // H(x) = log(x) if q == 1, (x^(1-q) - 1)/(1 - q) otherwise.
  // H(x) is an integral of h(x).
  // Note the numerator is one less than in the paper order to work with all
  // positive q.
  double H(const double x) const {
    const double log_x = std::log(x);
    return expxm1bx((1.0 - q) * log_x) * log_x;
  }

  // log(1 + x) / x
  static double log1pxbx(const double x) {
    return (std::abs(x) > epsilon)
               ? std::log1p(x) / x
               : 1.0 - x * ((1 / 2.0) - x * ((1 / 3.0) - x * (1 / 4.0)));
  }

  // The inverse function of H(x)
  double H_inv(const double x) const {
    const double t = std::max(-1.0, x * (1.0 - q));
    return std::exp(log1pxbx(t) * x);
  }

  // That hat function h(x) = 1 / (x ^ q)
  double h(const double x) const { return std::exp(-q * std::log(x)); }
};

class ScrambledZipfianDist : public ZipfianDist {
  static constexpr const uint64_t kFNVOffsetBasis64 = 0xCBF29CE484222325;
  static constexpr const uint64_t kFNVPrime64 = 1099511628211;

  inline uint64_t FNVHash64(uint64_t val) {
    uint64_t hash = kFNVOffsetBasis64;

    for (int i = 0; i < 8; i++) {
      uint64_t octet = val & 0x00ff;
      val = val >> 8;

      hash = hash ^ octet;
      hash = hash * kFNVPrime64;
    }
    return hash;
  }

public:
  ScrambledZipfianDist(const uint64_t n = std::numeric_limits<uint64_t>::max(),
                       int seed = 0)
      : ZipfianDist(n, seed) {}

  uint64_t next() { return scramble(ZipfianDist::next()) % n; }

  uint64_t scramble(uint64_t rand_num) { return FNVHash64(rand_num); }
};