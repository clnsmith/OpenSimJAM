#ifndef OPENSIM_PROPERTY_TRANSFORM_H_
#define OPENSIM_PROPERTY_TRANSFORM_H_
/* -------------------------------------------------------------------------- *
 *                       OpenSim:  PropertyTransform.h                        *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2017 Stanford University and the Authors                *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#ifdef _WIN32
#pragma warning( disable : 4251 )
#endif

#include "osimCommonDLL.h"
#include <string>
#include "PropertyDblArray.h"
#include "SimTKcommon/Orientation.h"

//=============================================================================
//=============================================================================
namespace OpenSim { 

/**
 * Class PropertyTransform extends class Property.  It consists of a
 * Transform (i.e., SimTK::Transform) and the methods for accessing
 * and modifying this Transform.
 *
 * @version 1.0
 * @author Ayman HAbib
 */
class OSIMCOMMON_API PropertyTransform : public PropertyDblArray
{

//=============================================================================
// DATA
//=============================================================================
private:
    /** Transform. */
    SimTK::Transform _transform;

//=============================================================================
// METHODS
//=============================================================================
    //--------------------------------------------------------------------------
    // CONSTRUCTION
    //--------------------------------------------------------------------------
public:
    PropertyTransform();
    PropertyTransform(const std::string &aName,
        const SimTK::Transform& aTransform);
    PropertyTransform(const std::string &aName,
        const Array<double> &aArray);
    PropertyTransform(const PropertyTransform &aProperty);
    PropertyTransform* clone() const override;

    //--------------------------------------------------------------------------
    // OPERATORS
    //--------------------------------------------------------------------------
public:
#ifndef SWIG
    PropertyTransform& operator=(const PropertyTransform &aProperty);
#endif
    //--------------------------------------------------------------------------
    // GET AND SET
    //--------------------------------------------------------------------------
public:
    // TYPE
    std::string getTypeName() const override;
    // VALUE
    virtual void setValue(const SimTK::Transform &aTransform);
    virtual SimTK::Transform& getValueTransform();
#ifndef SWIG
    virtual const SimTK::Transform& getValueTransform() const;
#endif
    void setValue(int aSize,const double aArray[]) override;
    void getRotationsAndTranslationsAsArray6(double aArray[]) const;

    void setValue(const Array<double> &aArray) override{
        setValue(6, &aArray[0]);
    };
    // VALUE as String
    std::string toString() const override;


//=============================================================================
};  // END of class PropertyTransform

}; //namespace
//=============================================================================
//=============================================================================

#endif // OPENSIM_PROPERTY_TRANSFORM_H_
