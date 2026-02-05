
This directory is intended for the project specific (private) libraries.
PlatformIO will compile them to static libraries and link them to the
executable file.

The source code of each library should be placed in a separate directory, for
example: "lib/private_lib/[here are source files]".

Example layout showing `Foo` and `Bar` libraries:

|-- lib
|   |-- Bar
|   |   |-- docs
|   |   |-- examples
|   |   |-- src
|   |      |- Bar.cpp
|   |      |- Bar.h
|   |-- Foo
|   |   |- Foo.cpp
|   |   |- Foo.h
|   |- readme.txt    --> THIS FILE
|- platformio.ini
|- src
    |- main.cpp

In `src/main.cpp` you can include your libraries like this:

#include <Foo.h>
#include <Bar.h>

PlatformIO will find your libraries automatically, configure include paths
and build them.

ESP32 (PlatformIO) — notes and tips
----------------------------------

If you are targeting an ESP32 board (Arduino framework under the `espressif32`
platform), add or adapt your project `platformio.ini` to include an ESP32
environment. Example minimal configuration:

[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
# Library Dependency Finder mode (optional): "deep" or "chain+" are common
# to make sure PlatformIO finds nested libs in `lib/` and `lib/*/src`.
lib_ldf_mode = deep

Notes when using ESP32:
- Source files should be C++ (use .cpp/.h) or Arduino-style sketches (.ino)
   — ESP32 Arduino core expects C++ linkage and typically needs `#include
   <Arduino.h>` at the top of C-style files.
- GPIO numbering: ESP32 uses GPIO numbers (for example, "GPIO 2" is pin 2).
   Avoid using flash-related pins (GPIO 6..11) and be cautious with
   boot-strapping pins (GPIO 0, 2, 15, 12) if they affect boot mode.
- Some pins are input-only (e.g., GPIO34–GPIO39). Check the ESP32 datasheet
   when choosing pins for outputs, PWM, or ADC.
- If your library toggles timers, PWM, or uses hardware peripherals, verify
   compatibility with ESP32 APIs (the ESP32 Arduino core has different timer
   and PWM APIs than AVR/Uno).
- If you need to enable partitioning, PSRAM, or other board-specific features,
   configure the board in PlatformIO or add appropriate `build_flags`.

PlatformIO library layout tips for ESP32 projects
-------------------------------------------------
- Put library sources in `lib/YourLib/src/` as `.cpp` / `.h` files. PlatformIO
   will add `lib/YourLib/src` to the include path so `#include <YourLib.h>` works.
- Place Arduino-style examples under `lib/YourLib/examples/ExampleName/`
   (e.g. `.ino`) so they are easy to find and test.
- If your library depends on ESP32-specific features, document that in the
   library README and consider using preprocessor guards to allow compilation
   on multiple platforms:

   #if defined(ARDUINO_ARCH_ESP32)
   // ESP32-specific code
   #endif

More information
----------------
See additional options for PlatformIO Library Dependency Finder (`lib_*`):
http://docs.platformio.org/en/latest/projectconf.html#lib-install

And ESP32 PlatformIO platform docs:
https://docs.platformio.org/en/latest/platforms/espressif32.html

