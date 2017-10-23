#pragma once
#include "FEMortarInterface.h"
#include "FEMortarContactSurface.h"

//-----------------------------------------------------------------------------
//! This class represents a surface used by the mortar contact interface.
class FEMortarTiedSurface : public FEMortarContactSurface
{
public:
	FEMortarTiedSurface(FEModel* pfem);

	//! Initializes data structures
	bool Init();

public:
	vector<vec3d>	m_L;		//!< Lagrange multipliers
};

//-----------------------------------------------------------------------------
//! This class implements a mortar based tied interface.
class FEMortarTiedContact : public FEMortarInterface
{
public:
	//! constructor
	FEMortarTiedContact(FEModel* pfem);

	//! return the master and slave surface
	FESurface* GetMasterSurface() { return &m_ms; }
	FESurface* GetSlaveSurface () { return &m_ss; }

public:
	//! temporary construct to determine if contact interface uses nodal integration rule (or facet)
	bool UseNodalIntegration() { return false; }

	//! interface activation
	void Activate();

	//! one-time initialization
	bool Init();

	//! calculate contact forces
	void Residual(FEGlobalVector& R, const FETimeInfo& tp);

	//! calculate contact stiffness
	void StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp);

	//! calculate Lagrangian augmentations
	bool Augment(int naug, const FETimeInfo& tp);

	//! serialize data to archive
	void Serialize(DumpStream& ar);

	//! build the matrix profile for use in the stiffness matrix
	void BuildMatrixProfile(FEGlobalMatrix& K);

	//! update interface data
	void Update(int niter, const FETimeInfo& tp);

private:
	double	m_atol;		//!< augmented Lagrangian tolerance
	double	m_eps;		//!< penalty factor
	int		m_naugmin;	//!< minimum number of augmentations
	int		m_naugmax;	//!< maximum number of augmentations

private:
	FEMortarTiedSurface	m_ms;	//!< mortar surface
	FEMortarTiedSurface	m_ss;	//!< non-mortar surface

	int		m_dofX;
	int		m_dofY;
	int		m_dofZ;

	DECLARE_PARAMETER_LIST();
};
