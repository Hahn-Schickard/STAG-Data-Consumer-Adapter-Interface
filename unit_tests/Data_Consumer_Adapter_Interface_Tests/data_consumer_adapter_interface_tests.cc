#include "DataConsumerAdapterInterface_MOCK.hpp"
#include "Event_Model/EventSource.hpp"
#include "Information_Model/mocks/Device_MOCK.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace std;
using namespace DCAI;
using namespace DCAI::testing;
using namespace Information_Model;
using namespace Information_Model::testing;

namespace Information_Model {
bool operator==(const NonemptyDevicePtr& lhs, const NonemptyDevicePtr& rhs) {
  return lhs.get() == rhs.get();
}
} // namespace Information_Model

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

NonemptyDevicePtr makeDevice(const string& id) {
  return NonemptyDevicePtr(
      std::make_shared<MockDevice>(id, "Mock", "Mock device"));
}

TEST_F(DCAI_TestFixture, canRegisterDevice) { // NOLINT
  auto device = makeDevice("12345");
  EXPECT_CALL(*adapter_mock, registrate(device)).Times(1);

  auto event = std::make_shared<ModelRegistryEvent>(device);
  event_source->sendEvent(event);
}

constexpr size_t INITIAL_MODEL_SIZE = 5;

TEST_F(DCAI_TestFixture, canInitialiseModel) { // NOLINT
  vector<NonemptyDevicePtr> devices;
  devices.reserve(INITIAL_MODEL_SIZE);
  for (size_t i = 0; i < INITIAL_MODEL_SIZE; ++i) {
    devices.emplace_back(makeDevice(to_string(i)));
  }

  EXPECT_FALSE(devices.empty());
  EXPECT_EQ(INITIAL_MODEL_SIZE, devices.size());

  for (const auto& device : devices) {
    EXPECT_CALL(*adapter_mock, registrate(device)).Times(1);

    auto event = std::make_shared<ModelRegistryEvent>(device);
    event_source->sendEvent(event);
  }
}

TEST_F(DCAI_TestFixture, canDeregisterDevice) { // NOLINTs
  auto device_id = "12345";
  EXPECT_CALL(*adapter_mock, deregistrate(device_id)).Times(1);

  auto event = std::make_shared<ModelRegistryEvent>(device_id);
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
