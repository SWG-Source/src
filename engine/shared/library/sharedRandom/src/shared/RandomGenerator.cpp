// ======================================================================
//
// RandomGenerator.h
// By Eric Sebesta
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedRandom/FirstSharedRandom.h"
#include "sharedRandom/RandomGenerator.h"

#include "sharedRandom/Random.h"

// ======================================================================

const real RandomGenerator::AM    = CONST_REAL(1.0/IM);
const real RandomGenerator::NDIV  = CONST_REAL(1+(IM-1)/static_cast<real>(NTAB));
const real RandomGenerator::EPS   = CONST_REAL(1.2e-7);
const real RandomGenerator::RNMX  = CONST_REAL(1.0-EPS);

// ======================================================================
// Construct a RandomGenerator
//
// Remarks:
//
//   Sets seed based on the global Random::random() call (i.e different for every RandomGenerator).

RandomGenerator::RandomGenerator (void)
:	idnum (-Random::random()+1), //+1 so that our seed is different from the global one
	iy    (0)
{
	memset(iv, 0, sizeof(iv));
}

// ----------------------------------------------------------------------

int32 RandomGenerator::randomNumber (void)
{
// This random algorithm comes from ran1() in Numerical Recipes in C, p. 280

	int32 j;
	int32 k;

	if(idnum <= 0 || !iy)
	{
		if(-(idnum) < 1)                      //initialize
			idnum = 1;
		else
			idnum = -(idnum);
		for (j = NTAB+7; j >= 0; --j)         //use idnum as the "seed value"
		{
			k = (idnum)/IQ;
			idnum = IA * (idnum-k *IQ) - IR*k;
			if (idnum < 0)
				idnum += IM;
			if (j < NTAB)
				iv[j] = idnum;
		}
		iy=iv[0];
	}
	k = (idnum) / IQ;                       //start here when not initializing
	idnum = IA*(idnum-k*IQ)-IR*k;           //generate random number (Schrage's Method)
	if (idnum < 0)
		idnum += IM;
	j = static_cast<int32>(iy/NDIV);        //put in range 0...NTAB-1
	iy = iv[j];                             //iy is our new random number
	iv[j] = idnum;
	return iy;
}

// ======================================================================
