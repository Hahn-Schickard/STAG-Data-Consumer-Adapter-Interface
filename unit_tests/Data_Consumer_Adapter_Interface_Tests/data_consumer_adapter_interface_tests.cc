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

using EventSourceFakePtr = shared_ptr<EventSourceFake>;

struct DCAI_TestFixture : public ::testing::Test {
protected:
  void SetUp() override {
    event_source = make_shared<EventSourceFake>();
    adapter_mock = make_shared<::testing::NiceMock<DCAI_Mock>>(
        event_source, "Mock adapter");
    adapter = adapter_mock;
  }

  EventSourceFakePtr event_source; // NOLINT(readability-identifier-naming)
  DCAI_MockPtr adapter_mock; // NOLINT(readability-identifier-naming)
  DCAI_Ptr adapter; // NOLINT(readability-identifier-naming)
};

TEST_F(DCAI_TestFixture, canHandleEvent) { // NOLINT
  ModelRegistryEventPtr event = std::make_shared<ModelRegistryEvent>(
      Information_Model::NonemptyDevicePtr(std::make_shared<MockDevice>(
          "12345", "Mock", "Mock device"))); // NOLINT
  EXPECT_CALL(*adapter_mock, handleEvent(event)).Times(1); // NOLINT

  event_source->sendEvent(event);
}

TEST_F(DCAI_TestFixture, canStart) { // NOLINT
  EXPECT_CALL(*adapter_mock, start());
  EXPECT_NO_THROW(adapter->start());
}

TEST_F(DCAI_TestFixture, canStop) { // NOLINT
  EXPECT_CALL(*adapter_mock, stop());

  EXPECT_NO_THROW(adapter->stop());
}
