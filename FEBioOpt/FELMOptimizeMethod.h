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
#include "FEOptimizeMethod.h"
#include <FECore/matrix.h>
#include <vector>
using namespace std;

//----------------------------------------------------------------------------
//! Optimization method using Levenberg-Marquardt method
class FELMOptimizeMethod : public FEOptimizeMethod
{
public:
	FELMOptimizeMethod();
	bool Solve(FEOptimizeData* pOpt, vector<double>& amin, vector<double>& ymin, double* minObj) override;

protected:
	FEOptimizeData* m_pOpt;

	void ObjFun(vector<double>& x, vector<double>& a, vector<double>& y, matrix& dyda);

	static FELMOptimizeMethod* m_pThis;
	static void objfun(vector<double>& x, vector<double>& a, vector<double>& y, matrix& dyda) { return m_pThis->ObjFun(x, a, y, dyda); }

public:
	double			m_objtol;	// objective tolerance
	double			m_fdiff;	// forward difference step size
	int				m_nmax;		// maximum number of iterations
	bool			m_bcov;		// flag to print covariant matrix

protected:
	vector<double>	m_yopt;	// optimal y-values

	DECLARE_FECORE_CLASS();
};
