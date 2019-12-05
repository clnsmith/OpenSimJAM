#ifndef OPENSIM_BLANKEVOORT_1991_LIGAMENT_H_
#define OPENSIM_BLANKEVOORT_1991_LIGAMENT_H_
/* -------------------------------------------------------------------------- *
 *                         Blankevoort1991Ligament.h                          *
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

#include <OpenSim/Simulation/Model/Force.h>
#include <OpenSim/Simulation/Model/GeometryPath.h>
#include <OpenSim/Common/LinearFunction.h>
#include <OpenSim/Common/PolynomialFunction.h>
#include "osimPluginDLL.h"

namespace OpenSim {

//==============================================================================
//                         Blankevoort1991LigamentModel
//==============================================================================
/**
This class implements a nonlinear spring ligament model as described in
Smith et al.\ (2016). This model is based on the formulation orginally 
proposed by Wismans et al.\ (1980) and Blankevoort et al.\ (1991). The ligament
is represented as a nonlinear spring with the force-strain relationship 
described by a quadratic "toe" region at low strains and a linear region at
high strains. Additionally, the ligament includes a normalized damping force.   
\image html fig_Blankevoort1991Ligament.svg

<B>Governing Equations</B>
Spring Force:
\f[
F_{spring} = 
\begin{Bmatrix}
0 & \epsilon < 0 \\
\frac{1}{2\epsilon_t }k\epsilon^2 & 0 \leq \epsilon \leq \epsilon_t \\  
 k(\epsilon - \frac{\epsilon_t}{2})& \epsilon > \epsilon_t
\end{Bmatrix}
\f]
Damping Force:
\f[
F_{damping} = k\dot{c}\epsilon 
\f]
Total Force:
\f[
F_{total} = F_{spring} + F_{damping}
\f]


This model has the following properties:
\li linear stiffness (k): The force/distance (e.g. N/m) stiffness of the linear
region of the ligament model.
\li slack_length (l_0): The resting length of the ligament.
\li normalized damping coefficient (c): Damping coefficient used in the damping
force calculation in units of seconds. Commonly set to 0.003.
\li transisiton_strain (e_t): The strain value where the ligament model 
transisitions from the quadratic toe region to the linear stiffness region.
Default value of 0.06 (6%) according to Blankevoort J Biomech Eng 1991.
This value is widely used in the multibody knee modeling literature:

Marra, J Biomech Eng, 2015
Guess, J Knee Surg, 2016
Smith, J Knee Surg, 2016
Li, J Biomech Eng, 1999

This value also agrees with some experimental studies:

Ristaniemi, A., Stenroth, L., Mikkonen, S., & Korhonen, R. K. (2018). 
Comparison of elastic, viscoelastic and failure tensile material properties of 
knee ligaments and patellar tendon. Journal of biomechanics, 79, 31-38.

Other literature suggests the transistion strain occurs in the region of 2% to
3% strain (0.02 - 0.03). 

Weiss, J. A., & Gardiner, J. C. (2001). Computational modeling of ligament 
mechanics. Critical Reviews in Biomedical Engineering, 29(3).

Martin, R. B., Burr, D. B., Sharkey, N. A., & Fyhrie, D. P. (2015). 
Mechanical properties of ligament and tendon. In Skeletal Tissue Mechanics 
(pp. 175-225). Springer, New York, NY.

In reality, the transisition strain appears to be dependent on the strain-rate:

Pioletti, D. P., Rakotomanana, L. R., Benvenuti, J. F., & Leyvraz, P. F. 
(1998). Viscoelastic constitutive law in large deformations: application to 
human knee ligaments and tendons. Journal of biomechanics, 31(8), 753-757.

Pioletti, D. P., Rakotomanana, L. R., & Leyvraz, P. F. (1999). Strain rate 
effect on the mechanical behavior of the anterior cruciate ligament�bone
complex. Medical Engineering & Physics, 21(2), 95-100.


  

This implementation is intended to be compatible with the common methods in
the literature to parameterize ligament properties. The slack_length property 
is saved in the model file, but this property can be set using 
setSlackLengthFromReferenceStrain() and setSlackLengthFromReferenceForce() as
well. If you want to compute the reference strain or reference force of the 
ligament at at reference state (pose), you can use the getStrain(state) and
getSpringForce(state) methods. 

The linear_stiffness is saved in 
units of force/distance but can be set and reported in units of force/strain.

Scaling the Blankevoort1991Ligament component adjusts the slack_length 
property after the scale factors are applied in a manner that keeps the strain 
in the ligament in the default model pose (reference strain) constant. The 
linear_stiffness parameter is not affected by scaling the model. 

<B>Reference</B>\n
	Blankevoort, L. and Huiskes, R., (1991).
	Ligament-bone interaction in a three-dimensional model of the knee.
	J Biomech Eng, 113(3), 263-269

	Smith, C.R., Lenhart, R.L., Kaiser, J., Vignos, M.F. and Thelen, D.G.,
	(2016). Influence of ligament properties on tibiofemoral mechanics
	in walking. J Knee Surg, 29(02), 99-106.\n
	
	Wismans, J.A.C., Veldpaus, F., Janssen, J., Huson, A. and Struben, P., 
	(1980). A three-dimensional mathematical model of the knee-joint. 
	J Biomech, 13(8), 677-685.\n

    @author Colin Smith

*/

