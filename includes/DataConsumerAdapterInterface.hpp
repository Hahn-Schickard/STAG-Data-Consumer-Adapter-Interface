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

using ModelRepositoryEvent =
    std::variant<std::string, Information_Model::NonemptyDevicePtr>;
using ModelRepositoryEventPtr = std::shared_ptr<ModelRepositoryEvent>;
using ModelEventSourcePtr =
    std::shared_ptr<Event_Model::EventSourceInterface<ModelRepositoryEvent>>;

/**
 * @brief DataConsumerAdapterInterface is an abstraction for various user
 * applications that need to interact with the Information Model
 *
 * This interface provides the latest available Information Model Repository
 * snapshot, access to Information Model Registry events as well as a common
 * logging mechanism
 */
struct DataConsumerAdapterInterface
    : public Event_Model::EventListenerInterface<ModelRepositoryEvent> {

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
  void initialiseModel(std::vector<Information_Model::DevicePtr> devices) {
    auto registrate_lock = std::lock_guard(event_mx_);
    for (auto device : devices) {
      registerDevice(Information_Model::NonemptyDevicePtr(device));
    }
  }

  /**
   * @brief Non-blocking start method, throws std::runtime_error if building and
   * registration interface was not set before this method was called
   *
   * @attention
   * Implementations MUST call this method in their overrides, if custom
   * start/stop functionality is required
   *
   * @note
   * Implementations MUST guarantee that the calling thread WILL NOT be
   * blocked from executing other operations after the call to this method.
   */
  virtual void start(std::vector<Information_Model::DevicePtr> devices = {}) {
    logger_->log(HaSLI::SeverityLevel::INFO, "Started!");
    initialiseModel(devices);
  }

  /**
   * @brief Blocking stop method, stops Technology Adapter implementation
   * communication operations. Blocks until communication operations are
   * finished
   *
   * @attention
   * Implementations MUST call this method in their overrides, if custom
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
  void handleEvent(ModelRepositoryEventPtr event) override {
    match(*event,
        [this](Information_Model::NonemptyDevicePtr device) {
          auto registrate_lock = std::lock_guard(event_mx_);
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