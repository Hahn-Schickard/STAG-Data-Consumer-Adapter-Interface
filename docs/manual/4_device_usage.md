# Using the Device abstraction {#consuming_devices} 

The following sections provide an abbreviated documentation for the Information Model module. For a more detailed documentation, please see [Information Model](https://stag.hahn-schickard.de/404) documentation page.

## Using Devices

`Information_Model::Device` is used to represent various technology endpoint abstractions. It provides a partial mixture of [Element](#elements) and [Group](#groups) functionalities and is the main way of interacting with various technology adapters. The following class diagram displays all of the available methods of the `Information_Model::Device` interface.

@startuml
hide empty members

interface Device {
    + {abstract} id(): std::string
    + {abstract} name(): std::string
    + {abstract} description(): std::string
    + {abstract} size(): size_t
    + {abstract} group(): GroupPtr
    + {abstract} element(ref_id: std::string): ElementPtr
    + {abstract} visit(visitor: Visitor): void
}

@enduml

Each `Information_Model::Device` has a unique ID, obtained by calling  `Information_Model::Device::id()` method. This ID is used to identify it and it's elements. The ID value will not contain any whitespaces, however other special characters like `_`, `.`, `:`, `@`, `#` are allowed. Devices also have names, obtained by calling `Information_Model::Device::name()` method, and descriptions, obtained by calling  `Information_Model::Device::description()` method. These values provide more information about said device, however there is no guarantee that they will be unique between different devices. There are no limitations on name and description values.

Each device also works like a `Information_Model::Group`. You can access the entire `Information_Model::Group` interface by calling the `Information_Model::Device::group()` method, but some of the most used methods like `Information_Model::Device::size()`, `Information_Model::Device::element()` and `Information_Model::Device::visit()` are also exposed to avoid having to call the `Information_Model::Device::group()` method every time. For more information on what and how groups work, please see the [Group](#groups) section.

## Using Elements {#elements}

`Information_Model::Element` is used to represent a single capability of it's parent device, which fall into one of the following categories, each of which will be discussed later on:
 - [Group](#groups) - used to group other elements together
 - [Readable](#readables) - represents the read capability
 - [Writable](#writables) - represents the read and/or write capability
 - [Observable](#observables) - represents the read capability and a provides a mechanism for receiving value change notifications
 - [Callable](#callables) - represents the function call capability

The following class diagram displays all of the available methods of the `Information_Model::Element` interface and any related classes or interfaces.

@startuml
hide empty members

enum ElementType {
  <b>Group</b>
  <b>Readable</b>
  <b>Writable</b>
  <b>Observable</b>
  <b>Callable</b>
}

dataclass ElementFunction <<variant>> {}

note left of ElementFunction
<i>{xor}</i>: Can only contain:
a <b>Group</b> or
a <b>Readable</b> or
a <b>Writable</b> or
a <b>Observable</b> or
a <b>Callable</b>
endnote

interface Group{}
interface Readable{}
interface Writable{}
interface Observable{}
interface Callable{}

interface Element {
  + {abstract} id(): std::string
  + {abstract} name(): std::string
  + {abstract} description(): std::string
  + {abstract} type(): ElementType
  + {abstract} function(): ElementFunction
}

Element -- ElementType
Element o-- ElementFunction

ElementFunction o-- Group
ElementFunction o-- Readable
ElementFunction o-- Writable
ElementFunction o-- Observable
ElementFunction o-- Callable

@enduml

Similar to `Information_Model::Device`, each `Information_Model::Element` has a unique auto-generated ID, obtained by calling the `Information_Model::Element::id()` method. This ID is based on `Information_Model::Device::id()` value and works like a filesystem path. The base device ID is separated from the rest of the numeric ids with a `:` character, every group reference id ends with a `.` character, and the remaining part is the element ID.

For example:
* The full generated ID of the first root element in a device with base ID `DEVICE_ID_1` will be: `DEVICE_ID_1:0`
* The full generated ID of the second sub-element in a third element in a device with base ID `DEVICE_ID_1` will be: `DEVICE_ID_1:1:2`

Just like devices, each `Information_Model::Element` can also have a name, obtained by calling `Information_Model::Element::name()` method, and a description, obtained by calling  `Information_Model::Element::description()` method. However there is no guarantee that these values will be unique between different elements.

`Information_Model::ElementFunction` variant is used to store the concrete capabilities of this elements. These capabilities will be covered in the sections bellow.

### Using Groups {#groups}

`Information_Model::Group` is used to group various `Information_Model::Element` together to create a logical structures. Groups can also be used to nest elements within other groups, thus allowing to organize elements in a hierarchy. `Information_Model::Device` always works as a the root level of this hierarchy. The following class diagram displays all of the available methods of the `Information_Model::Group` interface.


@startuml
hide empty members

interface Group {
    + {abstract} size(): size_t
    + {abstract} asMap(): std::unordered_map<std::string, ElementPtr>
    + {abstract} asVector(): std::vector<ElementPtr>
    + {abstract} element(ref_id: std::string): ElementPtr
    + {abstract} visit(visitor: Visitor): void
}

@enduml

The `Information_Model::Group::size()` method is used to get the number of directly stored elements within the group, however this number will not include any nested elements. 

You can access any element within the device if you know it's reference ID by calling the `Information_Model::Group::element(const std::string& ref_id)`. If the given reference ID does not point an element that exists within the device, you will get an `Information_Model::ElementNotFound` exception.

For convenience, `Information_Model::Group` provides a couple of view methods that return standard iterable C++ containers:
* `Information_Model::Group::asMap()` - which returns an unordered map of all of the elements stored within this group and index by their reference ids
* `Information_Model::Group::asVector()` - which returns a sorted vectored of all of the elements stored within this group. The resulting vector is sorted in an ascending element reference id order. 

These methods are not very efficient in regards to memory usage and execution time, since they have to create new unordered map and vector instances for each call, as well as sort the element order in the vector case. However they do allow the use of standard C++ iteration mechanism.

For a more efficient, albeit somewhat random, access to all of the elements within the group, you can use the visitor pattern. To do this you will need to define a visitor callable with `std::function<void(const ElementPtr&)>` call signature and pass it to the `Information_Model::Group::visit(const std::function<void(const ElementPtr&)>& visitor)` method. This will pass every element within the group to your callable where you can process it, just take note that you will have handle element nesting within this callable if you need to go thought all of the available elements. Here is a very rudimentary example on how to define a visitor callable, that is able to visit even the nested elements:

```cpp
using namespace std;
using namespace Information_Model;

// Declare the elementVisitor so we can use it in groupVisitor
void elementVisitor(const ElementPtr& element);

void groupVisitor(const GroupPtr& group){
  // go in deeper into nested groups
  group->visit(&elementVisitor);
}

void elementVisitor(const ElementPtr& element){
  if (holds_alternative<GroupPtr>(element->function())){
    auto group = get<GroupPtr>(element->function());
    groupVisitor(group);
  } else if (holds_alternative<ReadablePtr>(element->function())){
    auto readable = get<ReadablePtr>(element->function());
    // handle readable element
  } else if (holds_alternative<WritablePtr>(element->function())){
    auto writable = get<WritablePtr>(element->function());
    // handle writable element
  } else if (holds_alternative<ObservablePtr>(element->function())){
    auto observable = get<ObservablePtr>(element->function());
    // handle observable element
  } else {
    auto callable = get<CallablePtr>(element->function());
    // handle callable element
  }
}
```

### Value types {#value_types}

Before describing the concrete device capabilities, it is prudent to go other all of the available data types, that can be used by them. 

`Information_Model::Device` abstractions allow 7 data types: 
 1. Boolean values - stored as standard C++ `bool`
 2. Signed integer values - stored as standard C++ `intmax_t`
 3. Unsigned integer values - stored as standard C++ `uintmax_t`
 4. Double floating precision values - stored as standard C++ `double`
 5. Timestamp values - custom defined date timestamp with microsecond precision
 6. Byte vector values - stored as standard C++ `vector<uint8_t>`
 7. String values - stored as standard C++ `string`

These data types are stored within a standard c++ `variant` container as shown in the class diagram bellow:

@startuml
hide empty members

dataclass DataVariant <<variant>> {}

note left of DataVariant
<i>{xor}</i>: Can only contain:
a <b>bool</b> or
a <b>intmax_t</b> or
a <b>uintmax_t</b> or
a <b>double</b> or
a <b>Timestamp</b> or
a <b>vector</b> or
a <b>string</b>
endnote

dataclass bool
dataclass intmax_t
dataclass uintmax_t
dataclass double
dataclass vector <uint8_t>
dataclass string

struct Timestamp {
    + year: uint16_t
    + month: uint8_t
    + day: uint8_t
    + hours: uint8_t
    + minutes: uint8_t
    + seconds: uint8_t
    + microseconds: uint32_t
}

DataVariant o-down- bool
DataVariant o-down- intmax_t
DataVariant o-down- uintmax_t
DataVariant o-down- double
DataVariant o-down- Timestamp
DataVariant o-down- vector
DataVariant o-down- string

@enduml

The above mentioned `Information_Model::Timestamp` struct also has a couple of conversion functions.

Convert a given `Information_Model::Timestamp` into `std::chrono::system_clock::time_point`:
```cpp
std::chrono::system_clock::time_point Information_Model::toTimepoint(const Information_Model::Timestamp&)
``` 

Convert a given `std::chrono::system_clock::time_point` into a `Information_Model::Timestamp`:
```cpp
Information_Model::Timestamp Information_Model::toTimestamp(const std::chrono::system_clock::time_point&)
```

Convert a given `Information_Model::Timestamp` into an ISO-8601 formatted string:
```cpp
std::string Information_Model::toString(const Information_Model::Timestamp&)
```

Convert a given `Information_Model::Timestamp` based on a given [strftime flag](https://en.cppreference.com/w/cpp/chrono/c/strftime) format.
```cpp
std::string Information_Model::toString(const Information_Model::Timestamp&, const std::string&)
```

A data type enumeration indexing the above mentioned variant values is also available. This enumeration is provided by every element that uses the data variant.

@startuml
hide empty members

enum DataType{
    <b>Boolean</b>
    <b>Integer</b>
    <b>Unsigned_Integer</b>
    <b>Double</b>
    <b>Timestamp</b>
    <b>Opaque</b>
    <b>String</b>
    <b>None</b>
    <b>Unknown</b>
}

@enduml

You can use this enumeration to simplify obtaining the stored variant value, without having to write a large if-else block or using a variant visitor pattern.

You can also use it to ensure that your data variant value contains the correct data within by calling: 

```cpp
bool Information_Model::matchVariantType(const Information_Model::DataVariant&, Information_Model::DataType)
```

Information Model also defines a bunch of utility functions for the data variant. For example: 

Get the size of a given `Information_Model::DataVariant`:
```cpp
std::size_t Information_Model::size_of(const Information_Model::DataVariant&)
```

Get the corresponding `Information_Model::DataType` of a given `Information_Model::DataVariant`:
```cpp
DataType Information_Model::toDataType(const Information_Model::DataVariant&)
```

Convert a given `Information_Model::DataVariant` into human readable string:
```cpp
std::string Information_Model::toString(const Information_Model::DataVariant&)
```

Convert a given `Information_Model::DataVariant` into an alphanumeric string:
```cpp
std::string Information_Model::toSanitizedString(const Information_Model::DataVariant&)
```

Create a default value `Information_Model::DataVariant` based on a given `Information_Model::DataType` (returns `std::nullopt` if data type is `Information_Model::DataType::None` or `Information_Model::DataType::Unknown`):
```cpp
std::optional<Information_Model::DataVariant> setVariant(Information_Model::DataType);
```

### Using Readables {#readables}

`Information_Model::Readable` is used to read the latest available value of a specific endpoint that was modeled by `Information_Model::Element`. The read value type never changes throughout the entire `Information_Model::Device` lifetime. The following class diagram displays all of the available methods of the `Information_Model::Readable` interface.

@startuml
hide empty members

interface Readable  {
    + {abstract} dataType(): DataType
    + {abstract} read(): DataVariant
}

@enduml

### Using Writables {#writables}

`Information_Model::Writable` is used to write new value to a specific endpoint that was modeled by `Information_Model::Element`. Some writables, allow to read the value as well, but this is not guaranteed for every writable instance. Same as for the [readable](#readables) elements, the written value type never changes throughout the entire `Information_Model::Device` lifetime. The following class diagram displays all of the available methods of the `Information_Model::Writable` interface.

@startuml
hide empty members

interface Writable  {
    + {abstract} dataType(): DataType
    + {abstract} read(): DataVariant
    + {abstract} isWriteOnly(): bool
    + {abstract} write(value: DataVariant): void
}

@enduml

@note calling the `Information_Model::Writable::read()` method, when the writable element does not support reading, results in a `Information_Model::NonReadable` exception

### Using Observables {#observables}

`Information_Model::Observable` is a special kind of `Information_Model::Readable` element, that allows the user to receive value change notifications as well as the normal read capabilities via the [*Observer*](https://archive.org/details/designpatternsel00gamm/page/292/mode/2up) pattern. The following class diagram displays all of the available methods of the `Information_Model::Observable` interface.


@startuml
hide empty members
left to right direction

interface Observer 

interface Observable   {
    + {abstract} dataType(): DataType
    + {abstract} read(): DataVariant
    + {abstract} subscribe(observe: ObserveCallback, handler: ExceptionHandler): ObserverPtr
}

Observable -- Observer

@enduml

To start receiving notifications, you must first define two callables:
1. The notification handler callable, that uses `std::function<void(const std::shared_ptr<Information_Model::DataVariant>&)>` signature
2. The exception handler callable, that uses `std::function<void(const std::exception_ptr&)>` signature

Once you have these callables, pass them to the `Information_Model::Observable::subscribe(const ObserveCallback&, const ExceptionHandler&)` method and save the resulting `Information_Model::ObserverPtr` instance. Here is a very rudimentary example on how to define such callables and pass them to an observable: 

```cpp
using namespace std;
using namespace Information_Model;

void observerValueChanged(const shared_ptr<DataVariant>& value){
  // handle the value change
}

void handleException(const exception_ptr& exception_ptr){
  try {
    if (exception_ptr){
      rethrow_exception(exception_ptr);
    }
  }
  catch(const exception& ex){
    // handle exception
  }
}

ObservablePtr observable = device->element("some_observable_id");
auto observer = observable->subscribe(&observerValueChanged, &handleException); 
// for as long as the observer object exists, the observerValueChanged callback will be called when a new value change occurs 
```

@important
If you do not save the `Information_Model::Observable::subscribe(const ObserveCallback&, const ExceptionHandler&)` result in a variable, you will receive any notifications.

### Using Callables {#callables}

`Information_Model::Callable` is used to execute a certain function based on given input variables for a specific endpoint. Some callables return a `Information_Model::DataVariant` as a result value, while other only allow for fire and forget type of execution. Similar to other capabilities, the returned result value type as well as supported parameter types never change throughout the entire `Information_Model::Device` lifetime. The following class diagram displays all of the available methods of the `Information_Model::Callable` interface as well as related structures.

@startuml
hide empty members

struct ResultFuture {
    - result_: std::future<DataVariant>
    - id_: std::shared_ptr<uintmax_t>

    + get(): DataVariant 
    + waitFor<class Rep, class Period>(timeout: std::chrono::duration<Rep, Period>): std::future_status
    + id(): uintmax_t
}

struct ParameterType {
    + type: DataType
    + mandatory: bool
}

interface Callable {
    + <<alias>> Parameters: std::unordered_map<uintmax_t, std::optional<DataVariant>>
    + {abstract} execute(parameters: Parameters): void
    + {abstract} call(timeout: uintmaxt_t): DataVariant
    + {abstract} call(parameters: Parameters, timeout: uintmaxt_t): DataVariant
    + {abstract} asyncCall(parameters: Parameters): ResultFuture
    + {abstract} cancelAsyncCall(call_id: uintmaxt_t): void
    + {abstract} resultType(): DataType
    + {abstract} parameterTypes(): std::unordered_map<uintmax_t, ParameterType>
}

Callable -- ParameterType
Callable -- ResultFuture

@enduml

The above mentioned `Information_Model::ResultFuture` structure is used as a wrapper for the [`std::future`](https://en.cppreference.com/w/cpp/thread/future.html) container. It also stores a unique call id, that can be used to cancel the request by calling the `Information_Model::Callable::cancelAsyncCall()` method.

@note
* Calling `Information_Model::Callable::call()` or `Information_Model::Callable::asyncCall()` methods, when the callable does not support result retuning,  will result in  `Information_Model::ResultReturningNotSupported` exception
* Calling `Information_Model::Callable::execute()`, `Information_Model::Callable::call()` or `Information_Model::Callable::asyncCall()` methods, without mandatory parameters being set will result in `Information_Model::MandatoryParameterMissing` exception
* Calling `Information_Model::Callable::execute()`, `Information_Model::Callable::call()` or `Information_Model::Callable::asyncCall()` methods, with mandatory parameters that have no value will result in `Information_Model::MandatoryParameterHasNoValue` exception
* Calling `Information_Model::Callable::execute()`, `Information_Model::Callable::call()` or `Information_Model::Callable::asyncCall()` methods, with wrong parameter types will result in `Information_Model::ParameterTypeMismatch` exception
* Calling `Information_Model::Callable::execute()`, `Information_Model::Callable::call()` or `Information_Model::Callable::asyncCall()` methods, with parameters that are not part of the callable will result in `Information_Model::ParameterDoesNotExist` exception

# Simulating device abstractions

During development, you will not have access to the Information Model Manager, and thus any `Information_Model::Device` instances, however you may wish to test how your code uses `Information_Model::Device` instances. To avoid having to implement your own `Information_Model::Device` mocks, you can use the [Information Model Mocks](https://git.hahn-schickard.de/hahn-schickard/software-sollutions/application-engineering/internal/stag_dev/modules/core/information-model-mocks) project which defines all of the Information Model classes as [GTest](https://google.github.io/googletest/) mocks and provides a convenient mock builder class.
