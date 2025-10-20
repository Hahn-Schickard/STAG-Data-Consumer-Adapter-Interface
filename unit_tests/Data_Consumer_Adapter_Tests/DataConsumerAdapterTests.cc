#include "DataConsumerAdapter.hpp"

#include <Information_Model_Mocks/MockBuilder.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <thread>

namespace Data_Consumer_Adapter::testing {
using namespace std;
using namespace Information_Model;
using namespace Information_Model::testing;

struct DataConsumerAdapterMock : DataConsumerAdapter {
  DataConsumerAdapterMock(const string& name, const DataConnector& connector)
      : DataConsumerAdapter(name, connector) {
    ON_CALL(*this, start).WillByDefault([this]() {
      DataConsumerAdapter::start();
    });
    ON_CALL(*this, stop).WillByDefault([this]() {
      DataConsumerAdapter::stop();
    });
  }

  MOCK_METHOD(
      void, registrate, (const Information_Model::DevicePtr&), (override));
  MOCK_METHOD(void, deregistrate, (const string&), (override));
  MOCK_METHOD(void, start, (), (override));
  MOCK_METHOD(void, stop, (), (override));
};

using DataConsumerAdapterMockPtr = shared_ptr<DataConsumerAdapterMock>;

struct FakeSource {
  DataConnectionPtr connect(const DataNotifier& notifier) {
    auto connection = make_shared<FakeConnection>(notifier);
    connections_.emplace_back(connection);
    return connection;
  }

  void notify(const RegistryChangePtr& event) {
    for (auto it = connections_.begin(); it != connections_.end();) {
      if (auto connection = it->lock()) {
        connection->call(event);
        ++it;
      } else {
        it = connections_.erase(it);
      }
    }
  }

private:
  struct FakeConnection : DataConnection {
    FakeConnection(const DataNotifier& notifier) : notify_(notifier) {}

    void call(const RegistryChangePtr& event) { notify_(event); }

  private:
    DataNotifier notify_;
  };

  using WeakConnectionPtr = weak_ptr<FakeConnection>;
  vector<WeakConnectionPtr> connections_;
};

using FakeSourcePtr = shared_ptr<FakeSource>;

struct DataConsumerAdapterTests : public ::testing::Test {
  void SetUp() override {
    source = make_shared<FakeSource>();
    auto connector = [weak_source = weak_ptr<FakeSource>(source)](
                         const DataNotifier& notifier) {
      if (auto source = weak_source.lock()) {
        return source->connect(notifier);
      } else {
        return DataConnectionPtr();
      }
    };
    mock = make_shared<DataConsumerAdapterMock>("Mock adapter", connector);
    tested = mock;
  }

  void TearDown() override {
    tested.reset();
    mock.reset();
    source.reset();
  }

  FakeSourcePtr source;
  DataConsumerAdapterMockPtr mock;
  DataConsumerAdapterPtr tested;
};

DevicePtr makeDevice(const string& id) {
  auto builder = make_unique<MockBuilder>();
  builder->setDeviceInfo(id, BuildInfo{"Mock", "Mock device"});
  builder->addReadable(
      BuildInfo{"readable", "readable metric mock"}, DataType::Boolean);
  return builder->result();
}

TEST_F(DataConsumerAdapterTests, canRegisterDevice) {
  auto device = makeDevice("12345");
  EXPECT_CALL(*mock, registrate(device)).Times(1);

  auto event = make_shared<RegistryChange>(device);
  source->notify(event);
}

constexpr size_t INITIAL_MODEL_SIZE = 5;

TEST_F(DataConsumerAdapterTests, canInitialiseModel) {
  vector<DevicePtr> devices;
  devices.reserve(INITIAL_MODEL_SIZE);
  for (size_t i = 0; i < INITIAL_MODEL_SIZE; ++i) {
    devices.emplace_back(makeDevice(to_string(i)));
  }

  EXPECT_FALSE(devices.empty());
  EXPECT_EQ(INITIAL_MODEL_SIZE, devices.size());

  for (const auto& device : devices) {
    EXPECT_CALL(*mock, registrate(device)).Times(1);
  }

  tested->start();
  auto init_model_thread = thread(
      [&devices, weak_adapter = weak_ptr<DataConsumerAdapter>(tested)]() {
        if (auto adapter = weak_adapter.lock()) {
          adapter->initialiseModel(devices);
        }
      });
  init_model_thread.join(); // block until the task is finished
  EXPECT_FALSE(init_model_thread.joinable());
}

TEST_F(DataConsumerAdapterTests, canDeregisterDevice) {
  string device_id = "12345";
  EXPECT_CALL(*mock, deregistrate(device_id)).Times(1);

  auto event = make_shared<RegistryChange>(device_id);
  source->notify(event);
}

TEST_F(DataConsumerAdapterTests, canStart) {
  EXPECT_CALL(*mock, start());

  EXPECT_NO_THROW(tested->start());
}

TEST_F(DataConsumerAdapterTests, canStop) {
  EXPECT_CALL(*mock, stop());

  EXPECT_NO_THROW(tested->stop());
}
} // namespace Data_Consumer_Adapter::testing