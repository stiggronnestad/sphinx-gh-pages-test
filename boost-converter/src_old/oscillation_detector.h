#ifndef EVERT_OSCILLATION_DETECTOR_H_
#define EVERT_OSCILLATION_DETECTOR_H

#include <vector>

namespace evert
{
    ///
    /// @brief Helper class for detecting oscillations in delta values
    ///
    class OscillationDetector
    {
    public:
        /// @brief Construct a new OscillationDetector object
        /// @param cacheLength
        /// @param normalizedThreshold
        explicit OscillationDetector(int cacheLength, float normalizedThreshold);

        /// @brief Inject a new data point into the cache
        /// @param data
        void injectData(const float data);

        /// @brief Check if the cache "average" is oscillating
        /// @return true
        bool isOscillating() const;

    private:
        std::vector<float> m_cache;
        int m_currentIndex;
        float m_normalizedThreshold;
    };
}
#endif // EVERT_OSCILLATION_DETECTOR_H