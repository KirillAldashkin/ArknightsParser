#pragma once

#include <cstddef>
#include <expected>
#include <system_error>
#include <filesystem>

#include <common/rights.h>

namespace platform {

/// File mapped into process' address space.
class MappedFile {
 public:
  /// Tries to open given file.
  static std::expected<MappedFile, std::error_code> Open(const std::filesystem::path& path, common::RwxRights mode);

  MappedFile(const MappedFile&) = delete;
  MappedFile& operator=(const MappedFile&) = delete;
  MappedFile(MappedFile&& other);
  MappedFile& operator=(MappedFile&& other);
  ~MappedFile();

  /// Size of mapped region (equal to file size).
  std::size_t size() const { return size_; }
  /// Pointer to file content.
  const void* data() const { return data_; }
  /// Pointer to file content.
  void* data() { return data_; }

 private:
  MappedFile() = default;

  std::size_t size_;
  void* data_;
// actual type here is HANDLE but using it requires including
// platform-dependent headers to end user, which is not ideal
#ifdef OS_WINDOWS
  void* file_;
  void* mapping_;
#endif
};  // class MappedFile

}  // namespace platform