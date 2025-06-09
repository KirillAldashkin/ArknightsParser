#include <iostream>
#include <chrono>
#include <filesystem>
#include <memory>
#include <iomanip>

#include <platform/mapped_file.h>
#include <unity/file/bundle.h>

void do_one(const std::filesystem::path& path) {
  auto map = platform::MappedFile::Open(path, common::RwxRights::Read);
  if (!map) {
    std::cerr << "Couldn't open \"" << path << "\": " << map.error().message() << std::endl;
    return;
  }

  auto bundle = unity::file::Bundle<platform::MappedFile&>::Read(*map);
  if (!bundle) {
    std::cerr << "Couldn't read bundle \"" << path << "\": " << bundle.error() << std::endl;
    return;
  }

  auto asset = &bundle->first_file();
  for (std::uint32_t i = 0; i < bundle->file_count; ++i) {
    auto mem = std::make_unique<char[]>(asset->size);
    auto result = bundle->UnpackData(asset->offset, {mem.get(), asset->size});
    asset = &asset->next();
  }
}

int main() {
  std::string path;
  std::cout << "Path to resources: ";
  std::getline(std::cin, path);

  auto begin = std::chrono::high_resolution_clock::now();
  std::size_t total = 0;
  double size = 0;
  for (auto& file : std::filesystem::recursive_directory_iterator{path}) {
#pragma region Path validation
    if (file.is_directory()) continue;
    auto name = file.path().c_str();
    while (*name++);
    if (name[-4] != L'.') continue;
    if (name[-3] != L'a') continue;
    if (name[-2] != L'b') continue;
#pragma endregion

    ++total;
    size += file.file_size();
    do_one(file.path());
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Processed " << total << " files (" << std::fixed 
            << std::setprecision(1) << size / 1024 / 1024 / 1024 << "GB) in " 
            << std::chrono::duration_cast<std::chrono::seconds>(end - begin);
  return 0;
}