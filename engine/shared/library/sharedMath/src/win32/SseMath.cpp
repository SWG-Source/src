// ======================================================================
//
// SseMath.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/SseMath.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

// ======================================================================

#define SSE_ALIGN  __declspec(align(16))
#define SSE_VARIABLE_COUNT 5

// ======================================================================

namespace
{
	SSE_ALIGN float sseVariable[SSE_VARIABLE_COUNT][4];
}

// ======================================================================
/**
 * Retrieve whether the hardware can do SSE math.
 *
 * @return  true if SSE math processing is available; false otherwise.
 */

bool SseMath::canDoSseMath()
{
#if 0
	return false;
#else
	//-- First check the CPUID instruction.  If it raises an exception,
	//   the CPUID instruction doesn't exist and SSE math support is not available.
	bool    cpuHasSse              = false;
	bool    cpuHasSaveRestore      = false;

	uint32  featureBits;

#if 0
	bool    osSupportsSaveRestore  = false;
	bool    osSimdExceptionSupport = false;
	bool    x87EmulationDisabled   = false;

	uint32  controlRegister4;
	uint32  controlRegister0;
#endif

	try
	{
		__asm {
			//-- Get features bits.
			mov    eax, 1
			cpuid

			mov    featureBits, edx

#if 0
			//-- Get control registers.
			mov    ecx, CR4
			mov    controlRegister4, ecx

			mov    ecx, CR0
			mov    controlRegister0, ecx
#endif
		}

		cpuHasSse              = ((featureBits      & 0x02000000) != 0);    //lint !e530 // featureBits not initialized - yes it is, in the asm
		cpuHasSaveRestore      = ((featureBits      & 0x01000000) != 0);

#if 0
		osSupportsSaveRestore  = ((controlRegister4 & 0x00000200) != 0);
		osSimdExceptionSupport = ((controlRegister4 & 0x00000400) != 0);
		x87EmulationDisabled   = ((controlRegister0 & 0x00000004) == 0);
#endif
	}
	catch (...)
    { //lint !e1775 // catch block does not catch any declared exceptions
	}
	
#if 1
	return cpuHasSse && cpuHasSaveRestore;
#else
	return cpuHasSse && cpuHasSaveRestore && osSupportsSaveRestore && osSimdExceptionSupport && x87EmulationDisabled;
#endif
#endif
}

// ----------------------------------------------------------------------

Vector SseMath::rotateTranslateScale_l2p(const Transform &transform, const Vector &vector, float scale)
{
	// NOTE: technically, my xmm register data contents comments are listing items in reverse order from how INTEL docs list them, left most val is really least significant value.

	__asm {
		//-- Keep track of ebx.  Client is crashing if I trash this.
		push    ebx

		//-- Load up matrix.
		mov     ebx, transform

		movaps  xmm0, [ebx + 0]  // xmm0 = a1 a2 a3 a4
		movaps  xmm1, [ebx + 16] // xmm1 = b1 b2 b3 b4
		movaps  xmm2, [ebx + 32] // xmm2 = c1 c2 c3 c4
	}

	//-- Prepare source vector.
	sseVariable[0][0] = vector.x;
	sseVariable[0][1] = vector.y;
	sseVariable[0][2] = vector.z;
	sseVariable[0][3] = 1.0f;

	__asm {
		//-- Load up the source vector.
		mov     ebx, offset sseVariable

		movaps  xmm3, [ebx]                 // xmm3 = sx sy sz 1

		//-- Copy source to workspaces.
		movaps  xmm4, xmm3                  // xmm4 = sx sy sz 1
		movaps  xmm5, xmm3                  // xmm5 = sx sy sz 1
	}

	//-- Prepare the scale vector.
	__asm {
		movss   xmm6, scale                 // xmm6 = scale  ?      ?      ?
		shufps  xmm6, xmm6, 0x00            // xmm6 = scale  scale  
		movlhps xmm6, xmm6                  // xmm6 = scale  scale  scale  scale

		//-- Do the transform multiplies.
		mulps   xmm3, xmm0                  // xmm3 = a1*sx  a2*sy  a3*sz  a4
		mulps   xmm4, xmm1                  // xmm4 = b1*sx  b2*sy  b3*sz  b4
		mulps   xmm5, xmm2                  // xmm5 = c1*sx  c2*sy  c3*sz  c4

		//-- Do the scale multiplies.
		mulps   xmm3, xmm6                  // xmm3 = a1*sx*scale a2*sy*scale a3*sz*scale a4*scale
		mulps   xmm4, xmm6                  // xmm4 = b1*sx*scale b2*sy*scale b3*sz*scale b4*scale
		mulps   xmm5, xmm6                  // xmm5 = c1*sx*scale c2*sy*scale c3*sz*scale c4*scale

		//-- Save out data.
		movaps  [ebx + 32], xmm3
		movaps  [ebx + 48], xmm4
		movaps  [ebx + 64], xmm5

		//-- Restore EBX
		pop     ebx
	}

	// @todo consider twizzling/detwizzling to be able to perform add in sse.
	return Vector(
		sseVariable[2][0] + sseVariable[2][1] + sseVariable[2][2] + sseVariable[2][3],
		sseVariable[3][0] + sseVariable[3][1] + sseVariable[3][2] + sseVariable[3][3],
		sseVariable[4][0] + sseVariable[4][1] + sseVariable[4][2] + sseVariable[4][3]);
} //lint !e715 // scale/transform not referenced - it's in the asm

