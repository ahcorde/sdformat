/*
 * Copyright 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <sstream>
#include <string>

#include <gtest/gtest.h>
#include <ignition/math/Helpers.hh>

#include "sdf/Element.hh"
#include "sdf/Frame.hh"
#include "sdf/FrameSemantics.hh"
#include "sdf/Filesystem.hh"
#include "sdf/Link.hh"
#include "sdf/Model.hh"
#include "sdf/Root.hh"
#include "sdf/SDFImpl.hh"
#include "sdf/World.hh"
#include "sdf/parser.hh"
#include "sdf/sdf_config.h"

#include "test_config.h"

/////////////////////////////////////////////////
TEST(FrameSemantics, buildKinematicGraph)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "double_pendulum.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  // Get the first model
  const sdf::Model *model = root.ModelByIndex(0);

  sdf::KinematicGraph graph;
  auto errors = sdf::buildKinematicGraph(graph, model);
  EXPECT_TRUE(errors.empty());

  EXPECT_EQ(3u, graph.map.size());
  EXPECT_EQ(3u, graph.graph.Vertices().size());
  EXPECT_EQ(2u, graph.graph.Edges().size());

  EXPECT_EQ(1u, graph.map.count("base"));
  EXPECT_EQ(1u, graph.map.count("lower_link"));
  EXPECT_EQ(1u, graph.map.count("upper_link"));

  // Disable this part of test since FindSinkVertex isn't part of public API.
  // auto baseId = graph.map["base"];
  // auto lowerLinkId = graph.map["lower_link"];

  // for (auto const &nameId : graph.map)
  // {
  //   EXPECT_EQ(nameId.first, graph.graph.VertexFromId(nameId.second).Name());
  //   auto sourceVertexPair =
  //     ignition::math::graph::FindSourceVertex(graph.graph, nameId.second);
  //   EXPECT_EQ(baseId, sourceVertexPair.first.Id());
  //   auto sinkVertexPair =
  //     ignition::math::graph::FindSinkVertex(graph.graph, nameId.second);
  //   EXPECT_EQ(lowerLinkId, sinkVertexPair.first.Id());
  // }
}

/////////////////////////////////////////////////
TEST(FrameSemantics, buildFrameAttachedToGraph)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "model_frame_attached_to.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  // Get the first model
  const sdf::Model *model = root.ModelByIndex(0);

  sdf::FrameAttachedToGraph graph;
  auto errors = sdf::buildFrameAttachedToGraph(graph, model);
  EXPECT_TRUE(errors.empty());
  EXPECT_TRUE(sdf::validateFrameAttachedToGraph(graph).empty());

  EXPECT_EQ(6u, graph.map.size());
  EXPECT_EQ(6u, graph.graph.Vertices().size());
  EXPECT_EQ(5u, graph.graph.Edges().size());

  EXPECT_EQ(1u, graph.map.count("__model__"));
  EXPECT_EQ(1u, graph.map.count("L"));
  EXPECT_EQ(1u, graph.map.count("F00"));
  EXPECT_EQ(1u, graph.map.count("F0"));
  EXPECT_EQ(1u, graph.map.count("F1"));
  EXPECT_EQ(1u, graph.map.count("F2"));

  // Disable this part of test since FindSinkVertex isn't part of public API.
  // auto linkId = graph.map["L"];

  // for (auto const &nameId : graph.map)
  // {
  //   EXPECT_EQ(nameId.first, graph.graph.VertexFromId(nameId.second).Name());
  //   auto sinkVertexPair =
  //     ignition::math::graph::FindSinkVertex(graph.graph, nameId.second);
  //   EXPECT_EQ(linkId, sinkVertexPair.first.Id());
  // }

  std::string resolvedBody;
  EXPECT_TRUE(
    sdf::resolveFrameAttachedToBody(resolvedBody, graph, "L").empty());
  EXPECT_EQ("L", resolvedBody);
  EXPECT_TRUE(
    sdf::resolveFrameAttachedToBody(resolvedBody, graph, "__model__").empty());
  EXPECT_EQ("L", resolvedBody);
  EXPECT_TRUE(
    sdf::resolveFrameAttachedToBody(resolvedBody, graph, "F00").empty());
  EXPECT_EQ("L", resolvedBody);
  EXPECT_TRUE(
    sdf::resolveFrameAttachedToBody(resolvedBody, graph, "F0").empty());
  EXPECT_EQ("L", resolvedBody);
  EXPECT_TRUE(
    sdf::resolveFrameAttachedToBody(resolvedBody, graph, "F1").empty());
  EXPECT_EQ("L", resolvedBody);
  EXPECT_TRUE(
    sdf::resolveFrameAttachedToBody(resolvedBody, graph, "F2").empty());
  EXPECT_EQ("L", resolvedBody);

  // Try to resolve invalid frame name
  errors = sdf::resolveFrameAttachedToBody(resolvedBody, graph, "invalid");
  for (auto &e : errors)
    std::cerr << e.Message() << std::endl;
  ASSERT_EQ(1u, errors.size());
  EXPECT_EQ(errors[0].Code(), sdf::ErrorCode::FRAME_ATTACHED_TO_INVALID);
  EXPECT_NE(std::string::npos,
      errors[0].Message().find(
        "FrameAttachedToGraph unable to find unique frame with name ["
        "invalid] in graph."));
}

/////////////////////////////////////////////////
TEST(FrameSemantics, buildPoseRelativeToGraph)
{
  const std::string testFile =
    sdf::filesystem::append(PROJECT_SOURCE_PATH, "test", "sdf",
        "model_frame_relative_to_joint.sdf");

  // Load the SDF file
  sdf::Root root;
  EXPECT_TRUE(root.Load(testFile).empty());

  // Get the first model
  const sdf::Model *model = root.ModelByIndex(0);

  sdf::PoseRelativeToGraph graph;
  auto errors = sdf::buildPoseRelativeToGraph(graph, model);
  EXPECT_TRUE(errors.empty());
  EXPECT_TRUE(sdf::validatePoseRelativeToGraph(graph).empty());

  EXPECT_EQ(8u, graph.map.size());
  EXPECT_EQ(8u, graph.graph.Vertices().size());
  EXPECT_EQ(7u, graph.graph.Edges().size());

  EXPECT_EQ(1u, graph.map.count("__model__"));
  EXPECT_EQ(1u, graph.map.count("P"));
  EXPECT_EQ(1u, graph.map.count("C"));
  EXPECT_EQ(1u, graph.map.count("J"));
  EXPECT_EQ(1u, graph.map.count("F1"));
  EXPECT_EQ(1u, graph.map.count("F2"));
  EXPECT_EQ(1u, graph.map.count("F3"));
  EXPECT_EQ(1u, graph.map.count("F4"));

  // Disable this part of test since FindSinkVertex isn't part of public API.
  // auto sinkId = graph.map["__model__"];

  // for (auto const &nameId : graph.map)
  // {
  //   EXPECT_EQ(nameId.first, graph.graph.VertexFromId(nameId.second).Name());
  //   auto sinkVertexPair =
  //     ignition::math::graph::FindSinkVertex(graph.graph, nameId.second);
  //   EXPECT_EQ(sinkId, sinkVertexPair.first.Id());
  // }

  // Test resolvePoseRelativeToRoot for each frame.
  ignition::math::Pose3d pose;
  EXPECT_TRUE(sdf::resolvePoseRelativeToRoot(pose, graph, "__model__").empty());
  EXPECT_EQ(ignition::math::Pose3d::Zero, pose);
  EXPECT_TRUE(sdf::resolvePoseRelativeToRoot(pose, graph, "P").empty());
  EXPECT_EQ(ignition::math::Pose3d(1, 0, 0, 0, 0, 0), pose);
  EXPECT_TRUE(sdf::resolvePoseRelativeToRoot(pose, graph, "F1").empty());
  EXPECT_EQ(ignition::math::Pose3d(1, 0, 1, 0, 0, 0), pose);

  EXPECT_TRUE(sdf::resolvePoseRelativeToRoot(pose, graph, "C").empty());
  EXPECT_EQ(ignition::math::Pose3d(2, 0, 0, 0, IGN_PI/2, 0), pose);
  EXPECT_TRUE(sdf::resolvePoseRelativeToRoot(pose, graph, "F2").empty());
  EXPECT_EQ(ignition::math::Pose3d(4, 0, 0, 0, IGN_PI/2, 0), pose);

  EXPECT_TRUE(sdf::resolvePoseRelativeToRoot(pose, graph, "J").empty());
  EXPECT_EQ(ignition::math::Pose3d(2, 3, 0, 0, 0, 0), pose);
  EXPECT_TRUE(sdf::resolvePoseRelativeToRoot(pose, graph, "F3").empty());
  EXPECT_EQ(ignition::math::Pose3d(2, 3, 3, 0, IGN_PI/2, 0), pose);
  EXPECT_TRUE(sdf::resolvePoseRelativeToRoot(pose, graph, "F4").empty());
  EXPECT_EQ(ignition::math::Pose3d(6, 3, 3, 0, 0, 0), pose);

  // Test resolvePose for each frame with its relative_to value.
  // Numbers should match the raw pose value in the model file.
  EXPECT_TRUE(sdf::resolvePose(pose, graph, "P", "__model__").empty());
  EXPECT_EQ(ignition::math::Pose3d(1, 0, 0, 0, 0, 0), pose);
  EXPECT_TRUE(sdf::resolvePose(pose, graph, "C", "__model__").empty());
  EXPECT_EQ(ignition::math::Pose3d(2, 0, 0, 0, IGN_PI/2, 0), pose);
  EXPECT_TRUE(sdf::resolvePose(pose, graph, "J", "C").empty());
  EXPECT_EQ(ignition::math::Pose3d(0, 3, 0, 0, -IGN_PI/2, 0), pose);

  EXPECT_TRUE(sdf::resolvePose(pose, graph, "F1", "P").empty());
  EXPECT_EQ(ignition::math::Pose3d(0, 0, 1, 0, 0, 0), pose);
  EXPECT_TRUE(sdf::resolvePose(pose, graph, "F2", "C").empty());
  EXPECT_EQ(ignition::math::Pose3d(0, 0, 2, 0, 0, 0), pose);
  EXPECT_TRUE(sdf::resolvePose(pose, graph, "F3", "J").empty());
  EXPECT_EQ(ignition::math::Pose3d(0, 0, 3, 0, IGN_PI/2, 0), pose);
  EXPECT_TRUE(sdf::resolvePose(pose, graph, "F4", "F3").empty());
  EXPECT_EQ(ignition::math::Pose3d(0, 0, 4, 0, -IGN_PI/2, 0), pose);

  // Try to resolve invalid frame names
  errors = sdf::resolvePose(pose, graph, "invalid", "__model__");
  for (auto &e : errors)
    std::cerr << e.Message() << std::endl;
  ASSERT_EQ(1u, errors.size());
  EXPECT_EQ(errors[0].Code(), sdf::ErrorCode::POSE_RELATIVE_TO_INVALID);
  EXPECT_NE(std::string::npos,
      errors[0].Message().find(
        "PoseRelativeToGraph unable to find unique frame with name ["
        "invalid] in graph."));

  errors = sdf::resolvePose(pose, graph, "__model__", "invalid");
  for (auto &e : errors)
    std::cerr << e.Message() << std::endl;
  ASSERT_EQ(1u, errors.size());
  EXPECT_EQ(errors[0].Code(), sdf::ErrorCode::POSE_RELATIVE_TO_INVALID);
  EXPECT_NE(std::string::npos,
      errors[0].Message().find(
        "PoseRelativeToGraph unable to find unique frame with name ["
        "invalid] in graph."));
}