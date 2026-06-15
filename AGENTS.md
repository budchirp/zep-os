# Zep OS - Agent Guidelines

## Core Principles

* Preserve architectural consistency over convenience.
* Prefer compile-time guarantees over runtime checks.
* Prefer explicitness over magic.
* Prefer simple data flow over abstractions.
* Keep modules cohesive and isolated.
* Every line of code must justify its existence.

---

# Building, Running, and Testing

## Requirements

* CMake 3.30+
* Ninja
* Clang with full C++26 support
* LLVM toolchain available in `PATH`

## Configure Debug

```bash
cmake --preset debug
```

## Build Debug

```bash
cmake --build cmake-build-debug
```

## Configure Release

```bash
cmake --preset release
```

## Build Release

```bash
cmake --build cmake-build-release
```

## Clean Reconfigure

```bash
rm -rf cmake-build-debug cmake-build-release
cmake --preset debug
```

---

# Project Structure

## Language Rules

* Entire codebase uses C++26.
* All code MUST use modules.
* Module files MUST use `.cppm`.
* Traditional headers are forbidden unless required by third-party libraries.

## Build Rules

* Build directories:

  * `cmake-build-debug`
  * `cmake-build-release`

* Modules are auto-discovered through:

  ```cmake
  GLOB_RECURSE
  ```

* Every module MUST live inside a `src/` directory.

* Do not manually register source files unless explicitly necessary.

---

# Naming Conventions

## Files

* MUST use `snake_case`
* Examples:

  * `lexer.cppm`
  * `type_checker.cppm`

## Classes

* MUST use `PascalCase`
* Examples:

  * `Parser`
  * `SemanticAnalyzer`

## Enums

Enums MUST use wrapper classes.

Correct:

```cpp
class Backend {
public:
    enum class Type : std::uint8_t {
        LLVM,
        C
    };
};
```

Forbidden:

```cpp
enum class BackendType {
    LLVM
};
```

## Enum Values

* MUST use `PascalCase`

## Functions and Methods

* MUST use `snake_case`

Correct:

```cpp
parse_expression()
```

Forbidden:

```cpp
ParseExpression()
parseExpression()
```

## Variables, Parameters, and Fields

* MUST use `snake_case`

Correct:

```cpp
source_location
token_stream
```

Forbidden:

```cpp
sourceLocation
source_location_
```

## Pointer Naming

* Only `ptr` abbreviation is allowed.

Correct:

```cpp
Node* node_ptr
```

Forbidden:

```cpp
Node* node
Node* n
```

---

# Type Usage Rules

## Type Deduction

* Use `auto` whenever the type is obvious from the initializer.
* Function return types MUST always be explicit.
* `auto` MUST NOT be used for class instantiation.

Correct:

```cpp
std::vector<Token> tokens;

auto iterator = tokens.begin();
```

Forbidden:

```cpp
auto parser = Parser();
auto create_parser();
```

## Explicit Conversions

Implicit conversions are forbidden.

Correct:

```cpp
auto size = static_cast<std::uint32_t>(buffer.size());
```

Forbidden:

```cpp
std::uint32_t size = buffer.size();
```

## Constructors

* All constructors MUST be `explicit` unless impossible.

Correct:

```cpp
explicit Parser(Context& context)
```

Forbidden:

```cpp
Parser(Context& context)
```

---

# Class Rules

## General

* ALWAYS use `class`.
* `struct` is forbidden.
* Classes MUST initialize all members.

## Member Ordering

Member order MUST always be:

1. `private`
2. `protected`
3. `public`

Example:

```cpp
class Parser {
private:
    Context& context;

protected:
    void sync();

public:
    explicit Parser(Context& context);
};
```

## Field Access

* Do NOT create trivial getters/setters.
* Public data is preferred over meaningless wrappers.

Correct:

```cpp
class Token {
public:
    TokenType::Type type;
};
```

Forbidden:

```cpp
class Token {
private:
    TokenType::Type type;

public:
    TokenType::Type get_type() const;
};
```

## Naming Collisions

Constructor parameters MUST use the same name as fields.

Correct:

