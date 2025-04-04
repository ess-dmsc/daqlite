// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief A cyclic iterator over the integer positions in an N-dimensional space
//===----------------------------------------------------------------------===//
#pragma once

#include <array>

namespace fylgje {
  /// \brief A cyclic iterator over the integer positions in an N-dimensional space
  /// \tparam N the number of dimensions
  template <std::size_t N>
  class Cycles {
  public:
    using array_t = std::array<int, N>;
    /// \brief Construct a Cycles object with the given lengths, which are the maximum values for each dimension
    explicit Cycles(array_t const& lengths) : lengths(lengths) {}
    /// \brief Construct a Cycles object with the given lengths, which are the maximum values for each dimension
    explicit Cycles(array_t&& lengths) : lengths(std::move(lengths)) {}
    /// \brief Construct a Cycles object with the given lengths and current integer position
    Cycles(array_t const & lengths, array_t const & current) : lengths(lengths), current(current) {}

    /// \brief Update the current position to the next position in the cycle
    bool next() {
      for (int i = N; i-- > 0;) {
        if (++current[i] < lengths[i]) {
          return true;
        }
        current[i] = 0;
      }
      return false;
    }

    /// \brief Get the current position
    array_t const& operator*() const {
      return current;
    }

    /// \brief Get the current position along the i-th dimension
    int operator[](int i) const {
      return current[i];
    }

    /// \brief Set the current position
    void set(array_t const & c) {
      this->current = c;
    }

    /// \brief Get the current position along the i-th dimension, with bounds checking and exception throwing
    int at(int i) {
      if (i < 0 || i >= static_cast<int>(N)) {
        throw std::out_of_range("The index is out of range");
      }
      return current[i];
    }

  private:
    /// \brief The size of each dimension
    array_t lengths;
    /// \brief The current position
    array_t current{};
  };

}