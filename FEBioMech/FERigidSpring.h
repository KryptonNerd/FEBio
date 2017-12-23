#pragma once
#include "FECore/vec3d.h"
#include "FERigidConnector.h"

//-----------------------------------------------------------------------------
//! The FERigidSpring class implements a linear spring that connects
//! two rigid bodies at arbitrary points (not necessarily nodes).

class FERigidSpring : public FERigidConnector
{
public:
    //! constructor
    FERigidSpring(FEModel* pfem);
    
    //! destructor
    ~FERigidSpring() {}
    
    //! initialization
    bool Init() override;
    
    //! calculates the joint forces
    void Residual(FEGlobalVector& R, const FETimeInfo& tp) override;
    
    //! calculates the joint stiffness
    void StiffnessMatrix(FESolver* psolver, const FETimeInfo& tp) override;
    
    //! calculate Lagrangian augmentation
    bool Augment(int naug, const FETimeInfo& tp) override;
    
    //! serialize data to archive
    void Serialize(DumpStream& ar) override;
    
    //! update state
	void Update(int niter, const FETimeInfo& tp) override;
    
    //! Reset data
    void Reset() override;
    
public: // parameters
    vec3d	m_a0;       //! initial absolute position vector of spring on body A
    vec3d	m_b0;       //! initial absolute position vector of spring on body B
    double	m_k;        //! spring constant

protected:
    double  m_L0;       //! spring free length
	vec3d	m_qa0;      //! initial relative position vector of spring on body A
    vec3d	m_qb0;      //! initial relative position vector of spring on body B
    
    DECLARE_PARAMETER_LIST();
};
