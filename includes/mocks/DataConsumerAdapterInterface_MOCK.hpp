#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_

#include "DataConsumerAdapterInterface.hpp"

#include <gmock/gmock.h>

namespace DCAI {
namespace testing {
class DataConsumerAdapterInterfaceMock : public DataConsumerAdapterInterface {
public:
  DataConsumerAdapterInterfaceMock(const std::string &name)
      : DataConsumerAdapterInterface(name) {}
  MOCK_METHOD1(handleEvent,
               void(std::shared_ptr<Model_Event_Handler::NotifierEvent> event));
  MOCK_METHOD0(start, void());
  MOCK_METHOD0(stop, void());
};
} // namespace testing
} // namespace DCAI

#endif //__DATA_CONSUMER_ADAPTER_INTERFACE_MOCK_HPP_