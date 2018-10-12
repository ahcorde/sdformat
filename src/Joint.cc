/*
 * Copyright 2018 Open Source Robotics Foundation
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
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <ignition/math/Pose3.hh>
#include "sdf/Error.hh"
#include "sdf/Joint.hh"
#include "sdf/JointAxis.hh"
#include "sdf/Types.hh"
#include "Utils.hh"

using namespace sdf;

class sdf::JointPrivate
{
  public: JointPrivate()
  {
    // Initialize here because windows does not support list initialization
    // at member initialization (ie ... axis = {{nullptr, nullpter}};).
    this->axis[0] = nullptr;
    this->axis[1] = nullptr;
  }

  /// \brief Name of the joint.
  public: std::string name = "";

  /// \brief Name of the parent link.
  public: std::string parentLinkName = "";

  /// \brief Name of the child link.
  public: std::string childLinkName = "";

  /// \brief the joint type.
  public: JointType type = JointType::INVALID;

  /// \brief Pose of the joint
  public: ignition::math::Pose3d pose = ignition::math::Pose3d::Zero;

  /// \brief Frame of the pose.
  public: std::string poseFrame = "";

  /// \brief Joint axis
  // cppcheck-suppress
  public: std::array<std::unique_ptr<JointAxis>, 2> axis;

  public: std::shared_ptr<FrameGraph> frameGraph = nullptr;

  /// \brief The SDF element pointer used during load.
  public: sdf::ElementPtr sdf;
};

/////////////////////////////////////////////////
Joint::Joint()
  : dataPtr(new JointPrivate)
{
}

/////////////////////////////////////////////////
Joint::Joint(Joint &&_joint)
{
  this->dataPtr = _joint.dataPtr;
  _joint.dataPtr = nullptr;
}

/////////////////////////////////////////////////
Joint::~Joint()
{
  delete this->dataPtr;
  this->dataPtr = nullptr;
}

/////////////////////////////////////////////////
Errors Joint::Load(ElementPtr _sdf,
    std::shared_ptr<FrameGraph> _frameGraph)
{
  Errors errors;

  this->dataPtr->sdf = _sdf;

  // Check that the provided SDF element is a <joint>
  // This is an error that cannot be recovered, so return an error.
  if (_sdf->GetName() != "joint")
  {
    errors.push_back({ErrorCode::ELEMENT_INCORRECT_TYPE,
        "Attempting to load a Joint, but the provided SDF element is not a "
        "<joint>."});
    return errors;
  }

  // Read the joints's name
  if (!loadName(_sdf, this->dataPtr->name))
  {
    errors.push_back({ErrorCode::ATTRIBUTE_MISSING,
                     "A joint name is required, but the name is not set."});
  }

  // Load the pose. Ignore the return value since the pose is optional.
  loadPose(_sdf, this->dataPtr->pose, this->dataPtr->poseFrame);

  // Read the parent link name
  std::pair<std::string, bool> parentPair =
    _sdf->Get<std::string>("parent", "");
  if (parentPair.second)
    this->dataPtr->parentLinkName = parentPair.first;
  else
  {
    errors.push_back({ErrorCode::ELEMENT_MISSING,
        "The parent element is missing."});
  }

  // Read the child link name
  std::pair<std::string, bool> childPair = _sdf->Get<std::string>("child", "");
  if (childPair.second)
    this->dataPtr->childLinkName = childPair.first;
  else
  {
    errors.push_back({ErrorCode::ELEMENT_MISSING,
        "The child element is missing."});
  }

  if (_sdf->HasElement("axis"))
  {
    this->dataPtr->axis[0].reset(new JointAxis());
    Errors axisErrors = this->dataPtr->axis[0]->Load(_sdf->GetElement("axis"));
    errors.insert(errors.end(), axisErrors.begin(), axisErrors.end());
  }

  if (_sdf->HasElement("axis2"))
  {
    this->dataPtr->axis[1].reset(new JointAxis());
    Errors axisErrors = this->dataPtr->axis[1]->Load(_sdf->GetElement("axis2"));
    errors.insert(errors.end(), axisErrors.begin(), axisErrors.end());
  }

  // Read the type
  std::pair<std::string, bool> typePair = _sdf->Get<std::string>("type", "");
  if (typePair.second)
  {
    std::transform(typePair.first.begin(), typePair.first.end(),
        typePair.first.begin(),
        [](unsigned char c)
        {
          return static_cast<unsigned char>(std::tolower(c));
        });
    if (typePair.first == "ball")
      this->dataPtr->type = JointType::BALL;
    else if (typePair.first == "continuous")
      this->dataPtr->type = JointType::CONTINUOUS;
    else if (typePair.first == "fixed")
      this->dataPtr->type = JointType::FIXED;
    else if (typePair.first == "gearbox")
      this->dataPtr->type = JointType::GEARBOX;
    else if (typePair.first == "prismatic")
      this->dataPtr->type = JointType::PRISMATIC;
    else if (typePair.first == "revolute")
      this->dataPtr->type = JointType::REVOLUTE;
    else if (typePair.first == "revolute2")
      this->dataPtr->type = JointType::REVOLUTE2;
    else if (typePair.first == "screw")
      this->dataPtr->type = JointType::SCREW;
    else if (typePair.first == "universal")
      this->dataPtr->type = JointType::UNIVERSAL;
    else
    {
      this->dataPtr->type = JointType::INVALID;
      errors.push_back({ErrorCode::ATTRIBUTE_INVALID,
          "Joint type of " + typePair.first +
          " is invalid. Refer to the SDF documentation for a list of "
          "valid joint types"});
    }
  }
  else
  {
    errors.push_back({ErrorCode::ATTRIBUTE_MISSING,
        "A joint type is required, but is not set."});
  }

  if (_frameGraph)
  {
    _frameGraph->AddVertex(this->dataPtr->name,
        ignition::math::Matrix4d(this->dataPtr->pose));
    this->dataPtr->frameGraph = _frameGraph;
  }

  return errors;
}

/////////////////////////////////////////////////
const std::string &Joint::Name() const
{
  return this->dataPtr->name;
}

/////////////////////////////////////////////////
void Joint::SetName(const std::string &_name) const
{
  this->dataPtr->name = _name;
}

/////////////////////////////////////////////////
JointType Joint::Type() const
{
  return this->dataPtr->type;
}

/////////////////////////////////////////////////
void Joint::SetType(const JointType _jointType)
{
  this->dataPtr->type = _jointType;
}

/////////////////////////////////////////////////
const std::string &Joint::ParentLinkName() const
{
  return this->dataPtr->parentLinkName;
}

/////////////////////////////////////////////////
void Joint::SetParentLinkName(const std::string &_name) const
{
  this->dataPtr->parentLinkName = _name;
}

/////////////////////////////////////////////////
const std::string &Joint::ChildLinkName() const
{
  return this->dataPtr->childLinkName;
}

/////////////////////////////////////////////////
void Joint::SetChildLinkName(const std::string &_name) const
{
  this->dataPtr->childLinkName = _name;
}

/////////////////////////////////////////////////
const JointAxis *Joint::Axis(const unsigned int _index) const
{
  return this->dataPtr->axis[std::min(_index, 1u)].get();
}

/////////////////////////////////////////////////
const ignition::math::Pose3d &Joint::Pose() const
{
  return this->dataPtr->pose;
}

/////////////////////////////////////////////////
const std::string &Joint::PoseFrame() const
{
  return this->dataPtr->poseFrame;
}

/////////////////////////////////////////////////
void Joint::SetPose(const ignition::math::Pose3d &_pose)
{
  this->dataPtr->pose = _pose;
}

/////////////////////////////////////////////////
void Joint::SetPoseFrame(const std::string &_frame)
{
  this->dataPtr->poseFrame = _frame;
}

/////////////////////////////////////////////////
sdf::ElementPtr Joint::Element() const
{
  return this->dataPtr->sdf;
}