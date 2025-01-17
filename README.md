# OpenSim JAM

<img  height="300" src="graphics/msk_model_walk_full_body.png"><img  height="300" src="graphics/tka_ligament_elongation_walking.gif"><img  height="300" src="graphics/tka_contact.png">

_A framework to simulate **J**oint and **A**rticular **M**echanics in OpenSim._

_Author: Colin Smith_

This repository is a collection of force component plugins, models, and executables (tools) that are designed to enable OpenSim musculoskeletal simulations that include detailed joint mechanics. The project extends the opensim-core capabilities to enable joint representations that include 6 degree of freedom (DOF) joints (without kinematic constraints) and explicit representations of articular contact and ligament structures. 

This top level repository is used for the development and contains the [source code](src) for this package. Instructions to build the C++ code from source are provided [here](./building-opensim-jam-from-source.md).

Release versions of these codes are available for users to download from  the [OpenSim JAM SimTK website](https://simtk.org/projects/opensim-jam). This includes prebuilt versions of the opensim_jam.dll plugin for OpenSim, command-line exectuables for the tools, examples, and documentation. Browser friendly documentation and the bleeding edge version of the release folder can be found at [opensim-jam-release](opensim-jam-release).  