// ----------------------------------------------------------------------

Vector SseMath::rotateScale_l2p(const Transform &transform, const Vector &vector, float scale)
{
#if 0
	// @todo do the real thing.
	return transform.rotate_l2p(vector) * scale;
#else

	// NOTE: technically, my xmm register data contents comments are listing items in reverse order from how INTEL docs list them, left most val is really least significant value.

	__asm {
		//-- Keep track of ebx.  Client is crashing if I trash this.
		push    ebx

		//-- Load up matrix.
		mov     ebx, transform

		movaps  xmm0, [ebx + 0]  // xmm0 = a1 a2 a3 a4
		movaps  xmm1, [ebx + 16] // xmm1 = b1 b2 b3 b4
		movaps  xmm2, [ebx + 32] // xmm2 = c1 c2 c3 c4
	}

	//-- Prepare source vector.
	sseVariable[0][0] = vector.x;
	sseVariable[0][1] = vector.y;
	sseVariable[0][2] = vector.z;
	sseVariable[0][3] = 0.0f;

	__asm {
		//-- Load up the source vector.
		mov     ebx, offset sseVariable

		movaps  xmm3, [ebx]                 // xmm3 = sx sy sz 1

		//-- Copy source to workspaces.
		movaps  xmm4, xmm3                  // xmm4 = sx sy sz 1
		movaps  xmm5, xmm3                  // xmm5 = sx sy sz 1
	}

	//-- Prepare the scale vector.
	__asm {
		movss   xmm6, scale                 // xmm6 = scale  ?      ?      ?
		shufps  xmm6, xmm6, 0x00            // xmm6 = scale  scale  
		movlhps xmm6, xmm6                  // xmm6 = scale  scale  scale  scale

		//-- Do the transform multiplies.
		mulps   xmm3, xmm0                  // xmm3 = a1*sx  a2*sy  a3*sz  0
		mulps   xmm4, xmm1                  // xmm4 = b1*sx  b2*sy  b3*sz  0
		mulps   xmm5, xmm2                  // xmm5 = c1*sx  c2*sy  c3*sz  0

		//-- Do the scale multiplies.
		mulps   xmm3, xmm6                  // xmm3 = a1*sx*scale a2*sy*scale a3*sz*scale 0
		mulps   xmm4, xmm6                  // xmm4 = b1*sx*scale b2*sy*scale b3*sz*scale 0
		mulps   xmm5, xmm6                  // xmm5 = c1*sx*scale c2*sy*scale c3*sz*scale 0

		//-- Save out data.
		movaps  [ebx + 32], xmm3
		movaps  [ebx + 48], xmm4
		movaps  [ebx + 64], xmm5

		//-- Restore EBX
		pop     ebx
	}

	// @todo consider twizzling/detwizzling to be able to perform add in sse.
	return Vector(
		sseVariable[2][0] + sseVariable[2][1] + sseVariable[2][2],
		sseVariable[3][0] + sseVariable[3][1] + sseVariable[3][2],
		sseVariable[4][0] + sseVariable[4][1] + sseVariable[4][2]);

#endif
} //lint !e715 // scale/transform not referenced - it's in the asm

// ----------------------------------------------------------------------

