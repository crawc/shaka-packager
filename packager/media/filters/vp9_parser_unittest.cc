// Copyright 2015 Google Inc. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "packager/media/filters/vp9_parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ElementsAre;

namespace edash_packager {
namespace media {
namespace {
MATCHER_P5(EqualVPxFrame,
           frame_size,
           uncompressed_header_size,
           is_key_frame,
           width,
           height,
           "") {
  *result_listener << "which is (" << arg.frame_size << ", "
                   << arg.uncompressed_header_size << ", " << arg.is_key_frame
                   << ", " << arg.width << ", " << arg.height << ").";
  return arg.frame_size == frame_size &&
         arg.uncompressed_header_size == uncompressed_header_size &&
         arg.is_key_frame == is_key_frame && arg.width == width &&
         arg.height == height;
}
}  // namespace

TEST(VP9ParserTest, Superframe) {
  uint8_t data[] = {
      0x85, 0x00, 0x81, 0x25, 0x86, 0x0e, 0x09, 0x07, 0x06, 0x47, 0x00, 0x00,
      0xb4, 0x69, 0x29, 0x1f, 0x69, 0x46, 0x6d, 0xaf, 0x4c, 0x1f, 0xac, 0x8c,
      0x40, 0x7e, 0xb9, 0x52, 0xe3, 0x6f, 0xe9, 0x82, 0x23, 0x62, 0x9a, 0x40,
      0xda, 0x87, 0x21, 0x7f, 0x1f, 0xc8, 0xfe, 0x3f, 0xd1, 0xfc, 0x7f, 0xc1,
      0xbb, 0x3e, 0x77, 0xa4, 0xfc, 0x94, 0xa2, 0xfa, 0xa2, 0x00, 0x7a, 0xc3,
      0x87, 0x01, 0x02, 0x4b, 0x0a, 0x1c, 0x12, 0x0e, 0x0c, 0x75, 0x00, 0x01,
      0xa0, 0x69, 0x23, 0x0f, 0xd2, 0xf6, 0xfb, 0xb0, 0x6b, 0xf2, 0xab, 0x57,
      0xc3, 0x3a, 0xa5, 0x74, 0x4d, 0xb1, 0x48, 0xf4, 0x59, 0x0f, 0xf1, 0x7e,
      0x2f, 0x89, 0xf9, 0x00, 0xab, 0x7b, 0x01, 0x11, 0xd3, 0x8a, 0xe6, 0x8f,
      0xab, 0xeb, 0x5f, 0x57, 0xdd, 0x7f, 0x45, 0x31, 0xbb, 0x66, 0xee, 0xf5,
      0xbc, 0x85, 0xf1, 0xd0, 0x00, 0x7b, 0x80, 0xa7, 0x96, 0xbf, 0x8c, 0x21,
      0xc9, 0x3c, 0x00, 0x48, 0x00, 0xc9,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_TRUE(parser.Parse(data, arraysize(data), &frames));
  EXPECT_THAT(frames, ElementsAre(EqualVPxFrame(60u, 13u, false, 0u, 0u),
                                  EqualVPxFrame(72u, 13u, false, 0u, 0u)));

  // Corrupt super frame marker.
  data[arraysize(data) - 6] = 0xc0;
  ASSERT_FALSE(parser.Parse(data, arraysize(data), &frames));
}

TEST(VP9ParserTest, KeyframeChroma420) {
  const uint8_t kData[] = {
      0x82, 0x49, 0x83, 0x42, 0x00, 0x01, 0xf0, 0x00, 0x74, 0x04, 0x38, 0x24,
      0x1c, 0x18, 0x34, 0x00, 0x00, 0x90, 0x3e, 0x9e, 0xe3, 0xe1, 0xdf, 0x9c,
      0x6c, 0x00, 0x00, 0x41, 0x4d, 0xe4, 0x39, 0x94, 0xcd, 0x7b, 0x78, 0x30,
      0x4e, 0xb5, 0xb1, 0x78, 0x40, 0x6f, 0xe5, 0x75, 0xa4, 0x28, 0x93, 0xf7,
      0x97, 0x9f, 0x4f, 0xdf, 0xbf, 0xfc, 0xe2, 0x73, 0xfa, 0xef, 0xab, 0xcd,
      0x2a, 0x93, 0xed, 0xfc, 0x17, 0x32, 0x8f, 0x40, 0x15, 0xfa, 0xd5, 0x3e,
      0x35, 0x7a, 0x88, 0x69, 0xf7, 0x1f, 0x26, 0x8b,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_TRUE(parser.Parse(kData, arraysize(kData), &frames));
  EXPECT_EQ("vp09.00.00.08.00.01.00.00",
            parser.codec_config().GetCodecString(kCodecVP9));
  EXPECT_THAT(frames,
              ElementsAre(EqualVPxFrame(arraysize(kData), 18u, true, 32u, 8u)));
}

TEST(VP9ParserTest, KeyframeProfile1Chroma422) {
  const uint8_t kData[] = {
      0xa2, 0x49, 0x83, 0x42, 0x08, 0x01, 0x3e, 0x00, 0xb2, 0x80, 0xc7, 0x04,
      0x83, 0x83, 0x0e, 0x40, 0x00, 0x2e, 0x7c, 0x66, 0x79, 0xb9, 0xfd, 0x4f,
      0xc7, 0x86, 0xf7, 0xc3, 0xc0, 0x82, 0xb2, 0x3c, 0xd6, 0xc0, 0xd0, 0x8d,
      0xee, 0x00, 0x47, 0xe0, 0x00, 0x7e, 0x6f, 0xfe, 0x74, 0x31, 0xc6, 0x4f,
      0x23, 0x9d, 0x6e, 0x5f, 0xfc, 0xa8, 0xef, 0x67, 0xdc, 0xac, 0xf7, 0x3e,
      0x31, 0x07, 0xab, 0xc7, 0x11, 0x67, 0x95, 0x30, 0x37, 0x6d, 0xc5, 0xcf,
      0xa0, 0x96, 0xa7, 0xb8, 0xf4, 0xb4, 0x65, 0xff,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_TRUE(parser.Parse(kData, arraysize(kData), &frames));
  EXPECT_EQ("vp09.01.00.08.00.02.00.00",
            parser.codec_config().GetCodecString(kCodecVP9));
  EXPECT_THAT(frames, ElementsAre(EqualVPxFrame(arraysize(kData), 18u, true,
                                                160u, 90u)));
}

TEST(VP9ParserTest, KeyframeProfile2Chroma420) {
  const uint8_t kData[] = {
      0x92, 0x49, 0x83, 0x42, 0x00, 0x04, 0xf8, 0x02, 0xca, 0x04, 0x1c, 0x12,
      0x0e, 0x0c, 0x3d, 0x00, 0x00, 0xa8, 0x7c, 0x66, 0x85, 0xb9, 0xfb, 0x3c,
      0xc9, 0xf0, 0xff, 0xde, 0xf8, 0x78, 0x10, 0x59, 0x5f, 0xaa, 0x6e, 0xf0,
      0x2a, 0x70, 0x00, 0x7e, 0x6f, 0xfe, 0x74, 0x31, 0xc6, 0x4f, 0x23, 0x9d,
      0x6e, 0x5f, 0xfc, 0xa8, 0xef, 0x67, 0xdc, 0xac, 0xf7, 0x3e, 0x31, 0x07,
      0xab, 0xc7, 0x11, 0x67, 0x95, 0x30, 0x37, 0xde, 0x13, 0x16, 0x83, 0x0b,
      0xa4, 0xdf, 0x05, 0xaf, 0x6f, 0xff, 0xd1, 0x74,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_TRUE(parser.Parse(kData, arraysize(kData), &frames));
  EXPECT_EQ("vp09.02.00.10.00.01.00.00",
            parser.codec_config().GetCodecString(kCodecVP9));
  EXPECT_THAT(frames, ElementsAre(EqualVPxFrame(arraysize(kData), 18u, true,
                                                160u, 90u)));
}

TEST(VP9ParserTest, KeyframeProfile3Chroma444) {
  uint8_t kData[] = {
      0xb1, 0x24, 0xc1, 0xa1, 0x40, 0x00, 0x4f, 0x80, 0x2c, 0xa0, 0x41, 0xc1,
      0x20, 0xe0, 0xc3, 0xf0, 0x00, 0x09, 0x00, 0x7c, 0x57, 0x77, 0x3f, 0x67,
      0x99, 0x3e, 0x1f, 0xfb, 0xdf, 0x0f, 0x02, 0x0a, 0x37, 0x81, 0x53, 0x80,
      0x00, 0x7e, 0x6f, 0xfe, 0x74, 0x31, 0xc6, 0x4f, 0x23, 0x9d, 0x6e, 0x5f,
      0xfc, 0xa8, 0xef, 0x67, 0xdc, 0xac, 0xf7, 0x3e, 0x31, 0x07, 0xab, 0xc7,
      0x0c, 0x74, 0x48, 0x8b, 0x95, 0x30, 0xc9, 0xf0, 0x37, 0x3b, 0xe6, 0x11,
      0xe1, 0xe6, 0xef, 0xff, 0xfd, 0xf7, 0x4f, 0x0f,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_TRUE(parser.Parse(kData, arraysize(kData), &frames));
  EXPECT_EQ("vp09.03.00.12.00.03.00.00",
            parser.codec_config().GetCodecString(kCodecVP9));
  EXPECT_THAT(frames, ElementsAre(EqualVPxFrame(arraysize(kData), 19u, true, 160u, 90u)));
}

TEST(VP9ParserTest, Intra) {
  const uint8_t kData[] = {
      0x84, 0xc9, 0x30, 0x68, 0x40, 0x20, 0x2b, 0xe0, 0x23, 0xe8, 0x18, 0x70,
      0x48, 0x38, 0x30, 0xd4, 0x00, 0x04, 0xc0, 0x64, 0x17, 0xe3, 0xd1, 0x7a,
      0x6f, 0x87, 0xfa, 0x3e, 0x1f, 0xe4, 0xd0, 0xc1, 0x56, 0xaf, 0x9d, 0xad,
      0xcb, 0x37, 0x00, 0xf7, 0x5d, 0x83, 0x80, 0x40, 0x0f, 0x9f, 0xd6, 0xbf,
      0xe2, 0xbd, 0x53, 0xd9, 0x00, 0x3a, 0x70, 0xe0, 0x00, 0x78, 0xea, 0xa5,
      0x61, 0x08, 0xb7, 0x9f, 0x33, 0xe5, 0xf8, 0xa5, 0x82, 0x32, 0xbb, 0xa3,
      0x75, 0xb4, 0x60, 0xf3, 0x39, 0x75, 0x1f, 0x2b,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_TRUE(parser.Parse(kData, arraysize(kData), &frames));
  EXPECT_EQ("vp09.00.00.08.00.01.00.00",
            parser.codec_config().GetCodecString(kCodecVP9));
  EXPECT_THAT(frames, ElementsAre(EqualVPxFrame(arraysize(kData), 19u, false,
                                                352u, 288u)));
}

TEST(VP9ParserTest, ShowExisting) {
  const uint8_t kData[] = {0x88};
  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_TRUE(parser.Parse(kData, arraysize(kData), &frames));
  EXPECT_THAT(frames,
              ElementsAre(EqualVPxFrame(arraysize(kData), 1u, false, 0u, 0u)));
}

TEST(VP9ParserTest, Interframe) {
  const uint8_t kData[] = {
      0x86, 0x00, 0x40, 0x92, 0x88, 0x2c, 0x49, 0xe0, 0x00, 0x03, 0x00, 0x00,
      0x00, 0x78, 0xc9, 0x78, 0x71, 0x24, 0x4a, 0x59, 0x44, 0x61, 0xa6, 0x25,
      0xd4, 0x3e, 0xce, 0x00, 0x3a, 0x05, 0xfb, 0x9c, 0xf2, 0x4e, 0xd6, 0x1a,
      0x38, 0x94, 0x86, 0x17, 0x2a, 0x7b, 0x29, 0xbc, 0x22, 0x7e, 0xf8, 0xce,
      0x26, 0x00, 0xb9, 0xb4, 0xfd, 0x74, 0x39, 0x15, 0xaa, 0xe6, 0xe3, 0xb1,
      0xa0, 0xa6, 0x00, 0xf5, 0x6f, 0x57, 0x71, 0x4b, 0x69, 0xd2, 0xcc, 0x21,
      0x90, 0xeb, 0x8c, 0xad, 0x5f, 0x69, 0xb7, 0x9b,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_TRUE(parser.Parse(kData, arraysize(kData), &frames));
  EXPECT_THAT(frames,
              ElementsAre(EqualVPxFrame(arraysize(kData), 10u, false, 0u, 0u)));
}

TEST(VP9ParserTest, CorruptedFrameMarker) {
  const uint8_t kData[] = {0xc8};
  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_FALSE(parser.Parse(kData, arraysize(kData), &frames));
}

TEST(VP9ParserTest, CorruptedSynccode) {
  const uint8_t kData[] = {
      0x82, 0x49, 0x84, 0x42, 0x00, 0x01, 0xf0, 0x00, 0x74, 0x04, 0x38, 0x24,
      0x1c, 0x18, 0x34, 0x00, 0x00, 0x90, 0x3e, 0x9e, 0xe3, 0xe1, 0xdf, 0x9c,
      0x6c, 0x00, 0x00, 0x41, 0x4d, 0xe4, 0x39, 0x94, 0xcd, 0x7b, 0x78, 0x30,
      0x4e, 0xb5, 0xb1, 0x78, 0x40, 0x6f, 0xe5, 0x75, 0xa4, 0x28, 0x93, 0xf7,
      0x97, 0x9f, 0x4f, 0xdf, 0xbf, 0xfc, 0xe2, 0x73, 0xfa, 0xef, 0xab, 0xcd,
      0x2a, 0x93, 0xed, 0xfc, 0x17, 0x32, 0x8f, 0x40, 0x15, 0xfa, 0xd5, 0x3e,
      0x35, 0x7a, 0x88, 0x69, 0xf7, 0x1f, 0x26, 0x8b,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  ASSERT_FALSE(parser.Parse(kData, arraysize(kData), &frames));
}

TEST(VP9ParserTest, NotEnoughBytesForFirstPartitionSize) {
  const uint8_t kData[] = {
      0x82, 0x49, 0x83, 0x42, 0x04, 0xaf, 0xf0, 0x06, 0xbb, 0xdd, 0xf8, 0x03,
      0xfc, 0x00, 0x38, 0x24, 0x1c, 0x18, 0x00, 0x00, 0x03, 0x38, 0x7f, 0x8f,
      0xe8, 0xff, 0xf1, 0x3f, 0xf4, 0x1f, 0xc5, 0xfd, 0xff, 0xf2, 0x7f, 0xf8,
      0x4f, 0xc9, 0xff, 0x5d, 0xff, 0xca, 0xff, 0x91, 0xff, 0xb4, 0xff, 0xe1,
      0xff, 0xa1, 0xff, 0x2b, 0xff, 0xb8, 0xdb, 0x98, 0xff, 0x4b, 0xff, 0x19,
      0xff, 0x0d, 0xf9, 0xbf, 0xf0, 0xbf, 0xe4, 0x7f, 0xbb, 0xff, 0x54, 0x19,
      0x07, 0xf4, 0x7f, 0xc7, 0xff, 0x6d, 0xff, 0xeb,
  };

  VP9Parser parser;
  std::vector<VPxFrameInfo> frames;
  EXPECT_FALSE(parser.Parse(kData, arraysize(kData), &frames));
}

}  // namespace media
}  // namespace edash_packager
