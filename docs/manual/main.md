# Data Consumer Adapter Interface {#mainpage}

This module provides an interface to the Information Model Repository for various Data Consumer Adapters (DCAs). DCA implementations are managed by the system plugin manager, which controls the lifetimes of all the system modules.

@note Interface definition, inheritance and collaboration diagrams can be found in Data_Consumer_Adapter::DataConsumerAdapter reference page. 

@attention Data Consumer Adapter Implementers **SHOULD** be familiar with @ref ImplementationGuidelines and @ref AdapterLifetime guidelines and are expected to follow them.

@important Any Data Consumer Adapter implementations **MUST** implement the Data_Consumer_Adapter::DataConsumerAdapter struct for it to be detected by the system plugin manager.

A guide on how to use the Device abstraction can be found in [Using the Device abstraction](#consuming_devices) chapter