void SseMath::skinPositionNormal_l2p(const Transform &transform, const Vector &sourcePosition, const Vector &sourceNormal, float scale, Vector &destPosition, Vector &destNormal)
{
	// NOTE: technically, my xmm register data contents comments are listing items in reverse order from how INTEL docs list them, left most val is really least significant value.

	__asm {
		//-- Keep track of ebx.  Client is crashing if I trash this.
		push    ebx

		//-- Load up matrix.
		mov     ebx, transform

		movaps  xmm0, [ebx + 0]  // xmm0 = a1 a2 a3 a4
		movaps  xmm1, [ebx + 16] // xmm1 = b1 b2 b3 b4
		movaps  xmm2, [ebx + 32] // xmm2 = c1 c2 c3 c4
	}

	//-- Prepare source position.
	sseVariable[0][0] = sourcePosition.x;
	sseVariable[0][1] = sourcePosition.y;
	sseVariable[0][2] = sourcePosition.z;
	sseVariable[0][3] = 1.0f;

	__asm {
		//-- Load up the source vector.
		mov     ebx, offset sseVariable

		movaps  xmm3, [ebx]                 // xmm3 = sx sy sz 1

		//-- Copy source to workspaces.
		movaps  xmm4, xmm3                  // xmm4 = sx sy sz 1
		movaps  xmm5, xmm3                  // xmm5 = sx sy sz 1

		//-- Prepare the scale vector.
		movss   xmm6, scale                 // xmm6 = scale  ?      ?      ?
		shufps  xmm6, xmm6, 0x00            // xmm6 = scale  scale  
		movlhps xmm6, xmm6                  // xmm6 = scale  scale  scale  scale

		//-- Do the transform multiplies.
		mulps   xmm3, xmm0                  // xmm3 = a1*sx  a2*sy  a3*sz  a4
		mulps   xmm4, xmm1                  // xmm4 = b1*sx  b2*sy  b3*sz  b4
		mulps   xmm5, xmm2                  // xmm5 = c1*sx  c2*sy  c3*sz  c4

		//-- Do the scale multiplies.
		mulps   xmm3, xmm6                  // xmm3 = a1*sx*scale a2*sy*scale a3*sz*scale a4*scale
		mulps   xmm4, xmm6                  // xmm4 = b1*sx*scale b2*sy*scale b3*sz*scale b4*scale
		mulps   xmm5, xmm6                  // xmm5 = c1*sx*scale c2*sy*scale c3*sz*scale c4*scale

		//-- Save out data.
		movaps  [ebx + 32], xmm3
		movaps  [ebx + 48], xmm4
		movaps  [ebx + 64], xmm5
	}

	destPosition.x = sseVariable[2][0] + sseVariable[2][1] + sseVariable[2][2] + sseVariable[2][3];
	destPosition.y = sseVariable[3][0] + sseVariable[3][1] + sseVariable[3][2] + sseVariable[3][3];
	destPosition.z = sseVariable[4][0] + sseVariable[4][1] + sseVariable[4][2] + sseVariable[4][3];

	//-- Prepare source normal.
	sseVariable[0][0] = sourceNormal.x;
	sseVariable[0][1] = sourceNormal.y;
	sseVariable[0][2] = sourceNormal.z;
	sseVariable[0][3] = 1.0f;

	__asm {
		//-- Load up the source vector.
		movaps  xmm3, [ebx]                 // xmm3 = sx sy sz 1

		//-- Copy source to workspaces.
		movaps  xmm4, xmm3                  // xmm4 = sx sy sz 1
		movaps  xmm5, xmm3                  // xmm5 = sx sy sz 1

		//-- Do the transform multiplies.
		mulps   xmm3, xmm0                  // xmm3 = a1*sx  a2*sy  a3*sz  a4
		mulps   xmm4, xmm1                  // xmm4 = b1*sx  b2*sy  b3*sz  b4
		mulps   xmm5, xmm2                  // xmm5 = c1*sx  c2*sy  c3*sz  c4

		//-- Do the scale multiplies.
		mulps   xmm3, xmm6                  // xmm3 = a1*sx*scale a2*sy*scale a3*sz*scale a4*scale
		mulps   xmm4, xmm6                  // xmm4 = b1*sx*scale b2*sy*scale b3*sz*scale b4*scale
		mulps   xmm5, xmm6                  // xmm5 = c1*sx*scale c2*sy*scale c3*sz*scale c4*scale

		//-- Save out data.
		movaps  [ebx + 32], xmm3
		movaps  [ebx + 48], xmm4
		movaps  [ebx + 64], xmm5

		//-- Restore EBX
		pop     ebx
	}

	destNormal.x = sseVariable[2][0] + sseVariable[2][1] + sseVariable[2][2];
	destNormal.y = sseVariable[3][0] + sseVariable[3][1] + sseVariable[3][2];
	destNormal.z = sseVariable[4][0] + sseVariable[4][1] + sseVariable[4][2];
} //lint !e715 // scale/transform not referenced - it's in the asm

// ----------------------------------------------------------------------

