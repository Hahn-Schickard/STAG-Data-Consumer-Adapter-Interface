#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_

#include "../DataConsumerAdapterInterface.hpp"

#include <gmock/gmock.h>

namespace DCAI {
namespace testing {
struct DataConsumerAdapterMock : public DataConsumerAdapterInterface {
  DataConsumerAdapterMock(
      ModelEventSourcePtr event_source, const std::string& name)
      : DataConsumerAdapterInterface(event_source, name) {
    ON_CALL(*this, start).WillByDefault([this]() {
      DataConsumerAdapterInterface::start();
    });
    ON_CALL(*this, stop).WillByDefault([this]() {
      DataConsumerAdapterInterface::stop();
    });
  }

  MOCK_METHOD(
      void, registrate, (Information_Model::NonemptyDevicePtr), (override));
  MOCK_METHOD(void, deregistrate, (const std::string&), (override));
  MOCK_METHOD(void, start, (), (override));
  MOCK_METHOD(void, stop, (), (override));

  HaSLI::LoggerPtr getLogger() { return this->logger_; }
};
} // namespace testing
} // namespace DCAI

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_