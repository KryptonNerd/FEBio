#include "stdafx.h"
#include "FEDiscreteSpringDomain.h"
#include "FECore/DOFS.h"

BEGIN_PARAMETER_LIST(FEDiscreteSpringDomain, FEDiscreteDomain)
	ADD_PARAMETER(m_keps, FE_PARAM_DOUBLE, "k_eps");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEDiscreteSpringDomain::FEDiscreteSpringDomain(FEModel* pfem) : FEDiscreteDomain(&pfem->GetMesh())
{
	m_pMat = 0;
	m_keps = 0.0;
}

//-----------------------------------------------------------------------------
void FEDiscreteSpringDomain::SetMaterial(FEMaterial* pmat)
{
	m_pMat = dynamic_cast<FESpringMaterial*>(pmat);
	assert(m_pMat);
}

//-----------------------------------------------------------------------------
void FEDiscreteSpringDomain::UnpackLM(FEElement &el, vector<int>& lm)
{
	int N = el.Nodes();
	lm.resize(N*6);
	for (int i=0; i<N; ++i)
	{
		FENode& node = m_pMesh->Node(el.m_node[i]);
		vector<int>& id = node.m_ID;

		// first the displacement dofs
		lm[3*i  ] = id[DOF_X];
		lm[3*i+1] = id[DOF_Y];
		lm[3*i+2] = id[DOF_Z];

		// rigid rotational dofs
		lm[3*N + 3*i  ] = id[DOF_RU];
		lm[3*N + 3*i+1] = id[DOF_RV];
		lm[3*N + 3*i+2] = id[DOF_RW];
	}
}

