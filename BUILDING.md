# Building BaseElements Plugin

The BaseElements plugin consists of three parts:
1. **FileMaker Plugin SDK** from Claris
2. **Pre-built external libraries** from [BaseElements-Plugin-Libraries](https://github.com/GoyaPtyLtd/BaseElements-Plugin-Libraries) (ImageMagick, Boost, Poco, OpenSSL, curl, libxml2, etc.)
3. **BaseElements plugin source code** (this repository)

## Prerequisites

### Git

Standard Git is sufficient for cloning this repository:

```bash
git clone https://github.com/GoyaPtyLtd/BaseElements-Plugin.git
cd BaseElements-Plugin
```

**Note:** This repository  uses Git LFS for large binary files, but most linxu and mac libraries have been moved to the external [BaseElements-Plugin-Libraries](https://github.com/GoyaPtyLtd/BaseElements-Plugin-Libraries) repository. You may still need Git LFS for some remaining files in older branches.

## Getting External Libraries

The external libraries are pre-compiled and provided as release packages:

1. Go to the [BaseElements-Plugin-Libraries releases](https://github.com/GoyaPtyLtd/BaseElements-Plugin-Libraries/releases)
2. Download the appropriate package for your platform:
   - **macOS:** `external-macos-arm64_x86_64.tar.gz`
   - **Linux:** `external-linux-*.tar.gz` (choose your Ubuntu version and architecture)
3. Extract into the `external/` directory:

```bash
cd BaseElements-Plugin
tar -xzf external-macos-arm64_x86_64.tar.gz -C external/
# or
tar -xzf external-linux-*.tar.gz -C external/
```

The directory structure should be:
```
BaseElements-Plugin/
├── external/
│   ├── macos-arm64_x86_64/    (for macOS)
│   │   ├── include/
│   │   ├── lib/
│   │   ├── src/
│   │   └── PlugInSDK/
│   └── ubuntu24.04-aarch64/             (for Linux)
│       ├── include/
│       ├── lib/
│       ├── src/
│       └── PlugInSDK/
```

## Building on macOS

### System Requirements

- macOS 10.14 or later
- Xcode Command Line Tools
- CMake 3.16.3 or later

### Setup

```bash
# Install Command Line Tools
xcode-select --install

# Install CMake (via Homebrew)
brew install cmake
```

### Build Steps

```bash
# 1. Clone the repository
git clone https://github.com/GoyaPtyLtd/BaseElements-Plugin.git
cd BaseElements-Plugin

# 2. Download and extract external libraries (see above)

# 3. Create build directory
mkdir build
cd build

# 4. Configure
cmake -DCMAKE_BUILD_TYPE=Release ..

# 5. Build
make -j$(sysctl -n hw.ncpu)
```

The plugin will be built as `BaseElements.fmplugin` (universal binary: arm64 + x86_64).

### Build Options

```bash
# Debug build (with debug symbols)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Pro version
cmake -DCMAKE_BUILD_TYPE=Release -DPRO=1 ..
```

## Building on Linux (Ubuntu)

### System Requirements

- Ubuntu 22.04 or 24.04 (x86_64 or ARM64)
- CMake 3.16.3 or later
- LLVM/Clang compiler

### Setup

Follow the setup instructions from [BaseElements-Plugin-Libraries](https://github.com/GoyaPtyLtd/BaseElements-Plugin-Libraries) to install required build tools.

### Build Steps

```bash
# 1. Clone the repository
git clone https://github.com/GoyaPtyLtd/BaseElements-Plugin.git
cd BaseElements-Plugin

# 2. Download and extract external libraries (see above)

# 3. Copy Linux CMakeLists.txt
cp CMakeLists-linux.txt CMakeLists.txt

# 4. Create build directory
mkdir build
cd build

# 5. Configure
cmake -DCMAKE_BUILD_TYPE=Release ..

# 6. Build
make -j$(($(nproc) + 1))
```

The plugin will be built as `BaseElements.fmx`.

### Installation

For FileMaker Server installations:

```bash
# Install to FileMaker Server Extensions directory
sudo make install

# Set correct ownership
sudo chown fmserver:fmsadmin "/opt/FileMaker/FileMaker Server/Database Server/Extensions/BaseElements.fmx"
```

Or specify a custom installation path:
```bash
cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..
```

### Build Options

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..

# Pro version
cmake -DCMAKE_BUILD_TYPE=Release -DPRO=1 ..
```

## Building on Windows

### System Requirements

TBC

### Build Notes

TBC

### Build Steps

TBC

## Development

### Contributing

All code changes should be submitted to the **Development** branch, which should always compile and pass tests. Releases are made from Development to the main branch.

To contribute:
1. Fork this repository
2. Create a branch for your changes
3. Make and test your changes
4. Submit a pull request to the Development branch

### Code Signing

For macOS and Windows distribution:
- **macOS:** Obtain a Developer certificate from Apple
- **Windows:** Obtain a code signing certificate from a certificate provider

You don't need certificates for local development and testing - FileMaker Pro will prompt you to authorize the plugin.

## More Information

- **Plugin Libraries Repository:** https://github.com/GoyaPtyLtd/BaseElements-Plugin-Libraries
- **Work in Progress:** See [WorkInProgress.md](WorkInProgress.md) in the Development branch
- **FileMaker SDK:** Download from [Claris Downloads](https://www.claris.com/resources/downloads/)
