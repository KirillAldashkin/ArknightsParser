#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <memory>
#include <variant>
#include <algorithm>
#include <span>

#include <common/data_reader.h>
#include <common/any.h>
#include <common/guid.h>
#include <platform/endian.h>

#include <unity/flags.h>
#include <unity/misc.h>
#include <unity/type.h>

namespace unity {

namespace file {

class Asset {
 public:
  struct Header {
    std::uint64_t meta_size;
    std::uint64_t file_size;
    std::uint32_t version;
    std::uint32_t data_offset;
    platform::Endian endian;

    template<common::DataView Source>
    void Read(common::DataReader<Source>& from);
  };  // struct Header
  
  struct Object {
    std::int64_t path_id;
    std::uint64_t offset;
    std::uint32_t size;
    std::uint32_t type_id;
    ClassID class_id;
    std::uint16_t destroyed;
    std::int16_t script_type;
    bool stripped;

    template<common::DataView Source>
    void Read(common::DataReader<Source>& from, const Asset& asset);
  };  // struct Object

  struct ScriptType {
    std::uint32_t file_index;
    std::uint64_t index_in_file;
  };  // struct ScriptType

  struct FileIdentifier {
    common::Guid guid;
    AssetType type;
    const char* path;
  };  // struct FileIdentifier

  template<common::DataView Source>
  static std::expected<Asset, std::string> Read(Source&& from);

  template<common::DataView Source>
  static bool Detect(Source& from);

  Header header;
  const char* version;
  TargetPlatform platform;
  bool enable_typetree;
  std::uint32_t type_count;
  std::unique_ptr<Type[]> types;
  std::uint32_t big_id_enabled;
  std::uint32_t object_count;
  std::unique_ptr<Object[]> objects;
  std::uint32_t script_count;
  std::unique_ptr<ScriptType[]> scripts;
  std::uint32_t externals_count;
  std::unique_ptr<FileIdentifier[]> externals;
  std::uint32_t reftype_count;
  std::unique_ptr<Type[]> reftypes;
  std::uint8_t* data;

  std::span<const char> GetObject(std::uint32_t index) const;

 private:
  template<common::DataView Source>
  Asset(Source&& from);