```cpp
class Parser {
private:
    Context& context;

public:
    explicit Parser(Context& context)
        : context(context) {}
};
```

Forbidden:

```cpp
class Parser {
private:
    Context& context_;

public:
    explicit Parser(Context& ctx)
        : context_(ctx) {}
};
```

---

# Pointer Rules

## Null Checks

Pointers MUST always be compared explicitly.

Correct:

```cpp
if (node_ptr == nullptr) {
    return;
}
```

Forbidden:

```cpp
if (!node_ptr) {
    return;
}
```

## Ownership

* Raw pointers indicate non-ownership.
* `std::unique_ptr` indicates ownership.
* `std::shared_ptr` is forbidden unless explicitly approved.

---

# Module Rules

## Layout

* Headers and sources MUST NOT be separated.
* Entire implementation stays inside the module file.

Correct:

```text
parser.cppm
```

Forbidden:

```text
parser.hpp
parser.cpp
```

## Imports

* Prefer module imports over includes.
* `#include` should only exist for:

  * standard library compatibility
  * third-party libraries
  * compiler limitations

---

# Formatting Rules

## Spacing

* Keep code spacious.
* Separate logical blocks with blank lines.
* Avoid dense vertical packing.

Correct:

```cpp
auto lexer = Lexer(source);

lexer.tokenize();

Parser parser(context);

parser.parse();
```

Forbidden:

```cpp
auto lexer=Lexer(source);
lexer.tokenize();
Parser parser(context);
parser.parse();
```

## Line Width

* Prefer staying under 100 columns.
* Break long expressions vertically.

## Braces

* Always use braces.
* Single-line implicit blocks are forbidden.

Forbidden:

```cpp
if (failed)
    return;
```

Correct:

```cpp
if (failed) {
    return;
}
```

---

# Architecture Rules

## Modification Policy

* NEVER make unrelated changes.
* NEVER silently refactor surrounding code.
* NEVER introduce new abstractions without approval.
* ALWAYS ask before changing architecture.

## Before Writing Code

You MUST first determine:

1. Ownership model
2. Lifetime model
3. Threading implications
4. ABI implications
5. Compile-time impact
6. Error propagation strategy

## Planning

When creating plans for weaker models or contributors:

* Include concrete examples.
* Include expected directory layout.
* Include exact APIs.
* Include before/after examples.
* Avoid vague instructions.

Example:

```cpp
class DiagnosticEngine {
private:
    std::vector<Diagnostic> diagnostics;

public:
    void report(const Diagnostic& diagnostic);
};
```

Bad instruction:

```text
Add diagnostics support.
```

---

# Forbidden Patterns

## Absolutely Forbidden

* `using namespace`
* Macros for logic
* Hidden ownership
* Singleton globals
* Cyclic dependencies
* God objects
* Boolean parameter flags
* Magic numbers
* Implicit allocations in hot paths
* Exceptions for control flow
* RTTI-dependent design
* Premature abstraction
* Deep inheritance trees
* Virtual dispatch without justification

## Avoid Unless Approved

* `std::shared_ptr`
* Dynamic polymorphism
* Custom allocators
* Template metaprogramming
* Coroutines
* Global mutable state

---

# Error Handling

* Errors MUST be explicit.
* Prefer:

  * `std::optional`
  * `std::expected`
  * result objects

Forbidden:

```cpp
throw std::runtime_error("failed");
```

Preferred:

```cpp
auto result = parse();

if (!result.has_value()) {
    return std::nullopt;
}
```

---

# Performance Rules

* Minimize allocations.
* Avoid unnecessary copies.
* Move explicitly when ownership transfer is intended.
* Prefer stack allocation.
* Measure before optimizing.
* Do not sacrifice readability for speculative performance.

---

# Review Checklist

Before submitting code, verify:

* Naming follows conventions.
* No unnecessary abstractions exist.
* Ownership is explicit.
* No hidden allocations exist.
* No unrelated files were modified.
* No comments were added.
* No getters/setters were introduced.
* No implicit conversions exist.
* All pointers are explicitly checked.
* Formatting is spacious and readable.
* APIs are minimal and coherent.
* Module boundaries remain clean.

If any rule is violated, fix it before submission.
