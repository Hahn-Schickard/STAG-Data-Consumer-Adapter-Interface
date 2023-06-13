#include "DataConsumerAdapterInterface_MOCK.hpp"
#include "Event_Model/EventSource.hpp"
#include "Information_Model/mocks/Device_MOCK.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace std;
using namespace Data_Consumer_Adapter;
using namespace Data_Consumer_Adapter::testing;
using namespace Information_Model::testing;

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
  /* This is the expected handle exception footprint, don't change it without
   * changing the definition */
  void handleException(exception_ptr eptr) { // NOLINT
    if (eptr) {
      std::rethrow_exception(eptr);
    }
  }

public:
  EventSourceFake()
      : EventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)) {}

  /* NEVER move event values! clang linter is lying here! */
  void sendEvent(ModelRegistryEventPtr event) {
    notify(event); // NOLINT
  }
};

TEST(DCAI_Test, canHandleEvent) { // NOLINT
  auto event_source = make_shared<EventSourceFake>();
  auto adapter =
      make_shared<::testing::NiceMock<DataConsumerAdapterInterfaceMock>>(
          event_source, "adopt me!");

  ModelRegistryEventPtr event = std::make_shared<ModelRegistryEvent>(
      Information_Model::NonemptyDevicePtr(std::make_shared<MockDevice>(
          "12345", "Mock", "Mock device"))); // NOLINT
  EXPECT_CALL(*adapter, handleEvent(event)).Times(1); // NOLINT

  event_source->sendEvent(event);
}

TEST(DCAI_Test, canStart) { // NOLINT
  DataConsumerAdapterInterfaceMock adapter(
      make_shared<EventSourceFake>(), "start me!");
  EXPECT_CALL(adapter, start()); // NOLINT

  EXPECT_NO_THROW(adapter.start());
}

TEST(DCAI_Test, canGetLogger) { // NOLINT
  DataConsumerAdapterInterfaceMock adapter(
      make_shared<EventSourceFake>(), "start me!");
  EXPECT_NE(adapter.getLogger(), nullptr);
}

TEST(DCAI_Test, canStop) { // NOLINT
  DataConsumerAdapterInterfaceMock adapter(
      make_shared<EventSourceFake>(), "start me!");
  EXPECT_CALL(adapter, stop());

  EXPECT_NO_THROW(adapter.stop());
}
