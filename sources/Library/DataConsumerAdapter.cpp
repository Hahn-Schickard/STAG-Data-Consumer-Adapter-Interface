#include "DataConsumerAdapter.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <Variant_Visitor/Visitor.hpp>

#include <stdexcept>
#include <thread>

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

void DataConsumerAdapter::start() { logger->info("Started!"); }

void DataConsumerAdapter::stop() { logger->info("Stopped"); }

void DataConsumerAdapter::initialiseModel(const Devices& devices) {
  auto registrate_lock = scoped_lock(mx_);
  for (const auto& device : devices) {
    registerDevice(device);
  }
}

void DataConsumerAdapter::handleEvent(const ModelRepositoryEventPtr& event) {
  Variant_Visitor::match(
      *event,
      [this](const DevicePtr& device) {
        auto registrate_lock = scoped_lock(mx_);
        registerDevice(device);
      },
      [this](const string& device_id) {
        try {
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
