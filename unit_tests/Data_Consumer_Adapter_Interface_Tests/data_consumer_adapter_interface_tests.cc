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

NonemptyDevicePtr makeDevice(const string& id) {
  return NonemptyDevicePtr(
      std::make_shared<MockDevice>(id, "Mock", "Mock device"));
}

TEST(DCAI_Test, canRegisterDevice) { // NOLINT
  auto event_source = make_shared<EventSourceFake>();
  auto adapter =
      make_shared<::testing::NiceMock<DataConsumerAdapterInterfaceMock>>(
          event_source, "adopt me!");

  auto device = makeDevice("12345");
  EXPECT_CALL(*adapter, registrate(device)).Times(1);

  auto event = std::make_shared<ModelRegistryEvent>(device);
  event_source->sendEvent(event);
}

constexpr size_t INITIAL_MODEL_SIZE = 5;

TEST(DCAI_Test, canInitialiseModel) { // NOLINT
  auto event_source = make_shared<EventSourceFake>();
  auto adapter =
      make_shared<::testing::NiceMock<DataConsumerAdapterInterfaceMock>>(
          event_source, "adopt me!");

  vector<NonemptyDevicePtr> devices;
  devices.reserve(INITIAL_MODEL_SIZE);
  for (size_t i = 0; i < INITIAL_MODEL_SIZE; ++i) {
    devices.emplace_back(makeDevice(to_string(i)));
  }

  EXPECT_FALSE(devices.empty());
  EXPECT_EQ(INITIAL_MODEL_SIZE, devices.size());

  for (const auto& device : devices) {
    EXPECT_CALL(*adapter, registrate(device)).Times(1);

    auto event = std::make_shared<ModelRegistryEvent>(device);
    event_source->sendEvent(event);
  }
}

TEST(DCAI_Test, canDeregisterDevice) { // NOLINT
  auto event_source = make_shared<EventSourceFake>();
  auto adapter =
      make_shared<::testing::NiceMock<DataConsumerAdapterInterfaceMock>>(
          event_source, "adopt me!");

  auto device_id = "12345";
  EXPECT_CALL(*adapter, deregistrate(device_id)).Times(1);

  auto event = std::make_shared<ModelRegistryEvent>(device_id);
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
