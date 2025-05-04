# JSBJSON

JSBJSON is a lightweight C++ library designed for creating, manipulating, and parsing JSON objects. It provides a simple and intuitive interface for working with nested JSON objects, arrays, and complex data structures.

## Features

- Define JSON objects and their schema using macros like `JsonObjectBegin` and `JsonObjectEnd`.
- Serialize C++ objects to JSON strings.
- Parse JSON strings back into C++ objects.
- Handle nested objects, arrays, and various data types seamlessly.
- Modify JSON data dynamically.

## Example Usage

### Defining JSON Objects

```cpp
JsonObjectBegin(price)
    JsonAddMember(currency, std::string);
    JsonAddMember(value, uint64_t);
JsonObjectEnd(2);

JsonObjectBegin(complex)
    JsonAddMember(description, std::string);
    JsonAddObjectMember(price);
    JsonAddMember(values, std::vector<int64_t>);
JsonObjectEnd(3);
```

### Serializing and Parsing Objects

```cpp
#include "jsbjson.h"
#include <iostream>

int main() {
    complex lComplex;
    lComplex.description = "This is a fruit";
    lComplex.price.currency = "$";
    lComplex.price.value = 12;
    lComplex.values = {1, 2, 3};

    // Serialize object to JSON
    std::cout << "Generated JSON string:" << std::endl;
    std::cout << jsbjson::FromObject {}(lComplex) << std::endl;

    // Parse JSON back into object
    std::optional<complex> lParsedComplex = jsbjson::ToObject<complex> {}(jsbjson::FromObject {}(lComplex));
    if (lParsedComplex.has_value()) {
        std::cout << "Successfully parsed JSON back into object!" << std::endl;
    }
}
```

### Modifying JSON Dynamically

```cpp
jsbjson::JsonElementEx lMyObject;
lMyObject["description"] = std::string("This is a test object");
lMyObject["arrayOfNumber"] = std::vector<jsbjson::JsonVariant>{1, 2, 3, 4, 5, 6};

// Output the JSON
std::cout << "Generated JSON:" << std::endl;
std::cout << lMyObject.ToJson() << std::endl;

// Modify a value
lMyObject["description"] = std::string("Modified description");
std::cout << "Modified JSON:" << std::endl;
std::cout << lMyObject.ToJson() << std::endl;
```

### Output Example

```json
{
  "description": "This is a fruit",
  "price": {
    "currency": "$",
    "value": 12
  },
  "values": [1, 2, 3]
}
```

## Getting Started

### Prerequisites

- A C++17 compatible compiler.

### Installation

Clone the repository and include the necessary headers in your project:

```bash
git clone https://github.com/josephb01/jsbjson.git
```

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License.