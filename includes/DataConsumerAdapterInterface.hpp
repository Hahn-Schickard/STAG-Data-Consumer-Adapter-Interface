#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_

#include "Listener.hpp"
#include "LoggerRepository.hpp"
#include "Stoppable.hpp"

/**
 * @brief Data Consumer Adapter Interface
 * This namespace containes all of the information, required by the Information
 * Model Manager.
 * @author Dovydas Girdvainis
 * @date 04.12.2019
 */
namespace DCAI {
/**
 * @brief Generic Interface for all Data Consumer Adapter Implementations
 *
 */
class DataConsumerAdapterInterface : public Model_Event_Handler::Listener,
                                     public Stoppable {

  const std::string adapter_name_;

protected:
  std::shared_ptr<HaSLL::Logger> adapter_logger_;

public:
  DataConsumerAdapterInterface(const std::string &name)
      : adapter_name_(name),
        adapter_logger_(HaSLL::LoggerRepository::getInstance().registerLoger(
            adapter_name_)) {
    adapter_logger_->log(
        HaSLL::SeverityLevel::TRACE,
        "DataConsumerAdapterInterface::DataConsumerAdapterInterface({})",
        adapter_name_);
  }

  ~DataConsumerAdapterInterface() = default;

  const std::string getAdapterName() const { return adapter_name_; }

  void start() {
    adapter_logger_->log(HaSLL::SeverityLevel::INFO, "Started!");
    Stoppable::start();
  }

  void stop() {
    adapter_logger_->log(HaSLL::SeverityLevel::INFO,
                         "Received a stop command!");
    Stoppable::stop();
  }
};
} // namespace DCAI

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_