void SseMath::skinPositionNormalAdd_l2p(const Transform &transform, const Vector &sourcePosition, const Vector &sourceNormal, float scale, Vector &destPosition, Vector &destNormal)
{
	// NOTE: technically, my xmm register data contents comments are listing items in reverse order from how INTEL docs list them, left most val is really least significant value.

	__asm {
		//-- Keep track of ebx.  Client is crashing if I trash this.
		push    ebx

		//-- Load up matrix.
		mov     ebx, transform

		movaps  xmm0, [ebx + 0]  // xmm0 = a1 a2 a3 a4
		movaps  xmm1, [ebx + 16] // xmm1 = b1 b2 b3 b4
		movaps  xmm2, [ebx + 32] // xmm2 = c1 c2 c3 c4
	}

	//-- Prepare source position.
	sseVariable[0][0] = sourcePosition.x;
	sseVariable[0][1] = sourcePosition.y;
	sseVariable[0][2] = sourcePosition.z;
	sseVariable[0][3] = 1.0f;

	__asm {
		//-- Load up the source vector.
		mov     ebx, offset sseVariable

		movaps  xmm3, [ebx]                 // xmm3 = sx sy sz 1

		//-- Copy source to workspaces.
		movaps  xmm4, xmm3                  // xmm4 = sx sy sz 1
		movaps  xmm5, xmm3                  // xmm5 = sx sy sz 1

		//-- Prepare the scale vector.
		movss   xmm6, scale                 // xmm6 = scale  ?      ?      ?
		shufps  xmm6, xmm6, 0x00            // xmm6 = scale  scale  
		movlhps xmm6, xmm6                  // xmm6 = scale  scale  scale  scale

		//-- Do the transform multiplies.
		mulps   xmm3, xmm0                  // xmm3 = a1*sx  a2*sy  a3*sz  a4
		mulps   xmm4, xmm1                  // xmm4 = b1*sx  b2*sy  b3*sz  b4
		mulps   xmm5, xmm2                  // xmm5 = c1*sx  c2*sy  c3*sz  c4

		//-- Do the scale multiplies.
		mulps   xmm3, xmm6                  // xmm3 = a1*sx*scale a2*sy*scale a3*sz*scale a4*scale
		mulps   xmm4, xmm6                  // xmm4 = b1*sx*scale b2*sy*scale b3*sz*scale b4*scale
		mulps   xmm5, xmm6                  // xmm5 = c1*sx*scale c2*sy*scale c3*sz*scale c4*scale

		//-- Save out data.
		movaps  [ebx + 32], xmm3
		movaps  [ebx + 48], xmm4
		movaps  [ebx + 64], xmm5
	}

	destPosition.x += sseVariable[2][0] + sseVariable[2][1] + sseVariable[2][2] + sseVariable[2][3];
	destPosition.y += sseVariable[3][0] + sseVariable[3][1] + sseVariable[3][2] + sseVariable[3][3];
	destPosition.z += sseVariable[4][0] + sseVariable[4][1] + sseVariable[4][2] + sseVariable[4][3];

	//-- Prepare source normal.
	sseVariable[0][0] = sourceNormal.x;
	sseVariable[0][1] = sourceNormal.y;
	sseVariable[0][2] = sourceNormal.z;
	sseVariable[0][3] = 1.0f;

	__asm {
		//-- Load up the source vector.
		movaps  xmm3, [ebx]                 // xmm3 = sx sy sz 1

		//-- Copy source to workspaces.
		movaps  xmm4, xmm3                  // xmm4 = sx sy sz 1
		movaps  xmm5, xmm3                  // xmm5 = sx sy sz 1

		//-- Do the transform multiplies.
		mulps   xmm3, xmm0                  // xmm3 = a1*sx  a2*sy  a3*sz  a4
		mulps   xmm4, xmm1                  // xmm4 = b1*sx  b2*sy  b3*sz  b4
		mulps   xmm5, xmm2                  // xmm5 = c1*sx  c2*sy  c3*sz  c4

		//-- Do the scale multiplies.
		mulps   xmm3, xmm6                  // xmm3 = a1*sx*scale a2*sy*scale a3*sz*scale a4*scale
		mulps   xmm4, xmm6                  // xmm4 = b1*sx*scale b2*sy*scale b3*sz*scale b4*scale
		mulps   xmm5, xmm6                  // xmm5 = c1*sx*scale c2*sy*scale c3*sz*scale c4*scale

		//-- Save out data.
		movaps  [ebx + 32], xmm3
		movaps  [ebx + 48], xmm4
		movaps  [ebx + 64], xmm5

		//-- Restore EBX
		pop     ebx
	}

	destNormal.x += sseVariable[2][0] + sseVariable[2][1] + sseVariable[2][2];
	destNormal.y += sseVariable[3][0] + sseVariable[3][1] + sseVariable[3][2];
	destNormal.z += sseVariable[4][0] + sseVariable[4][1] + sseVariable[4][2];
} //lint !e715 // scale/transform not referenced - it's in the asm

// ======================================================================
