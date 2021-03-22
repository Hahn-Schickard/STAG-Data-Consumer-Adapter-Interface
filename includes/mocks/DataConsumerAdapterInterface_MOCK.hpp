#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_

#include "../DataConsumerAdapterInterface.hpp"

#include <gmock/gmock.h>

namespace DCAI {
namespace testing {
class DataConsumerAdapterInterfaceMock : public DataConsumerAdapterInterface {
public:
  DataConsumerAdapterInterfaceMock(
      const std::string &name,
      Event_Model::EventSourceInterfacePtr<ModelRegistryEvent> event_source)
      : DataConsumerAdapterInterface(name, event_source) {
    ON_CALL(*this, start).WillByDefault([this]() {
      DataConsumerAdapterInterface::start();
    });
    ON_CALL(*this, stop).WillByDefault([this]() {
      DataConsumerAdapterInterface::stop();
    });
  }
  MOCK_METHOD1(handleEvent, void(std::shared_ptr<ModelRegistryEvent> event));
  MOCK_METHOD0(start, void());
  MOCK_METHOD0(stop, void());
};
} // namespace testing
} // namespace DCAI

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_