# Changelog
## [0.4.1] - 2025.11.10
### Added
 - `DataConsumerAdapterNotImplemented` exception

### Changed
 - `DataConsumerAdapterInterface::registrate()` back to be a virtual method with 
 default implementation
 - `DataConsumerAdapterInterface::deregistrate()` back to be a virtual method with 
 default implementation
 - `DataConsumerAdapterInterface::handleEvent()` method to log `DataConsumerAdapterNotImplemented` 
 exception as a warning
 - `DataConsumerAdapterInterface` destructor to first release the `DataConnection`

## [0.4.0] - 2025.11.11
### Added
 - `DataConnection` class
 - `DataConnector` callable
 - `Variant_Visitor` v0.2 as an invisible dependency
 - `Information_Model_Mocks` v0.1 as a test requires dependency
 - transparent requires for `HaSLL` and `Information_Model` dependencies

### Changed
 - `ModelRepositoryEvent` into `RegistryChange`
 - `DataConsumerAdapterInterface::initialiseModel()` to be public
 - `DataConsumerAdapterInterface::registrate()` to be pure virtual
 - `DataConsumerAdapterInterface::deregistrate()` to be pure virtual
 - `DataConsumerAdapterInterface::event_mx_` into `DataConsumerAdapterInterface::mx_`
 - `DataConsumerAdapterInterface` into `DataConsumerAdapter`
 - `Information_Model` dependency to v0.5
 - conan package type to library
 
### Removed
 - `DataConsumerAdapterInterface::init_thread_` field
 - `Event_Model` dependency
 - direct `GTest` dependency

## [0.3.0] - 2025.03.06
### Added 
 - Windows 10 support 
 - `GTest` v1.16 as a **build_requires** dependency
 - `~DataConsumerAdapterInterface` dtor implementation
 - `DataConsumerAdapterInterface::name()` method
 - `DataConsumerAdapterInterface::Devices` alias for `std::vector<Information_Model::DevicePtr>`
 - `DataConsumerAdapterInterface::init_thread_` field to managed model initialization and cleanup

### Changed 
 - `Information_Model` dependency to v0.4
 - `HaSLL` dependency to v0.4
 - `DataConsumerAdapterInterface::name` field to be **private**
 - `DataConsumerAdapterInterface::logger` field to be **protected**
 - `virtual void DataConsumerAdapterInterface::registrate(Information_Model::NonemptyDevicePtr)` into `virtual void DataConsumerAdapterInterface::registrate(const Information_Model::NonemptyDevicePtr&)`
 - `virtual void DataConsumerAdapterInterface::start(std::vector<Information_Model::DevicePtr> devices = {})` into `virtual void DataConsumerAdapterInterface::start(const Devices& devices)`
 - `void DataConsumerAdapterInterface::initialiseModel(std::vector<Information_Model::DevicePtr>)` into  `void DataConsumerAdapterInterface::initialiseModel(const Devices&)`
 - `void DataConsumerAdapterInterface::registerDevice(Information_Model::NonemptyDevicePtr)` into `void DataConsumerAdapterInterface::registerDevice(const Information_Model::NonemptyDevicePtr&)`
 - `std::lock_guard` instances into `std::scoped_lock`

### Removed
 - explicit `Event_Model` dependency
 - `DataConsumerAdapterInterface` copy ctor and assignment operator

## [0.2.2] - 2023.07.28
### Added
 - STAG logo to documentation 

### Removed 
 - code coverage hooks from conan package 

## [0.2.1] - 2023.07.24
### Changed
 - conan recipe to use conan v2 syntax
 - CMake requirement to 3.24
 - conan cmake integration to use conan v2 engine

## [0.2.0] - 2023.06.28
### Added
 - `DataConsumerAdapterInterface::registrate()` 
 - `DataConsumerAdapterInterface::deregistrate()`
 - `DataConsumerAdapterInterface::initialiseModel()`
 - `DataConsumerAdapterInterface::registerDevice()`

