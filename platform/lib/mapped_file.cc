#include "platform/mapped_file.h"

#include <utility>

#ifdef OS_WINDOWS
# include <Windows.h>
# define OS_ERROR static_cast<int>(GetLastError())
#elifdef OS_UNIX
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <unistd.h>
# include <errno.h>
#include "mapped_file.h"
# define OS_ERROR errno
#endif

namespace platform {

static auto OsError() {
  return std::unexpected(std::error_code { 
    OS_ERROR, 
    std::system_category()
  });
}

std::expected<MappedFile, std::error_code> MappedFile::Open(const std::filesystem::path& path, common::RwxRights mode) {
#ifdef OS_WINDOWS
  DWORD file_mode = 0;
  if (mode << common::RwxRights::Read) file_mode |= GENERIC_READ;
  if (mode << common::RwxRights::Write) file_mode |= GENERIC_WRITE;

  auto file = CreateFileW(path.c_str(), file_mode, FILE_SHARE_READ, NULL, 
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (file == INVALID_HANDLE_VALUE) return OsError();

  LARGE_INTEGER size;
  if (!GetFileSizeEx(file, &size)) {
    auto err = OsError();
    CloseHandle(file);
    return err;
  }

  DWORD prot_mode = 0;
  switch (mode) {
   case common::RwxRights::Read:
    prot_mode = PAGE_READONLY;
    break;
   case common::RwxRights::Write:
   case common::RwxRights::Read | common::RwxRights::Write:
    prot_mode = PAGE_READWRITE;
    break;
   case common::RwxRights::Execute:
   case common::RwxRights::Read | common::RwxRights::Execute:
    prot_mode = PAGE_EXECUTE_READ;
    break;
   case common::RwxRights::Write | common::RwxRights::Execute:
   case common::RwxRights::Read | common::RwxRights::Write | common::RwxRights::Execute:
    prot_mode = PAGE_EXECUTE_READWRITE;
    break;
  }

  auto map = CreateFileMappingA(file, NULL, prot_mode, 0, 0, NULL);
  if (map == NULL) {
    auto err = OsError();
    CloseHandle(file);
    return err;
  }

  DWORD map_mode = 0;
  if (mode << common::RwxRights::Read) map_mode |= FILE_MAP_READ;
  if (mode << common::RwxRights::Write) map_mode |= FILE_MAP_WRITE;
  if (mode << common::RwxRights::Execute) map_mode |= FILE_MAP_EXECUTE;

  auto ptr = MapViewOfFile(map, map_mode, 0, 0, 0);
  if (ptr == NULL) {
    auto err = OsError();
    CloseHandle(map);
    CloseHandle(file);
    return err;
  }

  MappedFile ret;
  ret.file_ = file;
  ret.mapping_ = map;
  ret.size_ = size.QuadPart;
  ret.data_ = ptr;
#elifdef OS_UNIX
  // TODO account for access mode on UNIX
  int fd = open(path, O_RDWR | O_LARGEFILE);
  if (fd == -1) return OsError();

  struct stat64 stats;
  if (fstat64(fd, &stats) != 0) {
    auto err = OsError();
    close(fd);
    return OsError();
  }

  auto data = mmap64(nullptr, stats.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  auto err = OsError();
  close(fd);
  if (data == reinterpret_cast<void*>(-1)) return OsError();

  MappedFile ret;
  ret.size_ = stats.st_size;
  ret.data_ = data;
#endif
  return std::move(ret);
}

MappedFile::MappedFile(MappedFile&& other) 
  : size_{std::exchange(other.size_, 0)}
  , data_{std::exchange(other.data_, nullptr)}
#ifdef OS_WINDOWS
  , file_{std::exchange(other.file_, nullptr)}
  , mapping_{std::exchange(other.mapping_, nullptr)}
#endif
  {}

MappedFile& MappedFile::operator=(MappedFile&& other) {
  this->~MappedFile();
  new (this) MappedFile(std::forward<MappedFile>(other));
  return *this;
}

MappedFile::~MappedFile() {
  if (data_ == nullptr) return;

#ifdef OS_WINDOWS
  UnmapViewOfFile(data_);
  CloseHandle(mapping_);
  CloseHandle(file_);
#elifdef OS_UNIX
  munmap(data_, size_);
#endif
}

}  // namespace platform