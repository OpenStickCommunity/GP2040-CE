# GitHub Copilot Instructions for GP2040-CE

## Code Style and Formatting

### Indentation
- **Always use 4 spaces for indentation**
- **Never use tabs** - configure your editor to show tabs and convert them to spaces
- Maintain consistent indentation across all file types (C/C++, CMake, JSON, etc.)

### Examples:
```cpp
// Correct: 4 spaces
void function() {
    if (condition) {
        doSomething();
        if (nested) {
            doNestedThing();
        }
    }
}

// Incorrect: tabs or inconsistent spacing
void function() {
	if (condition) {  // Tab used - WRONG
		doSomething();
  if (nested) {       // 2 spaces - WRONG
    doNestedThing();
  }
	}
}
```

## Platform and SDK Configuration

### Default Target Board
- **Default to Raspberry Pi Pico** as the standard board unless explicitly specified otherwise
- Use `PICO_BOARD=pico` as the default environment variable
- Only suggest other boards (Pico W, Pico 2, Pico 2 W) when specifically requested

### Pico SDK Version
- **Always use Pico SDK version 2.1.1**
- When referencing SDK paths, use: `${env:USERPROFILE}/.pico-sdk/`
- Default tools versions:
  - Ninja: `v1.12.1`
  - Picotool: `2.1.1`
  - OpenOCD: `0.12.0+dev`

### CMake Configuration
- Default build configuration for standard Pico:
  ```cmake
  cmake -B build -S .
  ```
  With environment:
  ```
  PICO_BOARD=pico
  SKIP_WEBBUILD=TRUE
  ```

## Build System Preferences

### Task Usage
- Prefer using VS Code tasks over manual terminal commands
- Default tasks available:
  - `Configure CMake (Standard Pico)` - for standard Pico builds
  - `Configure CMake (Pico W)` - only when WiFi functionality is needed
  - `Compile Project` - for building
  - `Run Project` - for flashing via picotool

### Build Directory
- Always use `build/` directory for CMake builds
- Keep build artifacts in the build directory structure

## File Organization

### Config Files
- Board-specific configurations belong in `configs/` directory
- Default to `configs/Pico/` for standard Pico configurations
- Use `configs/PicoW/` only when WiFi features are required

### Headers and Source
- Include paths should reference `headers/` directory
- Implementation files should be in `src/` directory
- Follow existing project structure and naming conventions

## Development Workflow

### When suggesting code changes:
1. Always check current indentation in existing files and maintain consistency
2. Default to standard Pico unless user specifies another board
3. Use 4-space indentation in all suggested code
4. Reference appropriate SDK version (2.1.1) in build configurations
5. Suggest using existing VS Code tasks when applicable

### When creating new files:
- Use 4-space indentation from the start
- Follow existing project patterns and structure
- Include appropriate headers and dependencies for Pico SDK 2.1.1

## Common Patterns

### CMakeLists.txt files:
```cmake
# Use 4 spaces for indentation
target_sources(target_name PRIVATE
    source1.cpp
    source2.cpp
)

target_include_directories(target_name PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/headers
)
```

### C/C++ code:
```cpp
// Always 4 spaces
#include "pico/stdlib.h"

void setup() {
    stdio_init_all();
    
    if (condition) {
        performAction();
    }
}
```

These instructions ensure consistency across the GP2040-CE codebase and maintain compatibility with the standard Raspberry Pi Pico development environment.