//class OSIMSIMULATION_API Blankevoort1991Ligament : public Force  {
class OSIMPLUGIN_API Blankevoort1991Ligament : public Force  {
	OpenSim_DECLARE_CONCRETE_OBJECT(Blankevoort1991Ligament, Force)

	public:
	//=========================================================================
	// PROPERTIES
	//=========================================================================

        OpenSim_DECLARE_UNNAMED_PROPERTY(GeometryPath,
        "The set of points defining the path of the ligament")
        OpenSim_DECLARE_PROPERTY(linear_stiffness, double,
            "Slope of the linear portion of the force-strain curve of "
            "ligament. Units of force/length.")
        OpenSim_DECLARE_PROPERTY(transition_strain, double,
            "Strain at which ligament force-strain curve transitions from"
            "quadratic to linear. Default value of 0.06.")
        OpenSim_DECLARE_PROPERTY(normalized_damping_coefficient, double,
            "Coefficient for normalized damping of ligament."
            "Units of seconds, default value of 0.003.")
        OpenSim_DECLARE_PROPERTY(slack_length, double,
            "Length at which ligament begins developing tension")

        //=====================================================================
        // OUTPUTS
        //=====================================================================
        
        OpenSim_DECLARE_OUTPUT(force_spring, double, getSpringForce, 
            SimTK::Stage::Position);
        OpenSim_DECLARE_OUTPUT(force_damping, double, getDampingForce, 
            SimTK::Stage::Velocity);
        OpenSim_DECLARE_OUTPUT(force_total, double, getTotalForce,
            SimTK::Stage::Velocity);
        OpenSim_DECLARE_OUTPUT(strain, double, getStrain,
            SimTK::Stage::Position);
        OpenSim_DECLARE_OUTPUT(strain_rate, double, getStrainRate,
            SimTK::Stage::Velocity);
        OpenSim_DECLARE_OUTPUT(length, double, getLength,
            SimTK::Stage::Position);
        OpenSim_DECLARE_OUTPUT(lengthening_rate, double, getLengtheningSpeed,
            SimTK::Stage::Velocity);
	


	//=========================================================================
	// METHODS
	//=========================================================================
	public:
		//Constructors
		Blankevoort1991Ligament();

		Blankevoort1991Ligament(std::string name, 
            const PhysicalFrame& frame1, SimTK::Vec3 point1,
            const PhysicalFrame& frame2, SimTK::Vec3 point2);

		Blankevoort1991Ligament(std::string name, 
            const PhysicalFrame& frame1, SimTK::Vec3 point1,
			const PhysicalFrame& frame2, SimTK::Vec3 point2,
			double linear_stiffness, double slack_lnegth);

		//---------------------------------------------------------------------
		// SET
		//---------------------------------------------------------------------
        /**
	     * Set the slack length property from the strain in the ligament at a 
         * known pose (reference state)
	     */
		void setSlackLengthFromReferenceStrain(
            double strain, const SimTK::State& reference_state);
        /**
	     * Set the slack length property from the force in the ligament at a 
         * known pose (reference state)
	     */
		void setSlackLengthFromReferenceForce(
            double force, const SimTK::State& reference_state);
         /**
	     * Set the linear_stiffness property using a value in units of 
         * force/strain 
	     */
        void setLinearStiffnessForcePerStrain(double linear_stiffness);

		//--------------------------------------------------------------------------
		// GET
		//--------------------------------------------------------------------------
		double getTotalForce(const SimTK::State& state) const;
        double getStrain(const SimTK::State& state) const;
        double getStrainRate(const SimTK::State& state) const;
        double getLength(const SimTK::State& state) const;
        double getLengtheningSpeed(const SimTK::State& state) const;
        double getSpringForce(const SimTK::State& state) const;
        double getDampingForce(const SimTK::State& state) const;

        double getLinearStiffnessForcePerStrain() const;

		//--------------------------------------------------------------------------
		// COMPUTATIONS
		//--------------------------------------------------------------------------
		double computeMomentArm(
            const SimTK::State& s, Coordinate& aCoord) const;

		void computeForce(const SimTK::State& s, 
            SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
			SimTK::Vector& generalizedForces) const override;

		double computePotentialEnergy(
            const SimTK::State& state) const override;
				
		

		
		//--------------------------------------------------------------------------
		// SCALE
		//--------------------------------------------------------------------------
        
		void extendPostScale(const SimTK::State& s, const ScaleSet& scaleSet) override;

		//-----------------------------------------------------------------------------
		// REPORTING
		//-----------------------------------------------------------------------------
		OpenSim::Array<std::string> getRecordLabels() const override;
		OpenSim::Array<double> getRecordValues(const SimTK::State& state) const override;
		
    protected:

		void extendFinalizeFromProperties() override;
		void extendAddToSystem(SimTK::MultibodySystem& system) const override;

        double calcSpringForce(const SimTK::State& state) const;
		double calcDampingForce(const SimTK::State& state) const;
        double calcTotalForce(const SimTK::State& state) const;
	private:
		void setNull();
		void constructProperties();

    private:
        PolynomialFunction _springToeForceFunction;
        LinearFunction _springLinearForceFunction;

//=============================================================================
	};	// END of class Blankevoort1991Ligament
//=============================================================================
//=============================================================================

} // end of namespace OpenSim

#endif // OPENSIM_BLANKEVOORT_1991_LIGAMENT_H_
