#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_

#include "Event_Model/EventListenerInterface.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Information_Model/Device.hpp"

#include <memory>
#include <variant>
/**
 * @brief Data Consumer Adapter Interface
 * This namespace contains all of the information, required by the Information
 * Model Manager.
 * @author Dovydas Girdvainis
 * @date 04.12.2019
 */
namespace Data_Consumer_Adapter {

using ModelRegistryEvent =
    std::variant<std::string, Information_Model::NonemptyDevicePtr>;
using ModelRegistryEventPtr = std::shared_ptr<ModelRegistryEvent>;
using ModelEventSourcePtr =
    std::shared_ptr<Event_Model::EventSourceInterface<ModelRegistryEvent>>;

/**
 * @brief Generic Interface for all Data Consumer Adapter Implementations
 *
 */
struct DataConsumerAdapterInterface
    : public Event_Model::EventListenerInterface<ModelRegistryEvent> {

  DataConsumerAdapterInterface(
      ModelEventSourcePtr event_source, const std::string& adapter_name)
      : EventListenerInterface(event_source), name(adapter_name),
        logger_(HaSLI::LoggerManager::registerLogger(name)) {
    logger_->log(HaSLI::SeverityLevel::TRACE,
        "DataConsumerAdapterInterface::DataConsumerAdapterInterface({})", name);
  }

  virtual ~DataConsumerAdapterInterface() = default;

  /**
   * @brief Non-blocking start method, throws std::runtime_error if building and
   * registration interface was not set before this method was called
   *
   * Implementations must use Decorator pattern for this method.
   * Implementations should start a thread in the override of this method.
   *
   */
  virtual void start() { logger_->log(HaSLI::SeverityLevel::INFO, "Started!"); }

  /**
   * @brief Blocking stop method. Blocks until the thread is finished working.
   *
   * Implementations must use Decorator pattern for this method.
   * Implementations should call a join on the thread in the override of this
   * method.
   *
   */
  virtual void stop() {
    logger_->log(HaSLI::SeverityLevel::INFO, "Received a stop command!");
  }

  const std::string name; // NOLINT(readability-identifier-naming)

protected:
  HaSLI::LoggerPtr logger_;
};

using DCAI = DataConsumerAdapterInterface;
using DataConsumerAdapterInterfacePtr = std::shared_ptr<DCAI>;
using DCAI_Ptr = DataConsumerAdapterInterfacePtr;
} // namespace Data_Consumer_Adapter

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_