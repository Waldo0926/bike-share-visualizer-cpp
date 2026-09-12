# Contributing

This started as a personal coursework-modernisation project, but issues and
pull requests are welcome.

## Building

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Requires a C++17 compiler and CMake 3.16+. No third-party dependencies.

## Style

- Keep the core library (`include/bikeviz`, `src/`) free of platform-specific
  or UI code — it should build the same way on Linux, macOS and Windows.
- Every new behaviour should come with a test in `tests/bikeviz_tests.cpp`.
- Run `ctest` before opening a pull request; CI runs the same build on
  Ubuntu, macOS and Windows.
