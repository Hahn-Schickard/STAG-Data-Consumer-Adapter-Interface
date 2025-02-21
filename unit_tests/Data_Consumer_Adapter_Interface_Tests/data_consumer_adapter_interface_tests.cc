#include "DataConsumerAdapterInterface_MOCK.hpp"
#include "Event_Model/EventSource.hpp"
#include "Information_Model/mocks/DeviceMockBuilder.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace std;
using namespace Data_Consumer_Adapter;
using namespace Data_Consumer_Adapter::testing;
using namespace Information_Model;
using namespace Information_Model::testing;

namespace Information_Model {
bool operator==(const NonemptyDevicePtr& lhs, const NonemptyDevicePtr& rhs) {
  return lhs.get() == rhs.get();
}
} // namespace Information_Model

class EventSourceFake : public Event_Model::EventSource<ModelRepositoryEvent> {
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
  void sendEvent(ModelRepositoryEventPtr event) {
    notify(event); // NOLINT
  }
};

using EventSourceFakePtr = shared_ptr<EventSourceFake>;

// NOLINTNEXTLINE(readability-identifier-naming)
struct Naked_DCAI : DataConsumerAdapterInterface {
  Naked_DCAI(
      const ModelEventSourcePtr& event_source, const std::string& adapter_name)
      : DataConsumerAdapterInterface(move(event_source), adapter_name) {}

  void registrate(const NonemptyDevicePtr& device) override {
    DataConsumerAdapterInterface::registrate(device);
  }

  void deregistrate(const std::string& device_id) override {
    DataConsumerAdapterInterface::deregistrate(device_id);
  }
};

// NOLINTNEXTLINE
TEST(DataConsumerAdapterInterfaceTests,
    callingBaseImplementationThrowsRuntimeError) {
  auto dcai = Naked_DCAI(make_shared<EventSourceFake>(), "dcai");

  EXPECT_THROW(dcai.registrate(NonemptyDevicePtr(
                   make_shared<MockDevice>("1234", "test", "test device"))),
      runtime_error);
  EXPECT_THROW(dcai.deregistrate("1234"), runtime_error);
}

// NOLINTNEXTLINE(readability-identifier-naming)
struct DCAI_TestFixture : public ::testing::Test {
protected:
  void SetUp() override {
    event_source = make_shared<EventSourceFake>();
    adapter_mock = make_shared<::testing::NiceMock<DCAI_Mock>>(
        event_source, "Mock adapter");
    adapter = adapter_mock;
  }

  void TearDown() override {
    adapter.reset();
    adapter_mock.reset();
    event_source.reset();
  }

  EventSourceFakePtr event_source; // NOLINT(readability-identifier-naming)
  DCAI_MockPtr adapter_mock; // NOLINT(readability-identifier-naming)
  DCAI_Ptr adapter; // NOLINT(readability-identifier-naming)
};

NonemptyDevicePtr makeDevice(const string& id) {
  auto builder = DeviceMockBuilder();
  builder.buildDeviceBase(id, "Mock", "Mock device");
  builder.addReadableMetric(
      "readable", "readable metric mock", DataType::BOOLEAN);
  return NonemptyDevicePtr(move(builder.getResult()));
}

TEST_F(DCAI_TestFixture, canRegisterDevice) { // NOLINT
  auto device = makeDevice("12345");
  EXPECT_CALL(*adapter_mock, registrate(device)).Times(1);

  auto event = std::make_shared<ModelRepositoryEvent>(device);
  event_source->sendEvent(event);
}

constexpr size_t INITIAL_MODEL_SIZE = 5;

TEST_F(DCAI_TestFixture, canInitialiseModel) { // NOLINT
  vector<DevicePtr> devices;
  devices.reserve(INITIAL_MODEL_SIZE);
  for (size_t i = 0; i < INITIAL_MODEL_SIZE; ++i) {
    devices.emplace_back(makeDevice(to_string(i)).base());
  }

  EXPECT_FALSE(devices.empty());
  EXPECT_EQ(INITIAL_MODEL_SIZE, devices.size());

  for (const auto& device : devices) {
    auto nonempty_device = NonemptyDevicePtr(device);
    EXPECT_CALL(*adapter_mock, registrate(nonempty_device)).Times(1);
  }

  adapter->start(devices);
  // we need to keep this thread alive until all registrate() calls are finished
  this_thread::sleep_for(10ms * INITIAL_MODEL_SIZE * 2);
}

TEST_F(DCAI_TestFixture, canDeregisterDevice) { // NOLINT
  string device_id = "12345";
  EXPECT_CALL(*adapter_mock, deregistrate(device_id)).Times(1);

  auto event = std::make_shared<ModelRepositoryEvent>(device_id);
  event_source->sendEvent(event);
}

TEST_F(DCAI_TestFixture, canStart) { // NOLINT
  EXPECT_CALL(*adapter_mock, start(::testing::_));
  EXPECT_NO_THROW(adapter->start());
}

TEST_F(DCAI_TestFixture, canStop) { // NOLINT
  EXPECT_CALL(*adapter_mock, stop());

  EXPECT_NO_THROW(adapter->stop());
}
