/* -------------------------------------------------------------------------- *
 *                         Blankevoort1991Ligament.cpp                        *
 * -------------------------------------------------------------------------- *
 * Author(s): Colin Smith                                                     *
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

#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/PointForceDirection.h>
#include "Blankevoort1991Ligament.h"
#include <OpenSim/Common/LinearFunction.h>
#include <OpenSim/Common/PolynomialFunction.h>

using namespace SimTK;
using namespace OpenSim;


//=============================================================================
// CONSTRUCTOR(S) AND DESTRUCTOR
//=============================================================================
//_____________________________________________________________________________
/**
 * Default constructor
 */
Blankevoort1991Ligament::Blankevoort1991Ligament() : Force()
{
    constructProperties();
	setNull();
}
Blankevoort1991Ligament::Blankevoort1991Ligament(std::string name, const PhysicalFrame& frame1, Vec3 point1,
	const PhysicalFrame& frame2, Vec3 point2) : Blankevoort1991Ligament()
{
	setName(name);
	upd_GeometryPath().appendNewPathPoint("p1", frame1, point1);
	upd_GeometryPath().appendNewPathPoint("p2", frame2, point2);
}

Blankevoort1991Ligament::Blankevoort1991Ligament(std::string name, const PhysicalFrame& frame1, Vec3 point1,
	const PhysicalFrame& frame2, Vec3 point2,
	double linear_stiffness, double slack_length) :
	Blankevoort1991Ligament(name,frame1, point1, frame2, point2)
{
	set_linear_stiffness(linear_stiffness);
	set_slack_length(slack_length);	
}
//_____________________________________________________________________________




void Blankevoort1991Ligament::setNull()
{
	setAuthors("Colin Smith");
	setReferences(
    "Blankevoort, L. and Huiskes, R., (1991)."
	"Ligament-bone interaction in a three-dimensional model of the knee."
	"J Biomech Eng, 113(3), 263-269"

	"Smith, C.R., Lenhart, R.L., Kaiser, J., Vignos, M.F. and Thelen, D.G.,"
	"(2016). Influence of ligament properties on tibiofemoral mechanics"
	" in walking. J Knee Surg, 29(02), 99-106.\n\n"
	
	"Wismans, J.A.C., Veldpaus, F., Janssen, J., Huson, A. and Struben, P.," 
	"(1980). A three-dimensional mathematical model of the knee-joint. "
	"J Biomech, 13(8), 677-685.\n\n"
	);
}

void Blankevoort1991Ligament::constructProperties()
{
	constructProperty_GeometryPath(GeometryPath());
	constructProperty_linear_stiffness(0.0);
	constructProperty_transition_strain(0.06);
	constructProperty_normalized_damping_coefficient(0.003);
	constructProperty_slack_length(0.0);
	
}

void Blankevoort1991Ligament::extendFinalizeFromProperties()
{
	Super::extendFinalizeFromProperties();

	//Check that properties are valid
	OPENSIM_THROW_IF_FRMOBJ(get_slack_length() < 0.,
        InvalidPropertyValue, getProperty_slack_length().getName(),
        "Slack Length cannot be less than 0");

	OPENSIM_THROW_IF_FRMOBJ(get_linear_stiffness() < 0.,
        InvalidPropertyValue, getProperty_linear_stiffness().getName(),
        "Linear Stiffness cannot be less than 0");

	OPENSIM_THROW_IF_FRMOBJ(get_normalized_damping_coefficient() < 0.,
        InvalidPropertyValue, getProperty_normalized_damping_coefficient().getName(),
        "Normalized Damping Coefficient cannot be less than 0");

	OPENSIM_THROW_IF_FRMOBJ(get_transition_strain() < 0.,
        InvalidPropertyValue, getProperty_transition_strain().getName(),
        "Transistion Strain cannot be less than 0");

	//Set Default Ligament Color
	GeometryPath& path = upd_GeometryPath();
    path.setDefaultColor(SimTK::Vec3(0.1202, 0.7054, 0.1318));

    //Set Spring Force Functions
    //toe region F = 1/2 * k / e_t * e^2
    SimTK::Vector coefficients(3, 0.0);
    coefficients.set(0, 0.5*getLinearStiffnessForcePerStrain() / get_transition_strain());
    _springToeForceFunction = PolynomialFunction(coefficients);
	
    //linear region F = k * (e-e_t/2)
    double slope = getLinearStiffnessForcePerStrain();
    double intercept = -getLinearStiffnessForcePerStrain() * get_transition_strain() / 2;
    _springLinearForceFunction = LinearFunction(slope, intercept);
}

