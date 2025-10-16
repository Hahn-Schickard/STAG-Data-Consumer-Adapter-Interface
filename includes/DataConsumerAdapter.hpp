#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_

#include <HaSLL/Logger.hpp>
#include <Information_Model/Device.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <variant>

namespace Data_Consumer_Adapter {
struct DataConnection {
  virtual ~DataConnection() = default;
};

using DataConnectionPtr = std::shared_ptr<DataConnection>;
using ModelRepositoryEvent =
    std::variant<std::string, Information_Model::DevicePtr>;
using ModelRepositoryEventPtr = std::shared_ptr<ModelRepositoryEvent>;
using DataNotifier = std::function<void(const ModelRepositoryEventPtr&)>;
using DataConnector = std::function<DataConnectionPtr(DataNotifier&&)>;

/**
 * @brief DataConsumerAdapterInterface is an abstraction for various user
 * applications that need to interact with the Information Model
 *
 * This interface provides the latest available Information Model Repository
 * snapshot, access to Information Model Registry events as well as a common
 * logging mechanism
 */
struct DataConsumerAdapter {
  using Devices = std::vector<Information_Model::DevicePtr>;

  DataConsumerAdapter(const DataConsumerAdapter&) = delete;

  DataConsumerAdapter(const std::string& name, const DataConnector& connector);

  virtual ~DataConsumerAdapter() = default;

  DataConsumerAdapter& operator=(const DataConsumerAdapter&) = delete;

  /**
   * @brief Returns the assigned adapter name
   *
   * @return std::string
   */
  std::string name() const;

  /**
   * @brief Non-blocking start method
   *
   * @attention
   * Implementations MUST call this method in their overrides, if custom
   * start/stop functionality is required
   *
   * @note
   * Implementations MUST guarantee that the calling thread WILL NOT be
   * blocked from executing other operations after the call to this method.
   */
  virtual void start();

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
  virtual void stop();

  void initialiseModel(const Devices& devices);

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
  virtual void registrate(const Information_Model::DevicePtr& device) = 0;

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
  virtual void deregistrate(const std::string& device_id) = 0;

private:
  void handleEvent(const ModelRepositoryEventPtr& event);

  void registerDevice(const Information_Model::DevicePtr& device);

  std::string name_;
  std::mutex mx_;
  DataConnectionPtr connection_;
};

using DataConsumerAdapterPtr = std::shared_ptr<DataConsumerAdapter>;
} // namespace Data_Consumer_Adapter

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_