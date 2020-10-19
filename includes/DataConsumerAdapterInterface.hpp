#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_

#include "Device.hpp"
#include "Event_Model.hpp"
#include "LoggerRepository.hpp"

#include <memory>
#include <variant>
/**
 * @brief Data Consumer Adapter Interface
 * This namespace containes all of the information, required by the Information
 * Model Manager.
 * @author Dovydas Girdvainis
 * @date 04.12.2019
 */
namespace DCAI {

using ModelRegistryEvent =
    std::variant<std::string, std::shared_ptr<Information_Model::Device>>;
using ModelEventSourcePtr =
      std::shared_ptr<Event_Model::EventSourceInterface<ModelRegistryEvent>>;

/**
 * @brief Generic Interface for all Data Consumer Adapter Implementations
 *
 */
class DataConsumerAdapterInterface
    : public Event_Model::EventListener<ModelRegistryEvent> {
  using LoggerPtr = std::shared_ptr<HaSLL::Logger>;

  const std::string adapter_name_;
  LoggerPtr adapter_logger_;

public:
  DataConsumerAdapterInterface(
      const std::string &name,
      ModelEventSourcePtr event_source)
      : EventListener(event_source), adapter_name_(name),
        adapter_logger_(HaSLL::LoggerRepository::getInstance().registerLoger(
            adapter_name_)) {
    adapter_logger_->log(
        HaSLL::SeverityLevel::TRACE,
        "DataConsumerAdapterInterface::DataConsumerAdapterInterface({})",
        adapter_name_);
  }

  ~DataConsumerAdapterInterface() = default;

  const std::string getAdapterName() const { return adapter_name_; }

  LoggerPtr getLogger() { return adapter_logger_; }

  /**
   * @brief Non-blocking start method, throws std::runtime_error if building and
   * registration interface was not set before this method was called
   *
   * Implementations must use Decorator pattern for this method.
   * Implementations should start a thread in the override of this method.
   *
   */
  virtual void start() {
    adapter_logger_->log(HaSLL::SeverityLevel::INFO, "Started!");
  }

  /**
   * @brief Blocking stop method. Blocks until the thread is finished working.
   *
   * Implementations must use Decorator pattern for this method.
   * Implementations should call a join on the thread in the override of this
   * method.
   *
   */
  virtual void stop() {
    adapter_logger_->log(HaSLL::SeverityLevel::INFO,
                         "Received a stop command!");
  }
};
} // namespace DCAI

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_