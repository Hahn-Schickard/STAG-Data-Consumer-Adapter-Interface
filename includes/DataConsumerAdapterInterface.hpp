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
#include <thread>
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
  using Devices = std::vector<Information_Model::DevicePtr>;

  DataConsumerAdapterInterface(
      const ModelEventSourcePtr& event_source, const std::string& name)
      : EventListenerInterface(event_source),
        logger(HaSLL::LoggerManager::registerLogger(name)), name_(name) {
    logger->trace(
        "DataConsumerAdapterInterface::DataConsumerAdapterInterface({})",
        name_);
  }

  virtual ~DataConsumerAdapterInterface() {
    if (init_thread_.joinable()) {
      init_thread_.join();
    }
  }

  /**
   * @brief Returns the assigned adapter name
   *
   * @return std::string
   */
  std::string name() const { return name_; }

  /**
   * @brief Non-blocking start method, forwards an existing device abstraction
   * model to Data Consumer Adapter implementations, so it can register already
   * managed device abstraction instances and starts Data Consumer Adapter
   * implementation operations.
   *
   * Blocks all other calls to registrate() method until all device abstractions
   * have finished the registration process
   *
   * @attention
   * Implementations MUST call this method in their overrides, if custom
   * start/stop functionality is required
   *
   * @note
   * Implementations MUST guarantee that the calling thread WILL NOT be
   * blocked from executing other operations after the call to this method.
   */
  virtual void start(const Devices& devices = {}) {
    logger->info("Started!");
    init_thread_ = std::thread([this, devices]() { initialiseModel(devices); });
  }

  /**
   * @brief Blocking stop method, stops Data Consumer Adapter implementation
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
  virtual void stop() { logger->info("{} stopped", name_); }

protected:
  HaSLL::LoggerPtr logger; // NOLINT(readability-identifier-naming)

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
  virtual void registrate(
      const Information_Model::NonemptyDevicePtr& /* device */) {
    std::string error_msg = "Called based implementation of " + name_ +
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
    std::string error_msg = "Called based implementation of " + name_ +
        " DataConsumerAdapterInterface::deregistrate()";
    throw std::runtime_error(error_msg);
  }

private:
  void initialiseModel(const Devices& devices) {
    auto registrate_lock = std::scoped_lock(event_mx_);
    for (auto device : devices) {
      registerDevice(Information_Model::NonemptyDevicePtr(device));
    }
  }

  void handleEvent(ModelRepositoryEventPtr event) override {
    match(
        *event,
        [this](Information_Model::NonemptyDevicePtr device) {
          auto registrate_lock = std::scoped_lock(event_mx_);
          registerDevice(device);
        },
        [this](const std::string& device_id) {
          try {
            deregistrate(device_id);
          } catch (const std::exception& ex) {
            logger->error(
                "{} Data Consumer Adapter encountered an unhandled exception "
                "while deregistrating device {}. Exception: {}",
                name_, device_id, ex.what());
          }
        });
  }

  void registerDevice(const Information_Model::NonemptyDevicePtr& device) {
    try {
      registrate(device);
    } catch (const std::exception& ex) {
      logger->error(
          "{} Data Consumer Adapter encountered an unhandled exception "
          "while registrating device {}. Exception: {}",
          name_, device->getElementId(), ex.what());
    }
  }

  std::string name_;
  std::mutex event_mx_;
  std::thread init_thread_;
};

using DCAI = DataConsumerAdapterInterface;
using DataConsumerAdapterInterfacePtr = std::shared_ptr<DCAI>;
using DCAI_Ptr = DataConsumerAdapterInterfacePtr;
} // namespace Data_Consumer_Adapter

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_