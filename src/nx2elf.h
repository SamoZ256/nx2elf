#pragma once

#include <filesystem>
#include <functional>
#include <vector>

#include "elf.h"
#include "types.h"

struct NsoFile {
  enum FileType {
    kUnknown,
    kNso,
    kNro,
    kMod,
  };

  enum SegmentType { kText, kRodata, kData, kNumSegment };

  static const std::array<u8, 4> nso_magic;
  static const std::array<u8, 4> nro_magic;
  static const std::array<u8, 4> mod_magic;

  struct SegmentHeader {
    u32 file_offset;  // maybe &1==compressed?
    u32 mem_offset;
    u32 mem_size;
    u32 bss_align;
  };

  struct DataExtent {
    u32 offset;
    u32 size;
  };

  struct NsoHeader {
    u8 magic[4];
    u32 field_4;
    u32 field_8;
    u32 field_c;
    SegmentHeader segments[kNumSegment];
    // value from .note, can be various lengths :/
    std::array<u8, 32> gnu_build_id;
    u32 segment_file_sizes[kNumSegment];
    u32 field_6c[9];
    DataExtent dynstr;
    DataExtent dynsym;
    sha256_digest segment_digests[kNumSegment];
  };

  // NRO stores the flat memory image - nothing needs to be decompressed or
  // relocated (although relocation fixups need to be applied). This also
  // implies that +4 in the file points to MOD header, so NRO header is at
  // offset 0x10 instead of 0.
  struct NroHeader {
    u8 magic[4];
    u32 field_4;
    u32 file_size;
    u32 field_c;
    DataExtent segments[kNumSegment];
    u32 bss_size;
    u32 field_3c;
    std::array<u8, 32> gnu_build_id;
    u32 field_60[4];
    DataExtent dynstr;
    DataExtent dynsym;
  };

  struct ModPointer {
    u32 field_0;
    u32 magic_offset;
  };

  struct ModHeader {
    // yaya, there are some fields here...for parsing, easier to ignore.
    // ModPointer mod_ptr;
    u8 magic[4];
    s32 dynamic_offset;
    s32 bss_start_offset;
    s32 bss_end_offset;
    s32 eh_start_offset;
    s32 eh_end_offset;
    s32 module_object_offset;
    // It seems the area around MOD0 is used for .note section
    // There is also a nss-name section
  };

  template <typename T>
  char* FormatBytes(char* p, T d) {
    for (auto& b : d)
      p += sprintf(p, "%02x", b);
    return p;
  }
  void Dump(bool verbose = false);
  bool Decompress(u8* dst, u32 dst_len, const u8* src, u32 src_len);
  bool ResolvePlt(void* base, size_t len);
  bool Load(const std::vector<u8>& file);
  void DumpElfInfo();
  void iter_dynsym(std::function<void(const Elf64_Sym&, u32)> func);
  bool WriteElf(const std::filesystem::path& path);

  FileType file_type{kUnknown};

  NsoHeader header{};

  std::vector<u8> image;
  const Elf64_Dyn* dynamic{};
  const Elf64_Nhdr* note{};

  struct {
    u64 symtab;
    u64 rela;
    u64 relasz;
    u64 jmprel;
    u64 pltrelsz;
    u64 strtab;
    u64 strsz;
    u64 pltgot;
    u64 hash;
    u64 gnu_hash;
    u64 init;
    u64 fini;
    u64 init_array;
    u64 init_arraysz;
    u64 fini_array;
    u64 fini_arraysz;
  } dyn_info{};

  struct {
    u64 addr;
    u64 size;
  } plt_info;

  struct {
    u64 hdr_addr;
    u64 hdr_size;
    u64 frame_addr;
    u64 frame_size;
  } eh_info{};
};
