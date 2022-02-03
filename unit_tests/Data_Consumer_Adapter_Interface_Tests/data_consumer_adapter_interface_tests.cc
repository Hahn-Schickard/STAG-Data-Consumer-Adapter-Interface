#include "DataConsumerAdapterInterface_MOCK.hpp"
#include "Event_Model/EventSource.hpp"
#include "Information_Model/mocks/Device_MOCK.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace std;
using namespace DCAI;
using namespace DCAI::testing;
using namespace Information_Model::testing;

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
  void handleException(exception_ptr eptr) {
    if (eptr) {
      std::rethrow_exception(eptr);
    }
  }

public:
  EventSourceFake()
      : EventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)) {}

  void sendEvent(std::shared_ptr<ModelRegistryEvent> event) { notify(event); }
};

TEST(DCAI_Test, canHandleEvent) {
  auto event_source = make_shared<EventSourceFake>();
  auto adapter =
      make_shared<DataConsumerAdapterInterfaceMock>(event_source, "adopt me!");

  shared_ptr<ModelRegistryEvent> event = std::make_shared<ModelRegistryEvent>(
      Information_Model::NonemptyDevicePtr(
        std::make_shared<MockDevice>("12345", "Mock", "Mock device")));
  EXPECT_CALL(*adapter, handleEvent(event)).Times(1);

  event_source->sendEvent(event);
}

TEST(DCAI_Test, canStart) {
  DataConsumerAdapterInterfaceMock adapter(make_shared<EventSourceFake>(),
                                           "start me!");
  EXPECT_CALL(adapter, start());

  EXPECT_NO_THROW(adapter.start());
}

TEST(DCAI_Test, canGetLogger) {
  DataConsumerAdapterInterfaceMock adapter(make_shared<EventSourceFake>(),
                                           "start me!");
  EXPECT_NE(adapter.getLogger(), nullptr);
}

TEST(DCAI_Test, canStop) {
  DataConsumerAdapterInterfaceMock adapter(make_shared<EventSourceFake>(),
                                           "start me!");
  EXPECT_CALL(adapter, stop());

  EXPECT_NO_THROW(adapter.stop());
}