//-----------------------------------------------------------------------------
void FEDiscreteSpringDomain::Activate()
{
	for (int i=0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		if (node.m_bexclude == false)
		{
			if (node.m_rid < 0)
			{
				node.m_ID[DOF_X] = DOF_ACTIVE;
				node.m_ID[DOF_Y] = DOF_ACTIVE;
				node.m_ID[DOF_Z] = DOF_ACTIVE;
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! Calculates the forces due to discrete elements (i.e. springs)

void FEDiscreteSpringDomain::InternalForces(FEGlobalVector& R)
{
	FEMesh& mesh = *m_pMesh;

	vector<double> fe(6);
	vec3d u1, u2;

	vector<int> en(2), lm(6);

	for (size_t i=0; i<m_Elem.size(); ++i)
	{
		// get the discrete element
		FEDiscreteElement& el = m_Elem[i];

		// get the nodes
		FENode& n1 = mesh.Node(el.m_node[0]);
		FENode& n2 = mesh.Node(el.m_node[1]);

		// get the nodal positions
		vec3d& r01 = n1.m_r0;
		vec3d& r02 = n2.m_r0;
		vec3d& rt1 = n1.m_rt;
		vec3d& rt2 = n2.m_rt;

		vec3d e = rt2 - rt1; e.unit();

		// calculate spring lengths
		double L0 = (r02 - r01).norm();
		double Lt = (rt2 - rt1).norm();
		double DL = Lt - L0;
		
		// evaluate the spring force
		double F = m_pMat->force(DL);

		// set up the force vector
		fe[0] =   F*e.x;
		fe[1] =   F*e.y;
		fe[2] =   F*e.z;
		fe[3] =  -F*e.x;
		fe[4] =  -F*e.y;
		fe[5] =  -F*e.z;

		// setup the node vector
		en[0] = el.m_node[0];
		en[1] = el.m_node[1];

		// set up the LM vector
		UnpackLM(el, lm);

		// assemble element
		R.Assemble(en, lm, fe);
	}

	if (m_keps > 0)
	{
		double eps = m_keps;
		lm.resize(3);
		en.resize(1);
		fe.resize(3);
		int NN = Nodes();
		for (int i=1; i<NN-1; ++i)
		{
			int i0 = i - 1;
			int i1 = i + 1;

			vec3d xi = Node(i).m_rt;
			vec3d x0 = Node(i0).m_rt;
			vec3d x1 = Node(i1).m_rt;

			vec3d r = xi - x0;
			vec3d s = x1 - x0; s.unit();
			vec3d d = r - s*(r*s);

			fe[0] = -eps*d.x;
			fe[1] = -eps*d.y;
			fe[2] = -eps*d.z;

			en[0] = m_Node[i];
			lm[0] = Node(i).m_ID[DOF_X];
			lm[1] = Node(i).m_ID[DOF_Y];
			lm[2] = Node(i).m_ID[DOF_Z];
			R.Assemble(en, lm, fe);
		}
	}
}

//-----------------------------------------------------------------------------
//! Calculates the discrete element stiffness

void FEDiscreteSpringDomain::StiffnessMatrix(FESolver* psolver)
{
	FEMesh& mesh = *m_pMesh;

	matrix ke(6,6);
	ke.zero();

	vector<int> en(2), lm(6);

	// loop over all discrete elements
	for (size_t i=0; i<m_Elem.size(); ++i)
	{
		// get the discrete element
		FEDiscreteElement& el = m_Elem[i];

		// get the nodes of the element
		FENode& n1 = mesh.Node(el.m_node[0]);
		FENode& n2 = mesh.Node(el.m_node[1]);

		// get the nodal positions
		vec3d& r01 = n1.m_r0;
		vec3d& r02 = n2.m_r0;
		vec3d& rt1 = n1.m_rt;
		vec3d& rt2 = n2.m_rt;

		vec3d e = rt2 - rt1; 

		// calculate nodal displacements
		vec3d u1 = rt1 - r01;
		vec3d u2 = rt2 - r02;

		// calculate spring lengths
		double L0 = (r02 - r01).norm();
		double Lt = (rt2 - rt1).norm();
		double DL = Lt - L0;

		// evaluate the stiffness
		double F = m_pMat->force(DL);
		double E = m_pMat->stiffness(DL);

		if (Lt == 0) { F = 0; Lt = 1; e = vec3d(1,1,1); }

		double A[3][3] = {0};
		A[0][0] = ((E - F/Lt)*e.x*e.x + F/Lt);
		A[1][1] = ((E - F/Lt)*e.y*e.y + F/Lt);
		A[2][2] = ((E - F/Lt)*e.z*e.z + F/Lt);

		A[0][1] = A[1][0] = (E - F/Lt)*e.x*e.y;
		A[1][2] = A[2][1] = (E - F/Lt)*e.y*e.z;
		A[0][2] = A[2][0] = (E - F/Lt)*e.x*e.z;

		ke[0][0] = A[0][0]; ke[0][1] = A[0][1]; ke[0][2] = A[0][2];
		ke[1][0] = A[1][0]; ke[1][1] = A[1][1]; ke[1][2] = A[1][2];
		ke[2][0] = A[2][0]; ke[2][1] = A[2][1]; ke[2][2] = A[2][2];

		ke[0][3] = -A[0][0]; ke[0][4] = -A[0][1]; ke[0][5] = -A[0][2];
		ke[1][3] = -A[1][0]; ke[1][4] = -A[1][1]; ke[1][5] = -A[1][2];
		ke[2][3] = -A[2][0]; ke[2][4] = -A[2][1]; ke[2][5] = -A[2][2];

		ke[3][0] = -A[0][0]; ke[3][1] = -A[0][1]; ke[3][2] = -A[0][2];
		ke[4][0] = -A[1][0]; ke[4][1] = -A[1][1]; ke[4][2] = -A[1][2];
		ke[5][0] = -A[2][0]; ke[5][1] = -A[2][1]; ke[5][2] = -A[2][2];

		ke[3][3] = A[0][0]; ke[3][4] = A[0][1]; ke[3][5] = A[0][2];
		ke[4][3] = A[1][0]; ke[4][4] = A[1][1]; ke[4][5] = A[1][2];
		ke[5][3] = A[2][0]; ke[5][4] = A[2][1]; ke[5][5] = A[2][2];

		// setup the node vector
		en[0] = el.m_node[0];
		en[1] = el.m_node[1];

		// set up the LM vector
		UnpackLM(el, lm);

		// assemble the element into the global system
		psolver->AssembleStiffness(en, lm, ke);
	}

	// Add Bending stiffness
	if (m_keps > 0)
	{
		double eps = m_keps;
		vector<int> lmi(3);
		vector<int>	lmj(9);
		en.resize(3);
		int NN = Nodes();
		for (int i=1; i<NN-1; ++i)
		{
			int i0 = i - 1;
			int i1 = i + 1;

			vec3d xi = Node(i).m_rt;
			vec3d x0 = Node(i0).m_rt;
			vec3d x1 = Node(i1).m_rt;

			vec3d r = xi - x0*0.5 - x1*0.5;
			vec3d s = x1 - x0; 
			double L = s.unit();
			double c = (r*s)*(-eps/L);

			mat3ds SxS = dyad(s);
			mat3ds K = (mat3dd(1.0) - SxS)*(eps);

			ke.resize(3, 9);
			ke.zero();
			ke[0][0] = eps; ke[0][3] = -0.5*eps; ke[0][6] = -0.5*eps;
			ke[1][1] = eps; ke[1][4] = -0.5*eps; ke[1][7] = -0.5*eps;
			ke[2][2] = eps; ke[2][5] = -0.5*eps; ke[2][8] = -0.5*eps;

			vector<int>& IDi = Node(i ).m_ID;
			vector<int>& ID0 = Node(i0).m_ID;
			vector<int>& ID1 = Node(i1).m_ID;

			lmi[0] = IDi[DOF_X];
			lmi[1] = IDi[DOF_Y];
			lmi[2] = IDi[DOF_Z];

			lmj[0] = IDi[DOF_X];
			lmj[1] = IDi[DOF_Y];
			lmj[2] = IDi[DOF_Z];
			lmj[3] = ID0[DOF_X];
			lmj[4] = ID0[DOF_Y];
			lmj[5] = ID0[DOF_Z];
			lmj[6] = ID1[DOF_X];
			lmj[7] = ID1[DOF_Y];
			lmj[8] = ID1[DOF_Z];
			psolver->AssembleStiffness2(lmi, lmj, ke);
		}
	}
}