### Changed
 - Information_Model dependency to fuzzy v0.3
 - `DataConsumerAdapterInterface::start()` to accept `std::vector<Information_Model::DevicePtr>` parameter
 - `DataConsumerAdapterInterface::name` to be const public
 - `DataConsumerAdapterInterface::logger` to be const public
 - `DataConsumerAdapterInterface::handleEvent()` to be private
 - `ModelRegistryEvent` into `ModelRepositoryEvent` 

### Removed
 - `DataConsumerAdapterInterface::getAdapterName()` 

## [0.1.10] - 2022.11.21
### Changed
 - conan packaging recipe
 - gtest dependency to fuzzy v1.11
 - Information_Model dependency to fuzzy v0.2
 - HaSLL dependency to fuzzy v0.3
 - Event_Model dependency to fuzzy v0.3

## [0.1.9] - 2022.11.03
### Changed
 - Information_Model dependency version to 0.2.1
 - Event_Model dependency version to 0.3.4
 - HaSLL dependency version to 0.3.2
 - `ModelRegistryEvent` to use `NonemptyDevicePtr`
 - `std::shared_ptr<HaSLL::Logger>` into `HaSLI::LoggerPtr` 
 - `HaSLL::LoggerRepository::getInstance().*` into `HaSLI::LoggerManager::*`
 - change logger repository initialization mechanism for example and test runners
 - Contribution rules

### Added 
 - `ModelRegistryEventPtr` alias
 - `override` specifier for `handleEvent()` method implementation in main.cpp
 - `printException()` to handle nested exceptions

### Removed 
 - superfluous `override` specifiers

### Fixed
 - notifying typo in main and example cpp modules
 - nonsense typo in main and example cpp modules
 - Notice file to only include used Licenses

## [0.1.8] - 2021.11.05
### Changed
  - Information_Model to 0.1.6

### Fixed
 - typos in Changelog

## [0.1.7] - 2021.09.17
### Changed
  - Event_Model to 0.3.1

## [0.1.6] - 2021.08.09
### Changed
 - Event_Model to 0.3.0
 - integration test to use `AsyncEventSource`
 - integration test to fully utilize logging
 - integration test to test for exception handling

## [0.1.5] - 2021.04.01
### Changed
 - Information_Model to 0.1.5
 - Event_Model to 0.2.0
 - include paths to resolve potential header name collisions
 - example.cpp in conan package integration test to use new include paths

### Added
 - main runner to showcase usage
 - valgrind checks

## [0.1.4] - 2020.11.16
### Changed
 - Information_Model to 0.1.4
 - runValgrind.py script to use better error detection

## [0.1.3] - 2020.10.22
### Removed
 - Model_Event_Handler

### Added
 - Information Model 0.1.2
 - Event_Model 0.1.0
 - ModelRegistryEvent
 - getLogger method
 - test case to get adapter logger
 - delegate to start method for `DataConsumerAdapterInterfaceMock`
 - delegate to stop method for `DataConsumerAdapterInterfaceMock`

 ### Changed
  - `DataConsumerAdapterInterface` to implement `Event_Model::EventListener`
  - unit tests to use Event_Model
  - adapter_logger to private
  - Information_Model to 0.1.3
  - main.md

## [0.1.2] - 2020.10.02
### Changed
 - Model Event Handler to v0.1.1
 - `DataConsumerAdapterInterface` start and stop methods declaration

### Added
 - Documentation to `DataConsumerAdapterInterface` start and stop methods

### Removed
 - Stoppable class

## [0.1.1] - 2020.09.28
### Changed
 - Model Event Handler to v0.1.1

## [0.1.0] - 2020.09.21
### Changed
 - CMakeLists.txt
 - Readme to reflect conan and python 3 dependencies
 - utility scripts to work with python3

### Removed
 - Cpack configuration
 - ExternalProjectAdd usage
 - Google test dependency

### Added
 - conan package configuration
 - conan package integration test
 - Contributing
 - Changelog
 - License
 - Notice
 - Authors
 - Separate _MOCKs
 - Gtests runner
 - global logger instance for unit tests

## [0.0.1] - 2019.12.07
### Added
 - `DataConsumerAdapterInterface`


## [Initial Commit] - 2019.12.07
### Added
 - Set up project on Cpp Project Template v0.8.27
