# Arknights asset parser

## Info
Datamining tool I decided to wrote because all other tools are **way too slow**.

## Usage
Unusable yet.

## Progress
- [X] Parsing bundles
- [X] Extracting assets from bundles
  - [X] Handling new CN encoding
- [ ] Parsing assets
- [ ] Extracting content from assets
- [ ] Nice viewer with version difference

## Building
### Requirements
* [CMake](https://cmake.org/) `>=3.20.0`
* Any C++23 compiler, e.g. [CLang](https://github.com/llvm/llvm-project) `>=20.0.0`
### Process
```bash
cmake -B %build_dir%
cmake --build %build_dir%
```