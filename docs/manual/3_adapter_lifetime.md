# Adapter lifetime {#AdapterLifetime}

## STAG System activity diagram

The diagram below showcases a simplified STAG system activity overview when it comes to processing **Data Consumer Adapter** implementations.

@startuml
|Setup|
start
:STAG system is started; <<input>>
:Configure STAG system;
|Adapter construction|
while (data consumer adapter available?) is (yes)
   :**create data consumer adapter**;
   :**initialize model**;
   :add to built adapter list;
endwhile (no)
|Starting Adapters|
while (has next adapter in built adapters) is (yes)
    :get adapter;
    :**start adapter**;
endwhile (no)
|Running|
:normal operation; <<continuous>>
|Stopping Adapters|
:STAG shutdown is triggered; <<input>>
while (has next adapter in built adapters) is (yes)
    :get adapter;
    :**stop adapter**;
endwhile (no)
:stop STAG system;
stop
@enduml

### Adapter construction 
Each adapter implementation is first constructed when STAG system is started or after system start, once the adapter was detected.

### Model Initialization {#ModelInitialization} 
After the adapter was created, but before it is started, STAG system will use `Data_Consumer_Adapter::DataConsumerAdapter::initialiseModel()` to pass the latest available model snapshot to the data consumer adapter implementation. This method will be called asynchronously and will block `Data_Consumer_Adapter::DataConsumerAdapter::registrate()` and `Data_Consumer_Adapter::DataConsumerAdapter::deregistrate()` methods until it is finished. This method is only called once during entire adapter lifetime. 

### Starting Adapters
After the model snapshot was set for the adapter, STAG system will call `Data_Consumer_Adapter::DataConsumerAdapter::start()` method for every adapter implementation that was successfully constructed. Once called, adapter implementations can start the interacting with the Device abstraction from the provided model snapshot as well as handling any new `Data_Consumer_Adapter::DataConsumerAdapter::registrate()` and `Data_Consumer_Adapter::DataConsumerAdapter::deregistrate()` calls. Adapter implementations **MUST NOT** block `Data_Consumer_Adapter::Data ConsumerAdapter::start()` call indefinitely and must return as soon as possible. 

### Stopping Adapters
On system shutdown, STAG system will call `Data_Consumer_Adapter::DataConsumerAdapter::stop()` method for every running adapter implementation. Once this happens, no interactions with Devices abstractions should be made, any existing operations canceled and any communication processes stopped as soon as possible. Once all communication processes are stopped and all existing operations were canceled, the adapter implementation may return from `Data_Consumer_Adapter::DataConsumerAdapter::stop()` call.
