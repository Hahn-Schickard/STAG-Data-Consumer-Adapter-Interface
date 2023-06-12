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
namespace DCAI {

using ModelRegistryEvent =
    std::variant<std::string, Information_Model::NonemptyDevicePtr>;
using ModelRegistryEventPtr = std::shared_ptr<ModelRegistryEvent>;
using ModelEventSourcePtr =
    std::shared_ptr<Event_Model::EventSourceInterface<ModelRegistryEvent>>;

/**
 * @brief DataConsumerAdapterInterface is an abstraction for various user
 * applications that need to interact with the Information Model
 *
 * This interface provides the latest available Information Model Repository
 * snapshot, access to Information Model Registry events as well as a common
 * logging mechanism
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
   * @brief Non-blocking start method, starts Data Consumer Adapter
   * implementation operations
   *
   * @attention
   * Implementations MUST use Decorator pattern for this method, if custom
   * start/stop functionality is required
   *
   * @note
   * Implementations MUST guarantee that the calling thread WILL NOT be
   * blocked from executing other operations after the call to this method.
   */
  virtual void start() { logger_->info("{} started", name); }

  /**
   * @brief Blocking stop method, stops Technology Adapter implementation
   * communication operations. Blocks until communication operations are
   * finished
   *
   * @attention
   * Implementations MUST use Decorator pattern for this method, if custom
   * start/stop functionality is required
   *
   * @note
   * In the override of this method, implementations SHOULD join any threads
   * started in @see start() or wait for the result of any async tasks started
   * there
   */
  virtual void stop() { logger_->info("{} stopped", name); }

  const std::string name; // NOLINT(readability-identifier-naming)

protected:
  HaSLI::LoggerPtr logger_;
};

using DataConsumerAdapterInterfacePtr =
    std::shared_ptr<DataConsumerAdapterInterface>;
using DCAI_Ptr = DataConsumerAdapterInterfacePtr;
} // namespace DCAI

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_