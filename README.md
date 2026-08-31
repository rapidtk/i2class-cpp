# i2class-cpp: IBM i / RPG Legacy Language Modernization Framework

## Overview

`i2class-cpp` is a C++ class library that bridges legacy IBM i business logic with modern object-oriented development. Originally created to help RPGLE programmers transition to C++, it now serves as the foundation for converting IBM i, mainframe PLI, and COBOL applications to modern C++11, Java, or C# codebases.

The project addresses a fundamental skills gap: helping experienced business logic developers (RPGLE, COBOL, PLI programmers) and newer modern-language developers work together on the same codebase, with converted code that remains semantically similar regardless of target language.

**This repository is deliberately scoped to IBM i/RPG.** It's the "modernized but still IBM i-specific" half of a two-repository split:

- **`i2class-cpp`** (this repo) — modernizes the existing IBM i/RPG-specific class library in place, while guaranteeing that any program compiling against it today keeps compiling unchanged.
- **[iZlib-cpp-fork](https://github.com/universal-enterprise/iZlib-cpp-fork.git)** — a fork of this codebase that expands scope beyond IBM i to mainframe COBOL and PL/I, feeding into the broader **[Universal Enterprise](https://github.com/universal-enterprise)** framework.

## Philosophy & Design Goals

### Pragmatic Modernization, Not Rewriting
- **Not** a high-performance framework or binary-compatible successor to modern libraries
- **Focused** on converting business logic in a way that maintains developer comfort across languages
- **Goal**: Ease the transition from legacy mainframe/IBM i code to maintainable modern codebases
- **Strict compatibility rule**: modernization changes internals (safety fixes, portability, build tooling) but never breaks the public interface — existing generated/converted RPG code must keep compiling unmodified

### Multi-Language Support (Roadmap)
The architecture is designed to support multiple language backends:
- **C++** (current, C++11)
- **Java** (proof-of-concept, successfully commercialized)
- **C#** (planned)

Converted code maintains similar syntax and semantics across all three, while each language gets its natural idioms where appropriate.

### Binary Compatibility Constraints
Core data types (`Fixed<N>`, `Zoned<P,D>`, `Indicator`) must maintain **exact byte-for-byte compatibility** with IBM i and mainframe representations:
- `Fixed<10>` occupies exactly 10 bytes
- Zoned decimals match EBCDIC/ASCII zoned decimal representations
- These aren't just documented as a convention — `static_assert`s in `RPGTypes.h` enforce the exact sizes at compile time

This constraint is essential for seamless data interchange with legacy files and databases.

## Current Architecture

### Core Components

**Data Types** (`src/`)
- `Fixed<N>`: Fixed-length character strings (replaces RPG CHAR)
- `Zoned<P,D>`: Zoned decimal numbers (RPG numeric format)
- `DS<N,Occurs>`: Multi-occurrence data structures (RPG OCCUR)
- `Indicator`: Boolean flags (RPG-style)
- Figurative constants: `BLANKS`, `ZEROS`, `LOVAL`, `HIVAL`
- `AS400`: IBM i data type utilities and conversions
- `src/compat/`: portable fallbacks for platform-specific runtime functions (see [i2compat.h](src/i2compat.h)), used when the native compiler doesn't provide them (e.g. non-MSVC builds get a portable `_itoa_s`/`_ecvt_s`)

**File Access** (`src/file/`)
- `Rfile`: Base record file abstraction
- `RrecordPrint`: RPG-style printer output formatting (edit words/edit codes)
- `RFile400`, `RfileODBC`, `RfileADO`: backend-specific implementations (not part of the portable core build — see Known Limitations)

**Legacy Components**
- Bison/Lex parsers (archived, legacy RPG parser tooling)
- Old C++ projects (archived to `legacy/`)

## Building

The project builds with **CMake** — no `.sln`/`.vcxproj` needed. Verified on:
- **Windows / MSVC** (Visual Studio 2026, via the `Visual Studio 18 2026` generator)
- **Linux / GCC** (verified via WSL Ubuntu, GCC 13); should also work on any standard Linux distro
- Should also be consumable by Embarcadero's newer CMake-integrated toolchains, though that hasn't been verified directly

Produces a shared library — `i2class.dll` on Windows, `libi2class.so` on Linux — plus a demo executable and a CTest-driven test suite.

### Configure & build

```bash
# Windows (MSVC)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release   # or Debug

# Linux / WSL (GCC)
cmake -S . -B build
cmake --build build -j$(nproc)
```

`Debug` builds additionally turn on `I2_RUNTIME_ENABLE_BOUNDS_CHECK`, which makes `Fixed<>`/`DS<>` subscript and substring operations throw `CI2ErrSubscript` on out-of-range access instead of silently reading/writing past the buffer. `Release` (and any other config) leaves it off, matching historical behavior.

### Run the tests

```bash
ctest --test-dir build -C Release --output-on-failure   # add -C Debug to also exercise bounds checking
```

- `tests/smoke_core.cpp` — core data type smoke tests (`Fixed`, `Zoned`, `Indicator`, `DS`, `AS400`)
- `tests/bounds_check.cpp` — verifies `I2_RUNTIME_ENABLE_BOUNDS_CHECK` actually throws when enabled
- `tests/negative_zoned_check.cpp` — negative-value zoned-decimal encode/decode regression coverage
- `tests/i2compat_fallback_check.cpp` — exercises `i2compat.h`'s portable (non-MSVC) fallback path directly, even on a machine that only has MSVC available

## Project Structure

```
i2class-cpp/
  CMakeLists.txt        # Cross-platform build (shared library + demo + tests)
  i2class-cpp.cpp        # Demo program
  src/                   # Core library headers and implementations
    i2compat.h          # Compatibility layer: platform detection, portable fallback declarations
    RPGTypes.h/.cpp     # Fixed<N>, Zoned<P,D>, Indicator, DS<N,Occurs>, figurative constants
    xxcvt.h/.cpp        # Zoned-decimal <-> native numeric conversions (QXX* functions)
    xxdtaa.h/.cpp       # Data area emulation
    as400.h/.cpp        # AS/400 connection info
    compat/             # Portable implementations of platform-specific compatibility functions
    file/               # File access classes (record, ODBC, ADO, printer)
  tests/                 # Smoke tests and regression tests for core types
  parsers/               # Parser tools (historical)
  legacy/                # Archived components (pre-C++11, old projects)
```

## Known Limitations & Future Work

### Current State
- **No namespace organization** — all symbols at global scope (legacy design); this is intentional for now, since namespacing would break the "existing code keeps compiling unchanged" guarantee without a compatibility shim
- **Single platform focus for file backends** — `RFile400`/`RfileODBC`/`RfileADO` aren't part of the portable CMake build (ODBC/native-i dependencies); only the stable core (`RPGTypes`, `xxcvt`, `xxdtaa`, `as400`, `compat/stdlib`, `file/rfile`) is built and tested today
- **Runtime bounds checking is opt-in** — only active when `I2_RUNTIME_ENABLE_BOUNDS_CHECK` is defined (CMake enables it for Debug builds); raw arrays (`Fixed<sz>::overlay`, etc.) stay plain C arrays rather than `std::array` since they're public members other code accesses by pointer decay

### Planned Improvements
1. Extend the ODBC/native-i file backends to build under the same portable CMake setup
2. **Namespace reorganization** (breaking change, would need a compatibility facade) — hierarchical namespaces to reduce global symbol pollution, mirroring the approach already taken in `iZlib-cpp-fork`
3. **PL/I & COBOL support** — being pursued in `iZlib-cpp-fork`, not this repository
4. **Enhanced type safety** — compile-time checks and runtime validation for file operations

## License

MIT License — see [LICENSE](LICENSE) for details.

## References & Related Projects

- **[Universal Enterprise](https://github.com/universal-enterprise)** — the broader framework this project feeds into, generalizing beyond IBM i/RPG to mainframe COBOL and PL/I
- **[iZlib-cpp-fork](https://github.com/universal-enterprise/iZlib-cpp-fork.git)** — the fork of this repository where that generalization happens; this repo (`i2class-cpp`) stays the IBM i/RPG-specific, backward-compatible core
- **i2class-java**: Java equivalent of this library (proof-of-concept, legacy)
- **iz2class**: Intermediate representation and transpilation tools for multi-language support
- **IBM i C++ Documentation**: GENCSRC and native C++ file I/O patterns

---

For questions or contributions, see the project repository and documentation in the `legacy/` and `docs/` folders.