  common::Any source_;
};  // class Asset

template <common::DataView Source>
void unity::file::Asset::Header::Read(common::DataReader<Source>& from) {
  meta_size = from.template Read<platform::u32be>();
  file_size = from.template Read<platform::u32be>();
  version = from.template Read<platform::u32be>();
  data_offset = from.template Read<platform::u32be>();
  if (version >= 9) {
    endian = from.template Read<std::uint8_t>() ? platform::Endian::B : platform::Endian::L;
    from.position += 3;  // reserved space
  } else {
    from.position = file_size - meta_size;
    endian = from.template Read<std::uint8_t>() ? platform::Endian::B : platform::Endian::L;
  }
  if (version >= 22) {
    meta_size = from.template Read<platform::u64be>();
    file_size = from.template Read<platform::u64be>();
    data_offset = from.template Read<platform::u32be>();
    from.position += 8;
  }
}

template<common::DataView Source>
void Asset::Object::Read(common::DataReader<Source>& from, const Asset& asset) {
  if (asset.big_id_enabled != 0) {
    path_id = from.template Read<platform::i64re>().get(asset.header.endian);
  } else if(asset.header.version < 14) {
    path_id = from.template Read<platform::i32re>().get(asset.header.endian);
  } else {
    from.AlignTo(4);
    path_id = from.template Read<platform::i64re>().get(asset.header.endian);
  }

  if (asset.header.version >= 22) {
    offset = from.template Read<platform::i64re>().get(asset.header.endian);
  } else {
    offset = from.template Read<platform::u32re>().get(asset.header.endian);
  }
  size = from.template Read<platform::u32re>().get(asset.header.endian);

  type_id = from.template Read<platform::u32re>().get(asset.header.endian);
  if (asset.header.version < 16) {
    class_id = (ClassID)from.template Read<platform::u16re>().get(asset.header.endian);
  } else {
    class_id = asset.types[type_id].class_id;
  }

  if (asset.header.version < 11) {
    destroyed = from.template Read<platform::u16re>().get(asset.header.endian);
  } else if(asset.header.version < 17) {
    script_type = from.template Read<platform::i16re>().get(asset.header.endian);
  }

  if (asset.header.version == 15 || asset.header.version == 16)
    stripped = (bool)from.template Read<std::uint8_t>();
}


template <common::DataView Source>
std::expected<Asset, std::string> Asset::Read(Source&& from) {
  Asset data {std::forward<Source>(from)};
  common::DataReader<Source&> reader{data.source_.GetUnchecked<Source>()};

  data.header.Read(reader);
  // from now on, account for endianness from header

  if (data.header.version >= 7) {
    data.version = reader.template ReadNullTerm<char>();
  } else {
    data.version = nullptr;
  }

  if (data.header.version >= 8) {
    using re_platform = platform::RuntimeOrder<TargetPlatform>;
    data.platform = reader.template Read<re_platform>().get(data.header.endian);
  } else {
    data.platform = TargetPlatform::Unknown;
  }
  if (data.header.version >= 13) {
    data.enable_typetree = reader.template Read<std::uint8_t>();
  } else {
    data.enable_typetree = false;
  }

  data.type_count = reader.template Read<platform::u32re>().get(data.header.endian);
  data.types = std::make_unique<Type[]>(data.type_count);
  for (std::uint32_t i = 0; i < data.type_count; ++i) 
    data.types[i] = Type::Read(reader, data.header.endian, data.header.version, false, data.enable_typetree);
  
  if (data.header.version >= 7 && data.header.version < 14) {
    data.big_id_enabled = reader.template Read<platform::u32re>().get(data.header.endian);
  } else {
    data.big_id_enabled = 0;
  }
  
  data.object_count = reader.template Read<platform::u32re>().get(data.header.endian);
  data.objects = std::make_unique<Object[]>(data.object_count);
  for (std::uint32_t i = 0; i < data.object_count; ++i) 
    data.objects[i].Read(reader, data);

  if (data.header.version < 11) {
    data.script_count = 0;
  } else {
    data.script_count = reader.template Read<platform::u32re>().get(data.header.endian);
    data.scripts = std::make_unique<ScriptType[]>(data.script_count);
    for (std::uint32_t i = 0; i < data.script_count; ++i) {
      data.scripts[i].file_index = reader.template Read<platform::u32re>().get(data.header.endian);
      if (data.header.version >= 14) {
        reader.AlignTo(4);
        data.scripts[i].index_in_file = reader.template Read<platform::u64re>().get(data.header.endian);        
      } else {
        data.scripts[i].index_in_file = reader.template Read<platform::u32re>().get(data.header.endian);        
      }
    }
  }

  data.externals_count = reader.template Read<platform::u32re>().get(data.header.endian);
  data.externals = std::make_unique<FileIdentifier[]>(data.externals_count);
  for (std::uint32_t i = 0; i < data.externals_count; ++i) {
    if (data.header.version >= 6)
      reader.template ReadNullTerm<char>();
    if (data.header.version >= 5) {
      data.externals[i].guid = reader.template Read<common::Guid>();
      data.externals[i].type = (AssetType)reader.template Read<platform::i32re>().get(data.header.endian);
    }
    data.externals[i].path = reader.template ReadNullTerm<char>();
  }

  if (data.header.version < 20) {
    data.reftype_count = 0;
  } else {
    data.reftype_count = reader.template Read<platform::u32re>().get(data.header.endian);
    data.reftypes = std::make_unique<Type[]>(data.type_count);
    for (std::uint32_t i = 0; i < data.reftype_count; ++i) 
      data.reftypes[i] = Type::Read(reader, data.header.endian, data.header.version, true, data.enable_typetree);
  }

  if (data.header.version >= 5)
    reader.template ReadNullTerm<char>();  // user information

  auto pos = std::exchange(reader.position, data.header.data_offset);
  data.data = (std::uint8_t*)reader.current();
  reader.position = pos;

  return std::move(data);
}

template<common::DataView Source>
bool Asset::Detect(Source& from) {
  if (from.size() < 48) return false;
  common::DataReader<Source&> reader{from};

  const char* header = reader.template ReadArray<char>(8);
  constexpr auto bundle_header = "Unity";
  if (std::equal(bundle_header, bundle_header + 5, header)) return false;

  reader.position = 0;
  std::uint32_t meta_size = reader.template Read<platform::u32be>();
  std::uint32_t data_size = reader.template Read<platform::u32be>();
  if (data_size != from.size()) return false;
  if (meta_size >= data_size) return false;

  std::uint32_t version = reader.template Read<platform::u32be>();
  if (version > 50) return false;

  if (version >= 7) {
    reader.position += (version < 22) ? 8 : 36;
    constexpr auto probe_len = 64;
    const char* fullver = reader.template ReadArray<char>(probe_len);
 
    int index = 0;
    int dots = 0;
    for(; index < probe_len; ++index) {
      if (fullver[index] == '\0') break;
      if (fullver[index] == '.') {
        if (++dots > 8) return false;
        continue;
      }
      if ((fullver[index] != '-') && !isalnum((std::uint8_t)fullver[index])) return false;
    }

    return (index < probe_len) && (dots > 1);
  }

  return true;
}

template<common::DataView Source>
Asset::Asset(Source&& from) 
  : source_{common::Any::Make<Source>(std::forward<Source>(from))}
  {}

}  // namespace file

}  // namespace unity
