#include "DataConsumerAdapter.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <Variant_Visitor/Visitor.hpp>

namespace Data_Consumer_Adapter {
using namespace std;
using namespace Information_Model;

DataConsumerAdapter::DataConsumerAdapter(
    const string& name, const DataConnector& connector)
    : logger(HaSLL::LoggerManager::registerLogger(name)), name_(name) {
  logger->trace("DataConsumerAdapter::DataConsumerAdapter({})", name_);
  connection_ = connector(
      bind(&DataConsumerAdapter::handleEvent, this, placeholders::_1));
}

string DataConsumerAdapter::name() const { return name_; }

void DataConsumerAdapter::start() { logger->info("Started"); }

void DataConsumerAdapter::stop() { logger->info("Stopped"); }

void DataConsumerAdapter::initialiseModel(const Devices& devices) {
  auto event_lock = scoped_lock(mx_);
  logger->trace("Initialising Data Consumer Adapter");
  for (const auto& device : devices) {
    registerDevice(device);
  }
}

void DataConsumerAdapter::handleEvent(const RegistryChangePtr& event) {
  auto event_lock = scoped_lock(mx_);
  Variant_Visitor::match(
      *event,
      [this](const DevicePtr& device) {
        logger->trace("Registering a new device {}", device->id());
        registerDevice(device);
      },
      [this](const string& device_id) {
        try {
          logger->trace("Removing device {}", device_id);
          deregistrate(device_id);
        } catch (const exception& ex) {
          logger->error(
              "{} Data Consumer Adapter encountered an unhandled exception "
              "while deregistrating device {}. Exception: {}",
              name_, device_id, ex.what());
        }
      });
}

void DataConsumerAdapter::registerDevice(const DevicePtr& device) {
  try {
    registrate(device);
  } catch (const exception& ex) {
    logger->error("{} Data Consumer Adapter encountered an unhandled exception "
                  "while registrating device {}. Exception: {}",
        name_, device->id(), ex.what());
  }
}
} // namespace Data_Consumer_Adapter
