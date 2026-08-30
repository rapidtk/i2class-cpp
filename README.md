# i2class-cpp: IBM i Legacy Language Modernization Framework

## Overview

`i2class-cpp` is a C++ class library that bridges legacy IBM i business logic with modern object-oriented development. Originally created to help RPGLE programmers transition to C++, it now serves as the foundation for converting IBM i, mainframe PLI, and COBOL applications to modern C++11, Java, or C# codebases.

The project addresses a fundamental skills gap: helping experienced business logic developers (RPGLE, COBOL, PLI programmers) and newer modern-language developers work together on the same codebase, with converted code that remains semantically similar regardless of target language.

## Philosophy & Design Goals

### Pragmatic Modernization, Not Rewriting
- **Not** a high-performance framework or binary-compatible successor to modern libraries
- **Focused** on converting business logic in a way that maintains developer comfort across languages
- **Goal**: Ease the transition from legacy mainframe/IBM i code to maintainable modern codebases

### Multi-Language Support (Roadmap)
The architecture is designed to support multiple language backends:
- **C++** (current, C++11)
- **Java** (proof-of-concept, successfully commercialized)
- **C#** (planned)

Converted code maintains similar syntax and semantics across all three, while each language gets its natural idioms where appropriate.

### Binary Compatibility Constraints
Core data types (e.g., `Fixed<N>`, `Packed<P,D>`) must maintain **exact byte-for-byte compatibility** with IBM i and mainframe representations:
- `Fixed<10>` occupies exactly 10 bytes
- Packed decimals use BCD encoding matching IBM i native format
- Zoned decimals match EBCDIC/ASCII zoned decimal representations

This constraint is essential for seamless data interchange with legacy files and databases.

## Current Architecture

### Core Components

**Data Types** (`src/`)
- `Fixed<N>`: Fixed-length character strings (replaces RPG CHAR)
- `Varying<N>`: Variable-length strings (replaces RPG VARCHAR)
- `Packed<P,D>`: Packed decimal numbers (binary-coded decimal, IBM i compatible)
- `Zoned<P,D>`: Zoned decimal numbers (RPG numeric format)
- `Indicator`: Boolean flags (RPG-style)
- `AS400`: IBM i data type utilities and conversions

**File Access** (`src/file/`)
- `Rfile`: Base record file abstraction
- `RFile400`: Native IBM i file operations
- `RfileODBC`: SQL-based access via ODBC
- `RfileADO`: Windows/Borland ADO backend
- Device file abstractions for printer/display I/O

**Legacy Components**
- Bison/Lex parsers (archived, legacy RPG parser tooling)
- Old C++ projects (archived to `legacy/`)

## Building

### Prerequisites
- Visual Studio 2026 Community (or compatible MSVC toolset)
- C++11 or later
- Windows SDK 10.0+

### Compile via MSBuild
```bash
msbuild.exe .\i2class-cpp.vcxproj /t:Rebuild /p:Configuration=Compatibility /p:Platform=x64
```

### Compile Smoke Tests Directly
```bash
cl.exe /EHsc /nologo /Isrc /Isrc/file /Iutil /Isrc/ibmi /Iparsers ^
  tests\smoke_core.cpp src\RPGTypes.cpp src\xxcvt.cpp src\as400.cpp
```

## Project Structure

```
i2class-cpp/
  src/                  # Core library headers and implementations
    *.h, *.cpp         # Data types, AS/400 utilities
    file/              # File access classes (record, ODBC, ADO)
    ibmi/              # IBM i-specific APIs
  tests/               # Smoke tests for core types
  parsers/             # Parser tools (historical)
  legacy/              # Archived components (pre-C++11, old projects)
  build/               # Build outputs (binaries, object files)
  i2class-cpp.sln      # Visual Studio solution
  i2class-cpp.vcxproj  # MSVC project file
```

## Known Limitations & Future Work

### Current State
- **No namespace organization** — all symbols at global scope (legacy design)
- **Minimal memory safety** — pointer-based, pre-modern C++ idioms
- **Single platform focus** — Windows/ODBC primary target; IBM i native support incomplete
- **Incomplete error handling** — legacy-style silent failures in some paths

### Planned Improvements
1. **C++11 Modernization** — Move to standard containers, smart pointers, and RAII principles while maintaining binary compatibility constraints
2. **Namespace reorganization** — Hierarchical namespaces (`iz::core::datatype::`, `iz::file::`) to reduce global symbol pollution
3. **Multi-language code generation** — Framework to emit semantically equivalent C++/Java/C# from a unified IR
4. **PL/I & COBOL support** — File access patterns and data types for legacy mainframe languages
5. **Enhanced type safety** — Compile-time checks and runtime validation for file operations

## License

MIT License — see [LICENSE](LICENSE) for details.

## References & Related Projects

- **i2class-java**: Java equivalent of this library (proof-of-concept, legacy)
- **iz2class**: Intermediate representation and transpilation tools for multi-language support
- **IBM i C++ Documentation**: GENCSRC and native C++ file I/O patterns

---

For questions or contributions, see the project repository and documentation in the `legacy/` and `docs/` folders.
