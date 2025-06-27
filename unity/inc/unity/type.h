#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include <platform/endian.h>
#include <common/data_reader.h>
#include <unity/misc.h>
#include <unity/flags.h>

namespace unity {

struct TypeTree {
  struct Node {
    std::uint16_t version;
    std::uint8_t level;
    std::uint8_t flags;
    std::int32_t size;
    std::uint32_t meta_flags;
    std::uint64_t ref_type_hash;

    union {
      struct {
        const char* type;
        const char* name;
      };
      struct {
        std::uint32_t type_offset;
        std::uint32_t name_offset;
      };
    };

    bool aligned() const;
  };  // struct Node

  std::uint32_t node_count;
  std::unique_ptr<Node[]> nodes;
  const char* strings;

  const char* GetString(std::uint32_t offset) const;

  template<common::DataView Source>
  void ReadBlob(common::DataReader<Source>& from, platform::Endian endian, std::uint32_t version);

  template<common::DataView Source>
  void Read(common::DataReader<Source>& from, platform::Endian endian, std::uint32_t version);
};  // struct TypeTree

struct Type {
  ClassID class_id;
  bool is_stripped;
  std::int16_t script_type_index;
  Hash128 script_id;
  Hash128 old_type_hash;
  TypeTree tree;
  std::variant<TypeName, Sized<platform::u32re>> depends;

  template<common::DataView Source>
  static Type Read(common::DataReader<Source>& from, 
                   platform::Endian endian, 
                   std::uint32_t version, 
                   bool is_ref, bool use_typetree);
};  // struct Type

template<common::DataView Source>
void TypeTree::Read(common::DataReader<Source>& from, platform::Endian endian, std::uint32_t version) {
  assert(false && "TODO");
}

template<common::DataView Source>
void TypeTree::ReadBlob(common::DataReader<Source>& from, platform::Endian endian, std::uint32_t version) {
  node_count = from.template Read<platform::u32re>().get(endian);
  auto string_size = from.template Read<platform::u32re>().get(endian);
  nodes = std::make_unique<Node[]>(node_count);
  for (std::uint32_t i = 0; i < node_count; ++i) {
    nodes[i].version = from.template Read<platform::u16re>().get(endian);
    nodes[i].level = from.template Read<std::uint8_t>();
    nodes[i].flags = from.template Read<std::uint8_t>();
    nodes[i].type_offset = from.template Read<platform::u32re>().get(endian);
    nodes[i].name_offset = from.template Read<platform::u32re>().get(endian);
    nodes[i].size = from.template Read<platform::i32re>().get(endian);
    auto _ = from.template Read<platform::u32re>();  // "index" is not stored
    nodes[i].meta_flags = from.template Read<platform::u32re>().get(endian);
    if (version > 19) {
      nodes[i].ref_type_hash = from.template Read<platform::u64re>().get(endian);
    }
  }
  strings = from.template ReadArray<char>(string_size);
  for (std::uint32_t i = 0; i < node_count; ++i) {
    auto type_offset = nodes[i].type_offset;
    auto name_offset = nodes[i].name_offset;
    // don't reorder - these fields are in 'union'
    nodes[i].type = GetString(type_offset);
    nodes[i].name = GetString(name_offset);
  }
}

template<common::DataView Source>
Type Type::Read(common::DataReader<Source>& from, 
                platform::Endian endian, 
                std::uint32_t version, 
                bool is_ref, bool use_typetree) {

  Type ret;
  ret.class_id = (ClassID)from.template Read<platform::u32re>().get(endian);
  if (version >= 16) {
    ret.is_stripped = from.template Read<std::uint8_t>();
  } else {
    ret.is_stripped = false;
  }
  if (version >= 17) {
    ret.script_type_index = from.template Read<platform::i16re>().get(endian);
  } else {
    ret.script_type_index = -1;
  }
  if (version >= 13) {
    bool has_script = (version < 16) 
                    ? (ret.class_id < (ClassID)0) 
                    : (ret.class_id == ClassID::MonoBehaviour);
    if ((is_ref && ret.script_type_index >= 0) || has_script)
      ret.script_id = from.template Read<Hash128>();
    ret.old_type_hash = from.template Read<Hash128>();
  }

  if (use_typetree) {
    if (version >= 12 || version == 10) {
      ret.tree.ReadBlob(from, endian, version);
    } else {
      ret.tree.Read(from, endian, version);
    }
    if (version >= 21) {
      if (is_ref) {
        ret.depends = TypeName::Read(from);
      } else {
        auto size = from.template Read<platform::u32re>().get(endian);
        ret.depends = Sized<platform::u32re> {
          size,
          from.template ReadArray<platform::u32re>(size)
        };
      }
    } else {
      ret.depends = {};
    }
  }
  
  return ret;
}

inline bool TypeTree::Node::aligned() const {
  return meta_flags & 0x4000;
}

}  // namespace unity