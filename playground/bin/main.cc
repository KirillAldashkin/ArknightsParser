#include <iostream>
#include <chrono>
#include <filesystem>
#include <memory>
#include <iomanip>
#include <format>

#include <platform/mapped_file.h>
#include <unity/file/bundle.h>
#include <unity/file/asset.h>

void black_box(auto& v) { asm volatile("" : : "r,m"(v) : "memory"); }

bool do_one(const std::filesystem::path& path) {
  auto map = platform::MappedFile::Open(path, common::RwxRights::Read);
  if (!map) {
    std::cerr << std::format("Couldn't open \"{}\": {}", path.string(), map.error().message()) << std::endl;
    return false;
  }

  auto bundle = unity::file::Bundle::Read<platform::MappedFile&>(*map);
  if (!bundle) {
    std::cerr << std::format("Couldn't read bundle \"{}\": {}", path.string(), bundle.error()) << std::endl;
    return false;
  }

  auto asset = &bundle->first_file();
  for (std::uint32_t i = 0; i < bundle->file_count; ++i) {
    auto mem = std::make_unique<char[]>(asset->size);
    auto unpack_err = bundle->UnpackData(asset->offset, {mem.get(), asset->size});
    if (unpack_err) {
      std::cerr << std::format("Couldn't unpack asset \"{}\"[{}]: {}", path.string(), i, bundle.error()) << std::endl;
      return false;
    }

    std::span<char> mem_span(mem.get(), asset->size);
    if (unity::file::Asset::Detect(mem_span)) {
      auto asset_data = unity::file::Asset::Read(mem_span);
      if (!asset_data) {
        std::cerr << std::format("Couldn't read asset \"{}\"[{}]: {}", path.string(), i, asset_data.error()) << std::endl;
        return false;
      }
      black_box(*asset_data);
    }

    asset = &asset->next();
  }
  return true;
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