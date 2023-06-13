#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_

#include "Event_Model/EventListenerInterface.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Information_Model/Device.hpp"
#include "Variant_Visitor.hpp"

#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
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
   * @brief Forwards an existing device abstraction model to Data Consumer
   * Adapter implementations, so it can register already managed device
   * abstraction instances. Blocks all calls to registrate() method until all
   * device abstractions have finished the registration process
   *
   * @note
   * Used by Information Model Manager
   *
   * @param devices pre-existing list of device abstractions
   */
  void initialiseModel(
      std::vector<Information_Model::NonemptyDevicePtr> devices) {
    for (auto device : devices) {
      registerDevice(device);
    }
  }

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
  /**
   * @brief Adds a given device instance to Data Consumer Adapter
   * implementation. Blocks all other calls to registrate() method until device
   * abstraction registration is finished
   *
   * If the given device instance was already registered by a previous call,
   * this call should be treated as an update to the existing device instance
   *
   * @note
   * Any exceptions thrown from this call are logged by the internal logger as
   * an error
   *
   * @param device new/changed device instance
   */
  virtual void registrate(Information_Model::NonemptyDevicePtr /* device */) {
    std::string error_msg = "Called based implementation of " + name +
        " DataConsumerAdapterInterface::registrate()";
    throw std::runtime_error(error_msg);
  }

  /**
   * @brief Searches for a device with a given device id and removes it from the
   * Data Consumer Adapter implementation if it exists
   *
   * @note
   * Any exceptions thrown from this call are logged by the internal logger as
   * an error
   *
   * @param device_id
   */
  virtual void deregistrate(const std::string& /* device_id */) {
    std::string error_msg = "Called based implementation of " + name +
        " DataConsumerAdapterInterface::deregistrate()";
    throw std::runtime_error(error_msg);
  }

  HaSLI::LoggerPtr logger_;

private:
  void handleEvent(ModelRegistryEventPtr event) override {
    match(*event,
        [this](Information_Model::NonemptyDevicePtr device) {
          registerDevice(device);
        },
        [this](const std::string& device_id) {
          try {
            deregistrate(device_id);
          } catch (const std::exception& ex) {
            logger_->error(
                "{} Data Consumer Adapter encountered an unhandled exception "
                "while deregistrating device {}. Exception: {}",
                name, device_id, ex.what());
          }
        });
  }

  void registerDevice(Information_Model::NonemptyDevicePtr device) {
    try {
      auto registrate_lock = std::lock_guard(event_mx_);
      registrate(device);
    } catch (const std::exception& ex) {
      logger_->error(
          "{} Data Consumer Adapter encountered an unhandled exception "
          "while registrating device {}. Exception: {}",
          name, device->getElementId(), ex.what());
    }
  }

  std::mutex event_mx_;
};

using DCAI = DataConsumerAdapterInterface;
using DataConsumerAdapterInterfacePtr = std::shared_ptr<DCAI>;
using DCAI_Ptr = DataConsumerAdapterInterfacePtr;
} // namespace Data_Consumer_Adapter

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_