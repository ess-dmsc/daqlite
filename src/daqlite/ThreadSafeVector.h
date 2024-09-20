// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CustomAMOR2DTOFPlot.h
///
/// \brief Creates (maybe) a QCustomPlot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <cstdint>

template <typename T>
class ThreadSafeVector {
public:
    void push_back(const T& value) {
        std::lock_guard<std::mutex> lock(mMutex);
        mVector.push_back(value);
    }

    std::vector<T> get() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mVector;
    }

    T at(const size_t index) const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mVector.at(index);
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mVector.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mMutex);
        mVector.clear();
    }

    void resize(const size_t newSize) {
        std::lock_guard<std::mutex> lock(mMutex);
        mVector.resize(newSize);
    }

    void fill(const T& value) {
        std::lock_guard<std::mutex> lock(mMutex);
        std::fill(mVector.begin(), mVector.end(), value);
    }

    void add_values(const std::vector<T>& other) {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mVector.size() < other.size()) {
            mVector.resize(other.size());
        }
        for (size_t i = 0; i < other.size(); ++i) {
            mVector[i] += other[i];
        }
    }

    ThreadSafeVector<T>& operator=(const std::vector<T>& other) {
        std::lock_guard<std::mutex> lock(mMutex);
        mVector = other;
        return *this;
    }

    operator std::vector<T>() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mVector;
    }

private:
    mutable std::mutex mMutex;
    std::vector<T> mVector;
};