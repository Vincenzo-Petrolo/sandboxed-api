// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../sandboxed.h"  // NOLINT(build/include)
#include "absl/algorithm/container.h"
#include "absl/strings/str_join.h"
#include "sandboxed_api/sandbox2/util/fileops.h"
#include "sandboxed_api/sandbox2/util/path.h"
#include "sandboxed_api/vars.h"
#include "tiffio.h"  // NOLINT(build/include)

namespace {

struct ChannelLimits {
  uint8_t min_red;
  uint8_t max_red;
  uint8_t min_green;
  uint8_t max_green;
  uint8_t min_blue;
  uint8_t max_blue;
  uint8_t min_alpha;
  uint8_t max_alpha;
};

constexpr uint32_t kRawTileNumber = 9;
constexpr uint32_t kClusterSize = 6;
constexpr uint32_t kChannelsInPixel = 3;
constexpr uint32_t kTestCount = 3;
constexpr uint32_t kImageSize = 128 * 128;
constexpr uint32_t kClusterImageSize = 64 * 64;
using ClusterData = std::array<uint8_t, kClusterSize>;

constexpr std::array<std::pair<uint32_t, ClusterData>, kTestCount> kClusters = {
    {{0, {0, 0, 2, 0, 138, 139}},
     {64, {0, 0, 9, 6, 134, 119}},
     {128, {44, 40, 63, 59, 230, 95}}}};

constexpr std::array<std::pair<uint32_t, ChannelLimits>, kTestCount> kLimits = {
    {{0, {15, 18, 0, 0, 18, 41, 255, 255}},
     {64, {0, 0, 0, 0, 0, 2, 255, 255}},
     {512, {5, 6, 34, 36, 182, 196, 255, 255}}}};

constexpr absl::string_view kClusterErrorFormatStr =
    "Cluster %d did not match expected results.\n"
    "Expect:\t%s\n"
    "Got:\t%s";

constexpr absl::string_view kRgbPixelErrorFormatStr =
    "Pixel %d did not match expected results.\n"
    "Got R=%d (expected %d..%d), G=%d (expected %d..%d), "
    "B=%d (expected %d..%d)";

constexpr absl::string_view kRgbaPixelErrorFormatStr =
    "Pixel %d did not match expected results.\n"
    "Got R=%d (expected %d..%d), G=%d (expected %d..%d), "
    "B=%d (expected %d..%d), A=%d (expected %d..%d)";

absl::Status CheckCluster(uint32_t cluster,
                          const sapi::v::Array<uint8_t>& buffer,
                          const ClusterData& expected_cluster) {
  if (buffer.GetSize() < (cluster + 1) * kClusterSize) {
    return absl::InternalError("Buffer overrun");
  }

  std::vector<uint8_t> target(buffer.GetData() + cluster * kClusterSize,
                              buffer.GetData() + (cluster + 1) * kClusterSize);

  if (absl::c_equal(absl::MakeSpan(target), expected_cluster)) {
    return absl::OkStatus();
  }

  // the image is split on 6-bit clusters because it has YCbCr color format
  return absl::InternalError(absl::StrFormat(
      kClusterErrorFormatStr, cluster, absl::StrJoin(expected_cluster, "\t"),
      absl::StrJoin(target, "\t")));
}

absl::Status CheckRgbPixel(uint32_t pixel, const ChannelLimits& limits,
                           const sapi::v::Array<uint8_t>& buffer) {
  if (buffer.GetSize() < (pixel + 1) * kChannelsInPixel) {
    return absl::InternalError("Buffer overrun");
  }

  uint8_t* rgb = buffer.GetData() + pixel * kChannelsInPixel;

  if (rgb[0] >= limits.min_red && rgb[0] <= limits.max_red &&
      rgb[1] >= limits.min_green && rgb[1] <= limits.max_green &&
      rgb[2] >= limits.min_blue && rgb[2] <= limits.max_blue) {
    return absl::OkStatus();
  }

  return absl::InternalError(absl::StrFormat(
      kRgbPixelErrorFormatStr, pixel, rgb[0], limits.min_red, limits.max_red,
      rgb[1], limits.min_green, limits.max_green, rgb[2], limits.min_blue,
      limits.max_blue));
}

absl::Status CheckRgbaPixel(uint32_t pixel, const ChannelLimits& limits,
                            const sapi::v::Array<uint32_t>& buffer) {
  // RGBA images are upside down - adjust for normal ordering
  uint32_t adjusted_pixel = pixel % 128 + (127 - (pixel / 128)) * 128;

  if (buffer.GetSize() <= adjusted_pixel) {
    return absl::InternalError("Buffer overrun");
  }

  uint32_t rgba = buffer[adjusted_pixel];
  if (TIFFGetR(rgba) >= static_cast<unsigned>(limits.min_red) &&
      TIFFGetR(rgba) <= static_cast<unsigned>(limits.max_red) &&
      TIFFGetG(rgba) >= static_cast<unsigned>(limits.min_green) &&
      TIFFGetG(rgba) <= static_cast<unsigned>(limits.max_green) &&
      TIFFGetB(rgba) >= static_cast<unsigned>(limits.min_blue) &&
      TIFFGetB(rgba) <= static_cast<unsigned>(limits.max_blue) &&
      TIFFGetA(rgba) >= static_cast<unsigned>(limits.min_alpha) &&
      TIFFGetA(rgba) <= static_cast<unsigned>(limits.max_alpha)) {
    return absl::OkStatus();
  }

  return absl::InternalError(absl::StrFormat(
      kRgbaPixelErrorFormatStr, pixel, TIFFGetR(rgba), limits.min_red,
      limits.max_red, TIFFGetG(rgba), limits.min_green, limits.max_green,
      TIFFGetB(rgba), limits.min_blue, limits.max_blue, TIFFGetA(rgba),
      limits.min_alpha, limits.max_alpha));
}

}  // namespace

