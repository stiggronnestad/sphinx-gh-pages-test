#ifndef EVERT_OSCILLATION_DETECTOR_H_
#define EVERT_OSCILLATION_DETECTOR_H

#include <vector>

#include "oscillation_detector.h"

namespace evert
{
    /// @brief Construct a new OscillationDetector object
    /// @param cacheLength
    /// @param normalizedThreshold
    OscillationDetector::OscillationDetector(int cacheLength, float normalizedThreshold)
        : m_cache(std::vector<float>(cacheLength, 0.0f)),
          m_currentIndex(0),
          m_normalizedThreshold(normalizedThreshold)
    {
    }

    /// @brief Inject a new data point into the cache
    /// @param data
    void OscillationDetector::injectData(const float data)
    {
        m_cache[m_currentIndex] = data;
        m_currentIndex = (m_currentIndex + 1) % m_cache.size();
    }

    /// @brief Check if the cache "average" is oscillating
    /// @return true
    bool OscillationDetector::isOscillating() const
    {
        // Check if the cache is close to normalizedThreshold positive and negative signed values
        int positiveCount = 0;
        int negativeCount = 0;

        for (const auto &data : m_cache)
        {
            positiveCount += data >= 0 ? 1 : 0;
        }

        negativeCount = m_cache.size() - positiveCount;

        // If the cache is close to nt/nt positive and negative signed values, then we are oscillating
        return positiveCount > m_normalizedThreshold * m_cache.size() &&
               negativeCount > m_normalizedThreshold * m_cache.size();
    }
} // namespace evert
#endif // EVERT_OSCILLATION_DETECTOR_H