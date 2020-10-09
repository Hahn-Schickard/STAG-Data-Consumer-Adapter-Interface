#include "DataConsumerAdapterInterface_MOCK.hpp"
#include "Device_MOCK.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace std;
using namespace DCAI;
using namespace DCAI::testing;
using namespace Information_Model::testing;

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
public:
  void sendEvent(std::shared_ptr<ModelRegistryEvent> event) { notify(event); }
};

TEST(DCAI_Test, canHandleEvent) {
  auto event_source = make_shared<EventSourceFake>();
  auto adapter =
      make_shared<DataConsumerAdapterInterfaceMock>("adopt me!", event_source);

  shared_ptr<ModelRegistryEvent> event = make_shared<ModelRegistryEvent>(
      make_shared<MockDevice>("12345", "Mock", "Mock device"));
  EXPECT_CALL(*adapter, handleEvent(event)).Times(1);

  event_source->sendEvent(event);
}

TEST(DCAI_Test, canStart) {
  DataConsumerAdapterInterfaceMock adapter("start me!",
                                           make_shared<EventSourceFake>());
  EXPECT_CALL(adapter, start());

  EXPECT_NO_THROW(adapter.start());
}

TEST(DCAI_Test, canGetLogger) {
  DataConsumerAdapterInterfaceMock adapter("start me!",
                                           make_shared<EventSourceFake>());
  EXPECT_NE(adapter.getLogger(), nullptr);
}

TEST(DCAI_Test, canStop) {
  DataConsumerAdapterInterfaceMock adapter("start me!",
                                           make_shared<EventSourceFake>());
  EXPECT_CALL(adapter, stop());

  EXPECT_NO_THROW(adapter.stop());
}