std::string GetFilePath(const absl::string_view dir,
                        const absl::string_view filename) {
  return sandbox2::file::JoinPath(dir, "test", "images", filename);
}

std::string GetFilePath(const absl::string_view filename) {
  std::string cwd = sandbox2::file_util::fileops::GetCWD();
  auto find = cwd.rfind("build");

  std::string project_path;
  if (find == std::string::npos) {
    LOG(ERROR)
        << "Something went wrong: CWD don't contain build dir. "
        << "Please run tests from build dir or send project dir as a "
        << "parameter: ./sandboxed /absolute/path/to/project/dir .\n"
        << "Falling back to using current working directory as root dir.\n";
    project_path = cwd;
  } else {
    project_path = cwd.substr(0, find);
  }

  return sandbox2::file::JoinPath(project_path, "test", "images", filename);
}

absl::Status LibTIFFMain(const absl::string_view srcfile) {
  // to use dir and file inside sapi-libtiff, use
  // sandbox(file) – file only -- or
  // sandbox(file, dir) -- file and dir -- or
  // sandbox(nullopt, dir) -- dir only.
  // file and directory must exist.
  // all paths must be absolute.

  TiffSapiSandbox sandbox(srcfile);

  // initialize sapi vars after constructing TiffSapiSandbox

  SAPI_RETURN_IF_ERROR(sandbox.Init());

  TiffApi api(&sandbox);
  sapi::v::ConstCStr srcfile_var(srcfile.data());
  sapi::v::ConstCStr r_var("r");

  absl::StatusOr<TIFF*> status_or_tif;
  SAPI_ASSIGN_OR_RETURN(
      status_or_tif, api.TIFFOpen(srcfile_var.PtrBefore(), r_var.PtrBefore()));

  sapi::v::RemotePtr tif(status_or_tif.value());
  if (!tif.GetValue()) {
    return absl::InternalError(absl::StrCat("Could not open ", srcfile));
  }

  sapi::v::UShort h;
  sapi::v::UShort v;
  SAPI_ASSIGN_OR_RETURN(int return_value,
                        api.TIFFGetField2(&tif, TIFFTAG_YCBCRSUBSAMPLING,
                                          h.PtrBoth(), v.PtrBoth()));
  if (return_value == 0 || h.GetValue() != 2 || v.GetValue() != 2) {
    return absl::InternalError("Could not retrieve subsampling tag");
  }

  SAPI_ASSIGN_OR_RETURN(tsize_t sz, api.TIFFTileSize(&tif));
  if (sz != kClusterSize * kClusterImageSize) {
    return absl::InternalError(
        absl::StrCat("Unexpected TileSize ", sz, ". Expected ",
                     kClusterSize * kClusterImageSize, " bytes"));
  }

  sapi::v::Array<uint8_t> buffer(sz);
  // Read a tile in decompressed form, but still YCbCr subsampled
  SAPI_ASSIGN_OR_RETURN(
      tsize_t new_sz,
      api.TIFFReadEncodedTile(&tif, kRawTileNumber, buffer.PtrBoth(), sz));
  if (new_sz != sz) {
    return absl::InternalError(absl::StrCat(
        "Did not get expected result code from TIFFReadEncodedTile(): ", new_sz,
        " instead of ", sz));
  }

  absl::Status status;
  bool cluster_status_ok = true;
  for (const auto& [id, data] : kClusters) {
    if (status = CheckCluster(id, buffer, data); !status.ok()) {
      LOG(ERROR) << "CheckCluster failed:\n" << status.ToString() << '\n';
    }
    cluster_status_ok &= status.ok();
  }

  if (!cluster_status_ok) {
    return absl::InternalError("One or more clusters failed the check");
  }

  SAPI_ASSIGN_OR_RETURN(
      return_value,
      api.TIFFSetFieldU1(&tif, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB));
  if (return_value == 0) {
    return absl::InternalError("The JPEGCOLORMODE tag cannot be changed");
  }

  SAPI_ASSIGN_OR_RETURN(sz, api.TIFFTileSize(&tif));
  if (sz != kChannelsInPixel * kImageSize) {
    return absl::InternalError(
        absl::StrCat("Unexpected TileSize ", sz, ". Expected ",
                     kChannelsInPixel * kImageSize, " bytes"));
  }

  sapi::v::Array<uint8_t> buffer2_(sz);
  SAPI_ASSIGN_OR_RETURN(
      new_sz,
      api.TIFFReadEncodedTile(&tif, kRawTileNumber, buffer2_.PtrBoth(), sz));
  if (new_sz != sz) {
    return absl::InternalError(absl::StrCat(
        "Did not get expected result code from TIFFReadEncodedTile(): ", new_sz,
        " instead of ", sz));
  }

  bool pixel_status_ok = true;
  for (const auto& [id, data] : kLimits) {
    if (status = CheckRgbPixel(id, data, buffer2_); !status.ok()) {
      LOG(ERROR) << "CheckRgbPixel failed:\n" << status.ToString() << '\n';
    }
    pixel_status_ok &= status.ok();
  }

  SAPI_RETURN_IF_ERROR(api.TIFFClose(&tif));

  SAPI_ASSIGN_OR_RETURN(
      status_or_tif, api.TIFFOpen(srcfile_var.PtrBefore(), r_var.PtrBefore()));

  sapi::v::RemotePtr tif2(status_or_tif.value());
  if (!tif2.GetValue()) {
    return absl::InternalError(absl::StrCat("Could not reopen ", srcfile));
  }

  sapi::v::Array<uint32_t> rgba_buffer(kImageSize);

  // read as rgba
  SAPI_ASSIGN_OR_RETURN(
      return_value,
      api.TIFFReadRGBATile(&tif2, 1 * 128, 2 * 128, rgba_buffer.PtrBoth()));
  if (return_value == 0) {
    return absl::InternalError("TIFFReadRGBATile() returned failure code");
  }

  // Checking specific pixels from the test data, 0th, 64th and 512th
  for (const auto& [id, data] : kLimits) {
    if (status = CheckRgbaPixel(id, data, rgba_buffer); !status.ok()) {
      LOG(ERROR) << "CheckRgbaPixel failed:\n" << status.ToString() << '\n';
    }
    pixel_status_ok &= status.ok();
  }

  SAPI_RETURN_IF_ERROR(api.TIFFClose(&tif2));

  if (!pixel_status_ok) {
    return absl::InternalError("wrong encoding");
  }

  return absl::OkStatus();
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::string srcfile;
  std::string srcfilerel = "quad-tile.jpg.tiff";

  if (argc < 2) {
    srcfile = GetFilePath(srcfilerel);
  } else {
    srcfile = GetFilePath(argv[1], srcfilerel);
  }

  absl::Status status = LibTIFFMain(srcfile);
  if (!status.ok()) {
    LOG(ERROR) << "LibTIFFMain failed with error:\n"
               << status.ToString() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
