# i2class-cpp: IBM i / RPG Legacy Language Modernization Framework

## Overview

`i2class-cpp` is a C++ class library that bridges legacy IBM i business logic with modern object-oriented development. Originally created to help RPGLE programmers transition to C++, it now serves as the foundation for converting IBM i, mainframe PLI, and COBOL applications to modern C++11, Java, or C# codebases.

The project addresses a fundamental skills gap: helping experienced business logic developers (RPGLE, COBOL, PLI programmers) and newer modern-language developers work together on the same codebase, with converted code that remains semantically similar regardless of target language.

**This repository is deliberately scoped to IBM i/RPG.** It's the "modernized but still IBM i-specific" half of a two-repository split:

- **`i2class-cpp`** (this repo) — modernizes the existing IBM i/RPG-specific class library in place, keeping the public interface stable apart from a short list of deliberate, documented [Breaking Changes](#breaking-changes).
- **[iZlib-cpp-fork](https://github.com/universal-enterprise/iZlib-cpp-fork.git)** — a fork of this codebase that expands scope beyond IBM i to mainframe COBOL and PL/I, feeding into the broader **[Universal Enterprise](https://github.com/universal-enterprise)** framework.

## Philosophy & Design Goals

### Pragmatic Modernization, Not Rewriting
- **Not** a high-performance framework or binary-compatible successor to modern libraries
- **Focused** on converting business logic in a way that maintains developer comfort across languages
- **Goal**: Ease the transition from legacy mainframe/IBM i code to maintainable modern codebases
- **Compatibility rule**: modernization changes internals (safety fixes, portability, build tooling) and leaves the public interface alone by default. That rule has been *deliberately* broken in a small number of places where the original interface was unsafe or silently wrong — every one of them is listed under [Breaking Changes](#breaking-changes) with the reason and the fix.

## Breaking Changes

These are intentional. Each replaces something that compiled but could silently do the
wrong thing, and each fails loudly rather than quietly, so the compiler points at every
site that needs attention.

### `packed(n,p)` is now `Packed<n,p>`, not `_DecimalT<n,p>`

On IBM i, `packed(n,p)` previously expanded straight to bcd.h's `_DecimalT<n,p>`. It now
expands to `Packed<n,p>`, a thin wrapper whose backing store *is* a `_DecimalT` on IBM i
(and a `Zoned<>` elsewhere). Arithmetic still runs through the native packed type, so it
stays exact decimal.

Why: `packed()` used to mean a *different type with different conversion rules* depending
on the platform, and `_DecimalT` offers none of the RPG opcodes (`move()`, `movel()`,
`toInt()`, ...). One wrapper gives one interface and one set of rules everywhere.

The wrapper is layout-compatible — single member, no virtuals, `static_assert`-enforced —
so fields overlaid on record buffers are unaffected. What can change:

- Code that named `_DecimalT<n,p>` explicitly, or passed `packed()` values to bcd.h APIs, may need `.toPacked()` (returns the native type) or the implicit `_ConvertDecimal` conversion.
- Expressions mixing `packed()` with a raw `_DecimalT` can become ambiguous. Fix by adding an exact-match overload, or convert explicitly.

### Comparing a decimal to a floating-point literal no longer compiles

```cpp
if (znd == 32.1)          // was: allowed        now: deleted overload
if ((double)znd == 32.1)  // compare as floating point
if (znd == __D("32.1"))   // compare exactly, digit for digit
```

Why: most decimal values have no exact binary representation, so `znd == 32.1` asks whether
a decimal equals an *approximation* of 32.1 — rarely the intent, and it fails in ways that
are very hard to see.

Unaffected: `double d = znd;`, `znd = 3.14;` (stores the written digits), `int i = znd;`
(truncates like the built-in types), and all integer comparisons (`znd == 42`, `znd > 0`).

### Record I/O returns `bool`/`void` instead of status characters

The `Rfile*` hierarchy previously returned a `char` of `'0'` or `'1'`.

```cpp
if (file.read() == '0')   // was
while (file.read())       // now -- true means a record was read

file.open(READ_ONLY);     // now returns void, throws CI2ErrFile on failure
```

`error`, `found` and `eof` are now `bool` rather than `char`.

Why: `'0'` and `'1'` are *both* truthy as a `char`, so the natural-looking
`while (file.read())` was an infinite loop. The two conventions in the codebase also
disagreed about which character meant success. `true` now uniformly means "succeeded /
record found", matching the Java implementation this library mirrors.

### `char *` parameters and members are now `const char *`

File names, record format names, connection details and `AS400`'s constructor arguments
take `const char *`.

Why: passing a string literal to a `char *` parameter has been ill-formed since C++11, so
this was already a compile error on conforming compilers.

### `MAX_DECIMAL_DIGITS` is 63, not 31

It is derived from bcd.h's `DFT_DEC_DIG` where available (63 on IBM i 7.2+, 31 on 7.1 and
earlier) and 63 otherwise. `HIVAL`/`LOVAL` for numeric fields are correspondingly larger.
Scratch buffers were previously sized for 31 digits, which was a latent overrun on 7.2+
where an intermediate result can legitimately be 63.


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
- `tests/rfile_print_check.cpp` — `RrecordPrint` edit-word/edit-code formatting regressions
- `tests/i2compat_fallback_check.cpp` — exercises `i2compat.h`'s portable (non-MSVC) fallback path directly, even on a machine that only has MSVC available
- `tests/rfileodbc_custmast_check.cpp` — end-to-end `RfileODBC` read against a real data source (see below)

### The ODBC test fixture

`tests/CUSTMAST.csv` is the single source of truth for the ODBC read tests, but the two
platforms reach it differently:

- **Windows** queries the CSV in place through the built-in *Microsoft Access Text Driver*, so there is no ODBC setup to do after cloning.
- **Linux** has no CSV/text driver in unixODBC, so CMake generates an equivalent SQLite database from the same CSV at build time and queries that (`libsqliteodbc` + the `sqlite3` CLI). The generated `.db` is a build artifact and is gitignored.

The connection string is selected by `#ifdef` in [i2class-cpp.cpp](i2class-cpp.cpp) and
[tests/rfileodbc_custmast_check.cpp](tests/rfileodbc_custmast_check.cpp). Where ODBC is
present but no usable driver is, the test still *builds* (keeping `RfileODBC` under
compile coverage) and is simply not registered to run.

## Cross-Compiler Portability Notes

The library is built with three quite different compilers — MSVC, GCC, and IBM's ILE C++
on IBM i. The notes below record behaviour we ran into and adapted to, so contributors
don't have to rediscover it. None of it is a defect in any of these compilers; it is
mostly the ordinary consequence of a large C++11 feature surface landing in different
front ends at different times.

### ILE C++ (IBM i)

Compiled with `LANGLVL(*EXTENDED0X)`, which provides a substantial and very usable C++11
subset. Two constructs the library deliberately avoids, and one it relies on:

- **Default member initializers are not available.** Write `int x;` on the member and
  initialize it in the constructor's mem-initializer-list rather than `int x{};` on the
  declaration. (List members in declaration order — they are initialized in that order
  regardless of how the list is written, and most compilers will warn if the two differ.)
- **A `constexpr` function call is not accepted inside a constant expression** in the
  cases we tried — e.g. a `constexpr` string-length helper applied to a
  `constexpr const char *` inside a `static_assert` reports
  `CZP0016 The expression must be an integral non-volatile constant expression`.
  `static_assert` itself works fine; the workaround is to declare the data as an array
  (`static const char NAME[] = "...";`) and assert on `sizeof(NAME) - 1`, which is an
  integral constant expression in any C++ dialect. `RPGTypes.h` uses this for the
  `MAX_DECIMAL_STR`/`MIN_DECIMAL_STR` length checks.
- **`= delete` is fully supported and enforced**, for plain functions *and* function
  templates — a call to a deleted overload is rejected with
  `CZP1307 The declaration of "..." is deleted and cannot be used.` This is load-bearing:
  it is how the decimal types can accept a conversion in one direction while making the
  lossy direction a clear compile-time error rather than a silent surprise.

### MSVC vs GCC

- GCC is the stricter of the two on overload ambiguity, and is worth building against even
  if Windows is your primary target. An ambiguous `operator+` between two `Zoned<>` values
  compiled silently under MSVC while GCC correctly rejected it — the fix (giving `Zoned`
  its own operators rather than letting it borrow them from an implicit conversion) was
  the right one on both compilers.
- CMake's makefile generator escapes spaces in custom-command arguments but not `(`, `)`
  or `;`, so SQL or shell metacharacters passed inline can reach `/bin/sh` unquoted. The
  SQLite fixture generation keeps its SQL in a generated script file for this reason.

### Working with packed decimal (`bcd.h`)

- `bcd.h` provides `operator<<` for `_DecimalT`, but only under `IOSTREAMH` (the
  pre-standard `<iostream.h>`) or when `__POSIX_LOCALE__` is defined. Without one of those
  in scope, `std::cout << packedValue` falls back to an implicit conversion to a built-in
  type. `Zoned<>` supplies its own `operator<<` so it prints every digit on all platforms.
- Relatedly: `std::cout`'s default precision is **6 significant digits**, so an exact
  `60145.76` displays as `60145.8`. That looks like decimal precision loss but isn't —
  ILE C's `printf` supports `%D(n,p)` for packed decimal when you want the exact digits.
- `DFT_DEC_DIG` is 63 on IBM i 7.2 and later, and 31 before that. `MAX_DECIMAL_DIGITS` is
  derived from it rather than hardcoded, so scratch buffers and the figurative constants
  stay correctly sized on any release.
- The conversion operators on `_ConvertDecimal`, and `_DecimalT::operator _ConvertDecimal()`,
  are non-`const`, so a `const _ConvertDecimal &` cannot be converted to a numeric type.
  Take these by value or non-const reference.

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
- **Backend coverage varies by platform** — `RfileODBC` is built and tested on Windows and Linux; `RFile400` compiles natively on IBM i (where `<recio.h>` exists) and is exercised by the demo program there, but cannot be built off-platform; `RfileADO` is Borland-era and is not part of any current build
- **Runtime bounds checking is opt-in** — only active when `I2_RUNTIME_ENABLE_BOUNDS_CHECK` is defined (CMake enables it for Debug builds); raw arrays (`Fixed<sz>::overlay`, etc.) stay plain C arrays rather than `std::array` since they're public members other code accesses by pointer decay

### Planned Improvements
1. **`Packed<>` wrapper** — a thin, layout-compatible wrapper over the platform's native packed-decimal type (`_DecimalT` on IBM i), so `packed(n,p)` means the same thing with the same conversion rules on every platform, and provides the same `assign()`/`move()`/`movel()` interface as `Zoned<>`
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

