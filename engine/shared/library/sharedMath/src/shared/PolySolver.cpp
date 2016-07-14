// Polynomial solver code adapted from Graphics Gems version
// by Jochen Schwarze

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/PolySolver.h"

#include    <math.h>
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

double cuberoot( double x )
{
	return ((x) > 0.0 ? pow(x, 1.0/3.0) : ((x) < 0.0 ? -pow(-x, 1.0/3.0) : 0.0));
}

int PolySolver::solveQuadratic( double const c[3], double s[2] )
{
    double p, q, D;

    /* normal form: x^2 + px + q = 0 */

    p = c[ 1 ] / (2 * c[ 2 ]);
    q = c[ 0 ] / c[ 2 ];

    D = p * p - q;

    if (D < 0)
    {
		return 0;
    }
    else
    {
		double sqrt_D = sqrt(D);

		s[ 0 ] =   sqrt_D - p;
		s[ 1 ] = - sqrt_D - p;
		return 2;
    }
}

int PolySolver::solveCubic( double const c[4], double s[3] )
{
    int     i, num;
    double  sub;
    double  A, B, C;
    double  sq_A, p, q;
    double  cb_p, D;

    /* normal form: x^3 + Ax^2 + Bx + C = 0 */

    A = c[ 2 ] / c[ 3 ];
    B = c[ 1 ] / c[ 3 ];
    C = c[ 0 ] / c[ 3 ];

    /*  substitute x = y - A/3 to eliminate quadric term:
	x^3 +px + q = 0 */

    sq_A = A * A;
    p = (1.0/3) * (- (1.0/3) * sq_A + B);
    q = (1.0/2) * (((2.0/27) * A * sq_A - ((1.0/3) * A * B)) + C);

    /* use Cardano's formula */

    cb_p = p * p * p;
    D = q * q + cb_p;

    if (D < 0) /* Casus irreducibilis: three real solutions */
    {
		double phi = (1.0/3) * acos(-q / sqrt(-cb_p));
		double t = 2 * sqrt(-p);

		s[ 0 ] =   t * cos(phi);
		s[ 1 ] = - t * cos(phi + M_PI / 3);
		s[ 2 ] = - t * cos(phi - M_PI / 3);
		num = 3;
    }
    else /* one real solution */
    {
		double sqrt_D = sqrt(D);
		double u = cuberoot(sqrt_D - q);
		double v = - cuberoot(sqrt_D + q);

		s[ 0 ] = u + v;
		num = 1;
    }

    /* resubstitute */

    sub = (1.0/3) * A;

    for (i = 0; i < num; ++i)
    	s[ i ] -= sub;

    return num;
}

double cubicError = 0.0f;
double cleanedCubicError = 0.0f;

double quarticError = 0.0f;
double cleanedQuarticError = 0.0f;

double evaluateCubic( double x, const double c[4] )
{
	return ((x*c[3] + c[2]) * x + c[1]) * x + c[0];
}

double evaluateCubicDerivative ( double x, const double c[4] )
{
	return (3.0*x + 2.0*c[2]) * x + c[1];
}

double cleanCubicRoot( double x, const double c[4] )
{
	double e;

	e = evaluateCubic(x,c);

	if(fabs(e) > cubicError) cubicError = e;

	// ----------

//	for(int i = 0; i < 10; i++)
	{
		e = evaluateCubic(x,c);

		e *= 0.8;

		double d = evaluateCubicDerivative(x,c);

		if(d != 0.0)
		{
			x = x - e/d;
		}
	}

	// ----------
	
	e = evaluateCubic(x,c);

	if(fabs(e) > cleanedCubicError) cleanedCubicError = e;

	return x;
}

double evaluateQuartic( double x, const double c[5] )
{
	return (((x*c[4] + c[3]) * x + c[2]) * x + c[1]) * x + c[0];
}

double evaluateQuarticDerivative ( double x, const double c[5] )
{
	return ((4.0*x*c[4] + 3.0*c[3]) * x + 2.0*c[2]) * x + c[1];
}

