// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ThreadSafeVector.h
///
/// \brief This file contains the definition of the ThreadSafeVector template
/// class.
///
/// The ThreadSafeVector class provides a thread-safe wrapper around a
/// std::vector. It ensures that all operations on the vector are protected by a
/// mutex, making it safe to use in a multi-threaded environment.

#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

/// \class ThreadSafeVector
/// \brief A thread-safe wrapper around std::vector.
///
/// \tparam DataType The type of elements stored in the vector.
/// \tparam OtherDataType The type of elements in other vectors that can be
/// added or assigned to this vector.
template <typename DataType, typename OtherDataType> class ThreadSafeVector {

public:
  /// \brief Adds a value to the end of the vector.
  /// \param value The value to be added.
  void push_back(const DataType &value) {
    std::lock_guard<std::mutex> lock(mMutex);
    mVector.push_back(value);
  }

  /// \brief Retrieves a copy of the vector.
  /// \return A copy of the vector.
  std::vector<DataType> get() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mVector;
  }

  /// \brief Retrieves the element at the specified index.
  /// \param index The index of the element to retrieve.
  /// \return The element at the specified index.
  DataType at(const size_t index) const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mVector.at(index);
  }

  /// \brief Retrieves the number of elements in the vector.
  /// \return The number of elements in the vector.
  size_t size() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mVector.size();
  }

  /// \brief Clears all elements from the vector.
  void clear() {
    std::lock_guard<std::mutex> lock(mMutex);
    mVector.clear();
  }

  /// \brief Resizes the vector to the specified size.
  /// \param newSize The new size of the vector.
  void resize(const size_t newSize) {
    std::lock_guard<std::mutex> lock(mMutex);
    mVector.resize(newSize);
  }

  /// \brief Fills the vector with the specified value.
  /// \param value The value to fill the vector with.
  void fill(const DataType &value) {
    std::lock_guard<std::mutex> lock(mMutex);
    std::fill(mVector.begin(), mVector.end(), value);
  }

  /// \brief Adds values from another vector to this vector.
  /// \param other The vector containing values to be added.
  void add_values(const std::vector<DataType> &other) {
    std::lock_guard<std::mutex> lock(mMutex);
    if (mVector.size() < other.size()) {
      mVector.resize(other.size());
    }
    for (size_t i = 0; i < other.size(); ++i) {
      mVector[i] += other[i];
    }
  }

  /// \brief Adds values from another vector of a different type to this vector.
  /// \param other The vector containing values to be added.
  void add_values(const std::vector<OtherDataType> &other) {
    std::lock_guard<std::mutex> lock(mMutex);
    if (mVector.size() < other.size()) {
      mVector.resize(other.size());
    }
    for (size_t i = 0; i < other.size(); ++i) {
      mVector[i] += static_cast<DataType>(other[i]);
    }
  }

  /// \brief Assigns values from another vector to this vector.
  /// \param other The vector containing values to be assigned.
  /// \return A reference to this vector.
  ThreadSafeVector<DataType, OtherDataType> &
  operator=(const std::vector<DataType> &other) {
    std::lock_guard<std::mutex> lock(mMutex);
    mVector = other;
    return *this;
  }

  /// \brief Assigns values from another vector of a different type to this
  /// vector. \param other The vector containing values to be assigned. \return
  /// A reference to this vector.
  ThreadSafeVector<DataType, OtherDataType> &
  operator=(const std::vector<OtherDataType> &other) {
    std::lock_guard<std::mutex> lock(mMutex);
    mVector.resize(other.size());
    for (size_t i = 0; i < other.size(); ++i) {
      mVector[i] = static_cast<DataType>(other[i]);
    }
    return *this;
  }

  /// \brief Converts this vector to a std::vector.
  /// \return A copy of the vector as a std::vector.
  operator std::vector<DataType>() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mVector;
  }

private:
  mutable std::mutex mMutex;
  std::vector<DataType> mVector;
};