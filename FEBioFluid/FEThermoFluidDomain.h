/*This file is part of the FEBio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/



#pragma once
#include <vector>
#include "febiofluid_api.h"
using namespace std;

class FEModel;
class FELinearSystem;
class FEBodyForce;
class FEFluidHeatSupply;
class FEGlobalVector;
class FETimeInfo;

//-----------------------------------------------------------------------------
//! Abstract interface class for thermofluid domains.

//! An thermofluid domain is used by the thermofluid mechanics solver.
//! This interface defines the functions that have to be implemented by a
//! thermofluid domain. There are basically two categories: residual functions
//! that contribute to the global residual vector. And stiffness matrix
//! function that calculate contributions to the global stiffness matrix.
class FEBIOFLUID_API FEThermoFluidDomain
{
public:
    FEThermoFluidDomain(FEModel* pfem);
    virtual ~FEThermoFluidDomain(){}
    
    // --- R E S I D U A L ---
    
    //! calculate the internal forces
    virtual void InternalForces(FEGlobalVector& R, const FETimeInfo& tp) = 0;
    
    //! Calculate the body force vector
    virtual void BodyForce(FEGlobalVector& R, const FETimeInfo& tp, FEBodyForce& bf) = 0;
    
    //! calculate the interial forces (for dynamic problems)
    virtual void InertialForces(FEGlobalVector& R, const FETimeInfo& tp) = 0;
    
    //! Calculate the heat supply
    virtual void HeatSupply(FEGlobalVector& R, const FETimeInfo& tp, FEFluidHeatSupply& r) = 0;
    
    // --- S T I F F N E S S   M A T R I X ---
    
    //! Calculate global stiffness matrix (only contribution from internal force derivative)
    //! \todo maybe I should rename this the InternalStiffness matrix?
    virtual void StiffnessMatrix (FELinearSystem& LS, const FETimeInfo& tp) = 0;
    
    //! Calculate stiffness contribution of body forces
    virtual void BodyForceStiffness(FELinearSystem& LS, const FETimeInfo& tp, FEBodyForce& bf) = 0;
    
    //! Calculate stiffness contribution of heat supplies
    virtual void HeatSupplyStiffness(FELinearSystem& LS, const FETimeInfo& tp, FEFluidHeatSupply& bf) = 0;
    
    //! calculate the mass matrix (for dynamic problems)
    virtual void MassMatrix(FELinearSystem& LS, const FETimeInfo& tp) = 0;
    
    //! transient analysis
    void SetTransientAnalysis() { m_btrans = true; }
    void SetSteadyStateAnalysis() { m_btrans = false; }
    
protected:
    bool        m_btrans;   // flag for transient (true) or steady-state (false) analysis
    double      m_Tr;       // referential absolute temperature
};
