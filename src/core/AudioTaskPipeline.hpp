#pragma once
#include "AudioTaskElement.hpp"
#include <thread>
#include <atomic>
#include <algorithm>
namespace audiotask::core {
/// @brief Manages multiple elements' connections and lifecycle
/// @details Handles element start/stop order, does not participate in data flow
class AudioTaskPipeline {
private:
    std::vector<std::shared_ptr<AudioTaskElement>> _elements;  ///< All elements in the pipeline
public:
    /// @brief Add an element to the pipeline
    /// @param element Element to add
    void add(std::shared_ptr<AudioTaskElement> element) {
        _elements.push_back(std::move(element));
    }
    /// @brief Link two elements' pads
    /// @param src Source element
    /// @param dst Destination element
    /// @return Whether the link succeeded
    bool link(AudioTaskElement* src, AudioTaskElement* dst) {
        return src->link_to(dst);
    } 
    /// @brief Start all elements in the pipeline
    /// @details Starts from back to front, ensuring sinks are ready to receive data
    /// @return Whether the start succeeded
    bool start() {
        for (auto it = _elements.rbegin(); it != _elements.rend(); ++it) {
            if (!(*it)->start()) return false;
        }
        return true;
    }
    /// @brief Stop all elements in the pipeline
    /// @details Stops from front to back, sources stop pushing data first
    /// @return Whether the stop succeeded
    bool stop() {
        for (auto& elem : _elements) {
            auto b_result = elem->stop();
        }
        return true;
    }
};
} // namespace audiotask::core