void Blankevoort1991Ligament::extendAddToSystem(SimTK::MultibodySystem& system) const
{
	Super::extendAddToSystem(system);

    addCacheVariable<double>("strain", 0.0, SimTK::Stage::Position);
	addCacheVariable<double>("strain_rate", 0.0, SimTK::Stage::Velocity);
	addCacheVariable<double>("force_spring",0.0, SimTK::Stage::Position);
	addCacheVariable<double>("force_damping", 0.0, SimTK::Stage::Velocity);
	addCacheVariable<double>("force_total", 0.0, SimTK::Stage::Velocity);
}

//=============================================================================
// SCALING
//=============================================================================

void Blankevoort1991Ligament::extendPostScale(
    const SimTK::State& s, const ScaleSet& scaleSet)
{
    Super::extendPostScale(s, scaleSet);

	GeometryPath& path = upd_GeometryPath();
	double slack_length = get_slack_length();
    if (path.getPreScaleLength(s) > 0.0)
    {
        double scaleFactor = path.getLength(s) / path.getPreScaleLength(s);
		set_slack_length(scaleFactor * slack_length);

        // Clear the pre-scale length that was stored in the GeometryPath.
        path.setPreScaleLength(s, 0.0);
    }
}

//=============================================================================
// GET
//=============================================================================

double Blankevoort1991Ligament::getLength(const SimTK::State& state) const {
    return get_GeometryPath().getLength(state);
}

double Blankevoort1991Ligament::getLengtheningSpeed(const SimTK::State& state) const {
    return get_GeometryPath().getLengtheningSpeed(state);
}

double Blankevoort1991Ligament::getStrain(const SimTK::State& state) const{
    if(!isCacheVariableValid(state,"strain")){
        double length = getLength(state);
        double strain = (length - get_slack_length())/get_slack_length();
        
        setCacheVariableValue<double>(state, "strain", strain);
        return strain;
    }
    return getCacheVariableValue<double>(state, "strain");
}

double Blankevoort1991Ligament::getStrainRate(const SimTK::State& state) const {        
   if(!isCacheVariableValid(state,"strain_rate")){
        double lengthening_speed = getLengtheningSpeed(state);
        double strain_rate = lengthening_speed / get_slack_length();

        setCacheVariableValue<double>(state, "strain_rate", strain_rate);
        return strain_rate;
    }
    return getCacheVariableValue<double>(state, "strain_rate");
}

double Blankevoort1991Ligament::getSpringForce(const SimTK::State& state) const {
    if(!isCacheVariableValid(state,"force_spring")){        
        double force = calcSpringForce(state);
        setCacheVariableValue<double>(state, "force_spring", force);
        return force;
    }
    return getCacheVariableValue<double>(state, "force_spring");
}

double Blankevoort1991Ligament::getDampingForce(const SimTK::State& state) const {
    if(!isCacheVariableValid(state,"force_damping")){        
        double force = calcDampingForce(state);
        setCacheVariableValue<double>(state, "force_damping", force);
        return force;
    }
    return getCacheVariableValue<double>(state, "force_damping");
}

double Blankevoort1991Ligament::getTotalForce(const SimTK::State& state) const {
    if(!isCacheVariableValid(state,"force_total")){        
        double force = calcTotalForce(state);
        setCacheVariableValue<double>(state, "force_total", force);
        return force;
    }
    return getCacheVariableValue<double>(state, "force_total");
}

double Blankevoort1991Ligament::getLinearStiffnessForcePerStrain() const {
    return get_linear_stiffness() * get_slack_length();    
};

//=============================================================================
// SET
//=============================================================================

void Blankevoort1991Ligament::setSlackLengthFromReferenceStrain(
    double strain, const SimTK::State& reference_state) {
	
    double reference_length = getLength(reference_state);
	double slack_length = reference_length / (1.0 + strain);
    set_slack_length(slack_length);
};

void Blankevoort1991Ligament::setSlackLengthFromReferenceForce(
    double force, const SimTK::State& reference_state) {
   
    double transistion_force = get_transition_strain() * getLinearStiffnessForcePerStrain();

    double strain;
	if (force < 0.0) {
		strain = 0.0;
	}
	else if (force > transistion_force) {
		strain = force / get_linear_stiffness() + get_transition_strain() / 2;
	}
	else {
		strain = sqrt(2 * get_transition_strain() * force / get_linear_stiffness());
	}
    	
	setSlackLengthFromReferenceStrain(strain, reference_state);
};

