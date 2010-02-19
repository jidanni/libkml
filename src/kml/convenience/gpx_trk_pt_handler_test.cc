// Copyright 2008, Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This file contains the unit tests for the GpxTrkPtHandler class.

#include "kml/convenience/gpx_trk_pt_handler.h"
#include <vector>
#include "kml/base/expat_parser.h"
#include "kml/base/file.h"
#include "gtest/gtest.h"

#ifndef DATADIR
#error *** DATADIR must be defined! ***
#endif

namespace kmlconvenience {

class GpxTrkPtHandlerTest : public testing::Test {
};

typedef std::pair<kmlbase::Vec3, std::string> WhereWhenPair;
typedef std::vector<WhereWhenPair> PointVector;

// This test class implements a GpxTrkPtHandler which simply saves everything
// passed to HandlePoint into a vector.
class TestGpxTrkPtHandler : public GpxTrkPtHandler {
 public:
  TestGpxTrkPtHandler(PointVector* point_vector)
    : point_vector_(point_vector) {
  }
  void HandlePoint(const kmlbase::Vec3& where, const std::string& when) {
    point_vector_->push_back(std::make_pair(where, when));
  }
 private:
  PointVector* point_vector_;
};

// By simply compiling this verifies that the HandlePoint method has a default
// implementation.
TEST_F(GpxTrkPtHandlerTest, TestEmpty) {
  GpxTrkPtHandler gpx_trk_pt_handler;
}

// This tests parsing of the <trkpt> element using the StartElement() and
// EndElement() methods.
TEST_F(GpxTrkPtHandlerTest, TestTrkPt) {
  PointVector point_vector;
  TestGpxTrkPtHandler test_gpx_trk_pt_handler(&point_vector);
  const char* trk_pt_atts[] = { "lat", "-123.456", "lon", "37.37", NULL };
  test_gpx_trk_pt_handler.StartElement("trkpt", trk_pt_atts);
  test_gpx_trk_pt_handler.EndElement("trkpt");
  ASSERT_EQ(static_cast<size_t>(1), point_vector.size());
  const kmlbase::Vec3& vec3 = point_vector[0].first;
  ASSERT_EQ(-123.456, vec3.get_latitude());
  ASSERT_EQ(37.37, vec3.get_longitude());
  ASSERT_TRUE(point_vector[0].second.empty());
}

// This tests parsing of the <ele> element using the StartElement(),
// EndElement(), and CharData() methods.
TEST_F(GpxTrkPtHandlerTest, TestEle) {
  PointVector point_vector;
  TestGpxTrkPtHandler test_gpx_trk_pt_handler(&point_vector);
  // <ele> is only parsed within a <trkpt> with both lat and lon.
  const char* trk_pt_atts[] = { "lat", "-123.456", "lon", "37.37", NULL };
  test_gpx_trk_pt_handler.StartElement("trkpt", trk_pt_atts);
  test_gpx_trk_pt_handler.StartElement("ele", NULL);
  const char* kEle("12356.789");
  test_gpx_trk_pt_handler.CharData(kEle, strlen(kEle));
  test_gpx_trk_pt_handler.EndElement("ele");
  test_gpx_trk_pt_handler.EndElement("trkpt");
  ASSERT_EQ(static_cast<size_t>(1), point_vector.size());
  const kmlbase::Vec3& vec3 = point_vector[0].first;
  ASSERT_EQ(-123.456, vec3.get_latitude());
  ASSERT_EQ(37.37, vec3.get_longitude());
  ASSERT_EQ(12356.789, vec3.get_altitude());
}

// This tests parsing of the <ele> element using the StartElement(),
// EndElement(), and CharData() methods.
TEST_F(GpxTrkPtHandlerTest, TestTime) {
  PointVector point_vector;
  TestGpxTrkPtHandler test_gpx_trk_pt_handler(&point_vector);
  // <time> is only parsed within a <trkpt> with both lat and lon.
  const char* trk_pt_atts[] = { "lat", "-123.456", "lon", "37.37", NULL };
  test_gpx_trk_pt_handler.StartElement("trkpt", trk_pt_atts);
  // <time>2008-10-03T11:10:01Z</time>
  test_gpx_trk_pt_handler.StartElement("time", NULL);
  const std::string kTime("2008-10-03T11:10:01Z");
  test_gpx_trk_pt_handler.CharData(kTime.c_str(), kTime.size());
  test_gpx_trk_pt_handler.EndElement("time");
  test_gpx_trk_pt_handler.EndElement("trkpt");
  ASSERT_EQ(static_cast<size_t>(1), point_vector.size());
  const kmlbase::Vec3& vec3 = point_vector[0].first;
  ASSERT_EQ(-123.456, vec3.get_latitude());
  ASSERT_EQ(37.37, vec3.get_longitude());
  ASSERT_EQ(kTime, point_vector[0].second);
}

// These are some expected values from testdata/gpx/trkpts.gpx.
static const struct {
  const size_t index;
  const double latitude;
  const double longitude;
  const double altitude;
  const char* time;
} kTrkPtsFileData[] = {
  { 0, 39.235658487, -106.315917922, 3012.428223, "2007-09-16T19:22:00Z" },
  { 1, 39.235505015, -106.316187400, 3011.467285, "2007-09-16T19:22:03Z" },
  { 141, 39.251128044, -106.287899902, 3125.864258, "2007-09-16T19:50:18Z" },
  { 142, 39.251178671, -106.287928736, 3125.864258, "2007-09-16T19:50:35Z" }
};

// Verify overall usage of GpxTrkPtHandler on a real-world GPX file.
TEST_F(GpxTrkPtHandlerTest, TestTrkPtFile) {
  // Read the GPX file contents.
  std::string gpx_data;
  ASSERT_TRUE(kmlbase::File::ReadFileToString(
      std::string(DATADIR) + "/gpx/trkpts.gpx", &gpx_data));
  PointVector point_vector;
  TestGpxTrkPtHandler test_gpx_trk_pt_handler(&point_vector);
  std::string errors;
  ASSERT_TRUE(
      kmlbase::ExpatParser(gpx_data, &test_gpx_trk_pt_handler, &errors, false));
  ASSERT_TRUE(errors.empty());
  ASSERT_EQ(static_cast<size_t>(143), point_vector.size());
  size_t size = sizeof(kTrkPtsFileData)/sizeof(kTrkPtsFileData[0]);
  for (size_t i = 0; i < size; ++i) {
    size_t index = kTrkPtsFileData[i].index;
    ASSERT_EQ(kTrkPtsFileData[i].latitude,
                         point_vector[index].first.get_latitude());
    ASSERT_EQ(kTrkPtsFileData[i].longitude,
                         point_vector[index].first.get_longitude());
    ASSERT_EQ(kTrkPtsFileData[i].altitude,
                         point_vector[index].first.get_altitude());
    ASSERT_EQ(std::string(kTrkPtsFileData[i].time),
                         point_vector[index].second);
  }
}

}  // namespace kmlconvenience

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
