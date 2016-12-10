// ======================================================================
//
// CompressedQuaternion.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CompressedQuaternion_H
#define INCLUDED_CompressedQuaternion_H

// ======================================================================

class Quaternion;

// ======================================================================

class CompressedQuaternion
{
public:

	typedef std::vector<Quaternion>            QuaternionVector;
	typedef std::vector<CompressedQuaternion>  CompressedQuaternionVector;

public:

	static void  install();

	static uint8 getOptimalCompressionFormat(float minValue, float maxValue);
	static void  getOptimalCompressionFormat(const QuaternionVector &sourceRotations, uint8 &xFormat, uint8 &yFormat, uint8 &zFormat);
	static void  compressRotations(const QuaternionVector &sourceRotations, uint8 xFormat, uint8 yFormat, uint8 zFormat, CompressedQuaternionVector &compressedRotations);

public:

	explicit CompressedQuaternion(uint32 compressedValue);
	CompressedQuaternion(const Quaternion &rhs, uint8 xFormat, uint8 yFormat, uint8 zFormat);
	CompressedQuaternion(float w, float x, float y, float z, uint8 xFormat, uint8 yFormat, uint8 zFormat);


	Quaternion  expand(uint8 xFormat, uint8 yFormat, uint8 zFormat) const;
	void        expand(uint8 xFormat, uint8 yFormat, uint8 zFormat, Quaternion &destination) const;
	void        expand(uint8 xFormat, uint8 yFormat, uint8 zFormat, float &w, float &x, float &y, float &z) const;

	uint32      getCompressedValue() const;

	void        debugDump() const;

private:

	uint32  m_data;

};

// ======================================================================

#endif