void Blankevoort1991Ligament::setLinearStiffnessForcePerStrain(double linear_stiffness_strain) {
    double linear_stiffness_distance = linear_stiffness_strain / get_slack_length();
    set_linear_stiffness(linear_stiffness_distance);
};



//=============================================================================
// COMPUTATION
//=============================================================================
double Blankevoort1991Ligament::calcSpringForce(const SimTK::State& state) const 
{
    double strain = getStrain(state);
    double force_spring;
    //slack region
	if (strain <= 0) {
		force_spring = 0;
	}
    //toe region
	else if ((strain > 0) && (strain < (get_transition_strain()))) {	
        SimTK::Vector vec(1, strain);
        force_spring = _springToeForceFunction.calcValue(vec);        
	}
    //linear region 
	else if (strain >= get_transition_strain()) {		
        force_spring = _springLinearForceFunction.calcValue(SimTK::Vector(1,strain));
	}    
	return force_spring;
}

double Blankevoort1991Ligament::calcDampingForce(const SimTK::State& s) const {
    double strain = getStrain(s);
    double strain_rate = getStrainRate(s);
    double force_damping = 0.0;

    if (strain > 0) {
        double lin_stiff = getLinearStiffnessForcePerStrain();
        double damping_coeff = get_normalized_damping_coefficient();
	    force_damping = lin_stiff*damping_coeff*strain_rate;
    }
    else {
	    force_damping = 0.0;
    }

    //Phase-out damping as strain goes to zero with smooth-step function
    SimTK::Function::Step step(0, 1, 0, 0.01);
    SimTK::Vector in_vec(1,strain);
    force_damping = force_damping*step.calcValue(in_vec);

    setCacheVariableValue<double>(s, "force_damping", force_damping);
    return force_damping;
}

double Blankevoort1991Ligament::calcTotalForce(const SimTK::State& s) const
{	
	double force_total = getDampingForce(s) + getSpringForce(s);

    // make sure the ligament is only acting in tension
	if (force_total < 0.0) {
		force_total = 0.0;
	}

    setCacheVariableValue<double>(s, "force_total", force_total);
    return force_total;

}

void Blankevoort1991Ligament::computeForce(const SimTK::State& s,
							  SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
							  SimTK::Vector& generalizedForces) const
{
    if (get_appliesForce()) {
        // total force
        double force_total = getTotalForce(s);

        OpenSim::Array<PointForceDirection*> PFDs;
        get_GeometryPath().getPointForceDirections(s, &PFDs);

        for (int i = 0; i < PFDs.getSize(); i++) {
            applyForceToPoint(s, PFDs[i]->frame(), PFDs[i]->point(),
                force_total*PFDs[i]->direction(), bodyForces);
        }

        for (int i = 0; i < PFDs.getSize(); i++)
            delete PFDs[i];
    }
}

double Blankevoort1991Ligament::computePotentialEnergy(const SimTK::State& state) const {
	double strain = getCacheVariableValue<double>(state, "strain");
	double lin_stiff = getLinearStiffnessForcePerStrain();
	double trans_strain = get_transition_strain();
	double slack_len = get_slack_length();

	if (strain < trans_strain) {
		return 1 / 6 * lin_stiff / trans_strain*pow(strain, 3);
	}
	else {
		return 1 / 6 * lin_stiff / trans_strain*pow(trans_strain, 3)+1/2*lin_stiff*strain*(strain-trans_strain);
	}

}

double Blankevoort1991Ligament::computeMomentArm(const SimTK::State& s, Coordinate& aCoord) const
{
	return get_GeometryPath().computeMomentArm(s, aCoord);
}

//=============================================================================
// Reporting
//=============================================================================
	OpenSim::Array<std::string> Blankevoort1991Ligament::getRecordLabels() const {
		OpenSim::Array<std::string> labels("");
		labels.append(getName() + ".force_spring");
		labels.append(getName() + ".force_damping");
		labels.append(getName() + ".force_total");
		labels.append(getName() + ".length");
		labels.append(getName() + ".lengthening_speed");
		labels.append(getName() + ".strain");
		labels.append(getName() + ".strain_rate");
		return labels;
	}

	OpenSim::Array<double> Blankevoort1991Ligament::getRecordValues(const SimTK::State& s) const {
		OpenSim::Array<double> values(1);

		// Report values
		values.append(getSpringForce(s));
		values.append(getDampingForce(s));
		values.append(getTotalForce(s));
		values.append(getLength(s));
		values.append(getLengtheningSpeed(s));
		values.append(getStrain(s));
		values.append(getStrainRate(s));
		return values;
	}
