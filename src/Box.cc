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
#include <ignition/math/Vector3.hh>
#include "sdf/Box.hh"
#include "sdf/Geometry.hh"

using namespace sdf;

// Private data class
class sdf::BoxPrivate
{
  // Size of the box
  public: ignition::math::Vector3d size = ignition::math::Vector3d::One;
};

/////////////////////////////////////////////////
Box::Box()
  : dataPtr(new BoxPrivate)
{
}

/////////////////////////////////////////////////
Box::~Box()
{
  delete this->dataPtr;
  this->dataPtr = nullptr;
}

/////////////////////////////////////////////////
Errors Box::Load(ElementPtr _sdf)
{
  Errors errors;

  // Check that sdf is a valid pointer
  if (!_sdf)
  {
    errors.push_back({ErrorCode::ELEMENT_MISSING,
        "Attempting to load a box, but the provided SDF "
        "element is null."});
    return errors;
  }

  // We need a box element
  if (_sdf->GetName() != "box")
  {
    errors.push_back({ErrorCode::ELEMENT_INCORRECT_TYPE,
        "Attempting to load a box geometry, but the provided SDF "
        "element is not a <box>."});
    return errors;
  }

  if (_sdf->HasElement("size"))
  {
    std::pair<ignition::math::Vector3d, bool> pair =
      _sdf->Get<ignition::math::Vector3d>("size", this->dataPtr->size);

    if (!pair.second)
    {
      errors.push_back({ErrorCode::ELEMENT_INVALID,
          "Invalid <size> data for a <box> geometry. "
          "Using a size of 1, 1, 1 "});
    }
    this->dataPtr->size = pair.first;
  }
  else
  {
    errors.push_back({ErrorCode::ELEMENT_MISSING,
        "Box geometry is missing a <size> child element. "
        "Using a size of 1, 1, 1."});
  }

  return errors;
}

//////////////////////////////////////////////////
ignition::math::Vector3d Box::Size() const
{
  return this->dataPtr->size;
}

//////////////////////////////////////////////////
void Box::SetSize(const ignition::math::Vector3d &_size)
{
  this->dataPtr->size = _size;
}