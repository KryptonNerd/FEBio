#include "StdAfx.h"
#include "FEDamageMooneyRivlin.h"

// register the material with the framework
REGISTER_MATERIAL(FEDamageMooneyRivlin, "damage Mooney-Rivlin");

// define the material parameters
BEGIN_PARAMETER_LIST(FEDamageMooneyRivlin, FEUncoupledMaterial)
	ADD_PARAMETER(c1, FE_PARAM_DOUBLE, "c1");
	ADD_PARAMETER(c2, FE_PARAM_DOUBLE, "c2");
	ADD_PARAMETER(m_beta, FE_PARAM_DOUBLE, "beta");
	ADD_PARAMETER(m_smin, FE_PARAM_DOUBLE, "smin");
	ADD_PARAMETER(m_smax, FE_PARAM_DOUBLE, "smax");
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
FEDamageMooneyRivlin::FEDamageMooneyRivlin(void)
{
	c1 = 0;
	c2 = 0;
	m_beta = 0.1;
	m_smin = 0.1635;
	m_smax = 0.2974;
}

//-----------------------------------------------------------------------------
void FEDamageMooneyRivlin::Init()
{
	if (c1 <= 0) throw MaterialError("c1 must be a positive number");
	if (c1 + c2 <= 0) throw MaterialError("c1 + c2 must be a positive number.");

	FEUncoupledMaterial::Init();
}

//-----------------------------------------------------------------------------
//! Calculate the deviatoric stress
mat3ds FEDamageMooneyRivlin::DevStress(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// determinant of deformation gradient
	double J = pt.J;

	// calculate deviatoric left Cauchy-Green tensor
	mat3ds B = pt.DevLeftCauchyGreen();

	// calculate square of B
	mat3ds B2 = B*B;

	// Invariants of B (= invariants of C)
	// Note that these are the invariants of Btilde, not of B!
	double I1 = B.tr();
	double I2 = 0.5*(I1*I1 - B2.tr());

	// --- TODO: put strain energy derivatives here ---
	//
	// W = C1*(I1 - 3) + C2*(I2 - 3)
	//
	// Wi = dW/dIi
	double W1 = c1;
	double W2 = c2;
	// ---

	// calculate T = F*dW/dC*Ft
	// T = F*dW/dC*Ft
	mat3ds T = B*(W1 + W2*I1) - B2*W2;

	// calculate damage reduction factor
	double g = Damage(mp);

	return T.dev()*(2.0*g/J);
}

//-----------------------------------------------------------------------------
//! Calculate the deviatoric tangent
tens4ds FEDamageMooneyRivlin::DevTangent(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// determinant of deformation gradient
	double J = pt.J;
	double Ji = 1.0/J;

	// calculate deviatoric left Cauchy-Green tensor: B = F*Ft
	mat3ds B = pt.DevLeftCauchyGreen();

	// calculate square of B
	mat3ds B2 = B*B;

	// Invariants of B (= invariants of C)
	double I1 = B.tr();
	double I2 = 0.5*(I1*I1 - B2.tr());

	// --- TODO: put strain energy derivatives here ---
	// Wi = dW/dIi
	double W1, W2;
	W1 = c1;
	W2 = c2;
	// ---

	// calculate dWdC:C
	double WC = W1*I1 + 2*W2*I2;

	// calculate C:d2WdCdC:C
	double CWWC = 2*I2*W2;

	// deviatoric cauchy-stress, trs = trace[s]/3
	mat3ds devs = pt.s.dev();

	// Identity tensor
	mat3ds I(1,1,1,0,0,0);

	tens4ds IxI = dyad1s(I);
	tens4ds I4  = dyad4s(I);
	tens4ds BxB = dyad1s(B);
	tens4ds B4  = dyad4s(B);

	// d2W/dCdC:C
	mat3ds WCCxC = B*(W2*I1) - B2*W2;

	tens4ds cw = (BxB - B4)*(W2*4.0*Ji) - dyad1s(WCCxC, I)*(4.0/3.0*Ji) + IxI*(4.0/9.0*Ji*CWWC);

	tens4ds c = dyad1s(devs, I)*(-2.0/3.0) + (I4 - IxI/3.0)*(4.0/3.0*Ji*WC) + cw;

	return c;
}


//-----------------------------------------------------------------------------
// Calculate damage reduction factor 
double FEDamageMooneyRivlin::Damage(FEMaterialPoint &mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// calculate right Cauchy-Green tensor
	mat3ds C = pt.RightCauchyGreen();
	mat3ds C2 = C*C;

	// Invariants
	double I1 = C.tr();
	double I2 = 0.5*(I1*I1 - C2.tr());

	// strain-energy value
	double SEF = c1*(I1 - 3) + c2*(I2 - 3);

	// get the damage material point data
	FEDamageMaterialPoint& dp = *mp.ExtractData<FEDamageMaterialPoint>();

	// calculate trial-damage parameter
	dp.m_Etrial = sqrt(2.0*fabs(SEF));

	// calculate damage parameter
	double Es = max(dp.m_Etrial, dp.m_Emax);

	// calculate reduction parameter
	double g = 1.0;
	if (Es < m_smin) g = 1.0;
	else if (Es > m_smax) g = 0.0;
	else 
	{
		double F = (Es - m_smin)/(m_smin - m_smax);
		g = 1.0 - (1.0 - m_beta + m_beta*F*F)*(F*F);
	}

	dp.m_D = g;
	return g;
}