double cleanQuarticRoot( double x, const double c[5] )
{
	double e;

	e = evaluateQuartic(x,c);

	if(fabs(e) > quarticError) quarticError = e;

	// ----------

//	for(int i = 0; i < 10; i++)
	{
		e = evaluateQuartic(x,c);

		e *= 0.8;

		double d = evaluateQuarticDerivative(x,c);

		if(d != 0.0)
		{
			x = x - e/d;
		}
	}

	// ----------

	e = evaluateQuartic(x,c);

	if(fabs(e) > cleanedQuarticError) cleanedQuarticError = e;

	return x;
}

#ifdef WIN32
	#define isnan(a) _isnan(a)
#endif

int PolySolver::solveQuartic( const double c[5], double s[4] )
{
	double a3 = c[3] / c[4];
	double a2 = c[2] / c[4];
	double a1 = c[1] / c[4];
	double a0 = c[0] / c[4];

	// ----------
	// solve the resolvent cubic to get a real root

	double y1 = 1.0f;

	{
		double c[4];

		c[3] = 1.0;
		c[2] = -a2;
		c[1] = (a1*a3) - (4.0)*(a0);
		c[0] = (4.0)*(a2*a0) - (a1*a1) - (a3*a3*a0);

		double s[3];

		int nRoots = PolySolver::solveCubic(c,s);

		for(int i = 0; i < nRoots; i++)
		{
			if(s[i] == s[i])
			{
				// root is real

				y1 = cleanCubicRoot( s[i], c );

				break;
			}
		}
	}

	// ----------
	// use the root to find the roots of the quadric
	
	double t1 = (1.0/4.0)*(a3*a3) - a2 + y1;

	double R = sqrt(t1);

	double D;

	if(R == 0.0)
	{
		double t1 = (y1*y1) - (4.0)*(a0);

		double t2 = sqrt(t1);

		double t3 = (3.0/4.0)*(a3*a3) - (2.0)*(a2) + (2.0)*t2;

		D = sqrt(t3);
	}
	else
	{
		double t1 = (4.0)*(a3*a2) - (8.0)*(a1) - (a3*a3*a3);

		double t2 = t1 / (4.0 * R);

		double t3 = (3.0/4.0)*(a3*a3) - (R*R) - (2.0)*(a2) + t2;

		D = sqrt(t3);
	}
	
	double E;

	if(R == 0.0)
	{
		double t1 = (y1*y1) - (4.0)*(a0);

		double t2 = sqrt(t1);

		double t3 = (3.0/4.0)*(a3*a3) - (2.0)*(a2) - (2.0)*(t2);

		E = sqrt(t3);
	}
	else
	{
		double t1 = (4.0)*(a3*a2) - (8.0)*(a1) - (a3*a3*a3);

		double t2 = t1 / (4.0 * R);

		double t3 = (3.0/4.0)*(a3*a3) - (R*R) - (2.0)*(a2) - t2;

		E = sqrt(t3);
	}

	static const double nan = sqrt(-1.0f);
	if (std::isnan(D))
	{
		s[0] = nan;
		s[1] = nan;
	}
	else
	{
		s[0] = (-1.0/4.0)*a3 + (1.0/2.0)*R + (1.0/2.0)*D;
		s[1] = (-1.0/4.0)*a3 + (1.0/2.0)*R - (1.0/2.0)*D;
	}

	if (std::isnan(E))
	{
		s[2] = nan;
		s[3] = nan;
	}
	else
	{
		s[2] = (-1.0/4.0)*a3 - (1.0/2.0)*R + (1.0/2.0)*E;
		s[3] = (-1.0/4.0)*a3 - (1.0/2.0)*R - (1.0/2.0)*E;
	}

/*
	// Perform one step of a Newton iteration in order to minimize round-off errors

	int i;

	for(i = 0; i < 4; i++)
	{
		s[i] = cleanQuarticRoot(s[i],c);
	}
	*/

	return 4;
}
