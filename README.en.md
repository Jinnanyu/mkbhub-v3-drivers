# MKBHub V3 SoC Peripheral Drivers

[中文说明](README.md) | English

An archive of SPI, PIT timer, and WDT watchdog driver development and board-level test materials.

## Background and contribution

- **Author:** Nanyu Jin (金南宇)
- **Internship:** Embedded development, 北京宁矩科技有限公司
- **Period:** June–July 2025

I independently developed the SPI, PIT, and WDT driver interfaces and register-level implementations,
prepared peripheral test programs, and completed board-level functional testing during the internship.
The work covered register configuration, interrupt handling, peripheral bring-up, and test documentation.

This repository preserves the original source and test materials for reference and portfolio use.
It requires the matching SoC SDK, board support, and target toolchain; it is not a standalone firmware project.

## Driver functionality

| Peripheral | Functionality represented in the source |
| --- | --- |
| SPI | Master/slave selection, CPOL/CPHA, data format and clock-divider configuration; FIFO-based transfers; status and timeout handling; interrupt configuration and handler registration. |
| PIT | 32-bit, dual 16-bit, and four 8-bit timer modes; clock selection, reload values, channel enable, interrupt status handling, and user callbacks. |
| WDT | Write-protection unlocking, clock and timeout configuration, reset/interrupt enable settings, watchdog restart (feeding), and disable control. |

SPI DMA-enable fields are exposed as register configuration options. This archive does not contain a complete DMA transfer implementation.
The configuration structures also describe more SPI modes than the transfer routines fully handle; their presence does not establish complete mode coverage.

## Repository contents

| Path | Purpose |
| --- | --- |
| [spi2_driver.h](spi2_driver.h) | SPI configuration types, enumerations, and API declarations. |
| [spi2_driver_user.h](spi2_driver_user.h) | SPI register-level implementation and interrupt interfaces. |
| [pit_driver.h](pit_driver.h) | PIT configuration types and API declarations. |
| [pit_driver_user.h](pit_driver_user.h) | PIT configuration, reload, and interrupt implementation. |
| [wdt_driver.h](wdt_driver.h) | WDT configuration types and API declarations. |
| [wdt_driver_user.h](wdt_driver_user.h) | WDT configuration, restart, and disable implementation. |
| [main.c](main.c) | Historical SPI development entry point, currently selecting `SPIIN2`. |
| [test_spi_main.txt](test_spi_main.txt) | Separate SPI test entry point, currently selecting `SPIIN1`. |
| [test_pit_main.txt](test_pit_main.txt) | PIT interrupt example using GPIO toggles to observe timer events. |
| [test_wdt_main.txt](test_wdt_main.txt) | WDT configuration and reset/feeding development example. |
| [测试文档.pdf](测试文档.pdf) | Original board-test report, in Chinese. |
| [DSLogic/](DSLogic/) | Original logic-analyzer capture files (`.dsl`). |

## Test records

The [test report](测试文档.pdf) and [raw DSLogic captures](DSLogic/) document the internship's hardware experiments.
The capture collection includes SPI communication, PIT timer, and WDT-related recordings.
Use compatible DSLogic capture-viewing software to inspect the `.dsl` files.

These are historical test materials. The code was not rebuilt, flashed, or retested on hardware during this repository's publication preparation.
Capture filenames retain their original experiment labels and should not be read as a new, comprehensive validation matrix.

## SDK and hardware dependencies

The following directly referenced SDK headers are **not included**:

```text
platform.h        interrupt.h        ndslib.h
uwb_drivers.h     mkb03_ct_func.h
atcpit100.h       atcwdt200.h
```

The implementation also relies on SDK-provided peripheral register structures and bit masks,
memory-mapped device definitions, interrupt numbers and function pointers, GPIO support, and delay functions.
Target startup code, a linker script, build configuration, and programming/debug configuration are not supplied.
Consequently, compiling these files alone with a desktop C compiler will not produce usable firmware.

## Integrating an example

1. Start with a working project from the matching SoC SDK and board support package.
2. Review the archived code and the limitations below before using it on hardware.
3. Select **one** entry point: `main.c`, or one of the `test_*_main.txt` examples copied into a C source file.
   Each example defines its own `main()`; exclude the other entry points from the target build.
4. Include the selected peripheral's declarations and implementation using the original example as a reference.
   The `*_driver_user.h` files contain function definitions, so include each implementation in only one translation unit.
5. Verify the board-specific clock setup, pin multiplexing, peripheral instance, interrupt routing, and external connections.
6. Build and program the target with its SDK workflow, then verify the selected function using suitable hardware measurements.

The clock-related macros in the examples do not by themselves initialize the system clock.
No universal build command is provided because the required SDK project and toolchain configuration are absent.

## Known limitations of the archived code

- **SPI development entry point:** `main.c` allocates buffers without multiplying the element count by `sizeof(uint32_t)`.
  Its current 32-bit data configuration also reaches a `1 << 32` expression. Correct the allocation and shift handling before execution.
- **SPI interrupt routing:** the interrupt disable and clear helpers use the fixed `INT_NO_SPI2` value even when another SPI instance is selected.
  Check and adapt instance-specific interrupt handling before reuse.
- **Header integration:** PIT and WDT both declare `err_t` and `err_success`; including both interfaces together causes naming conflicts.
  Their names and the implementation-header organization need adjustment for a combined driver library.
- **WDT example state:** registration of the PIT handler used for periodic watchdog feeding is commented out in `test_wdt_main.txt`.
  The archived file therefore should not be assumed to run periodic interrupt-driven feeding as written.

These notes describe issues observed in the preserved snapshot, not a complete correctness audit.
The original driver and example source files have been retained rather than silently rewritten.
