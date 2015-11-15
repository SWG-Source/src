// ======================================================================
//
// LayerProperty.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LayerProperty_H
#define INCLUDED_LayerProperty_H

// ======================================================================

#include "sharedObject/Property.h"
#include "sharedTerrain/TerrainGenerator.h"


// ======================================================================

class LayerProperty : public Property
{
public:

	static PropertyId getClassPropertyId();

public:
	LayerProperty(Object& thisObject);
	~LayerProperty();

	TerrainGenerator::Layer * getLayer() const;
	void                      setLayer(TerrainGenerator::Layer* layer);

private:
	TerrainGenerator::Layer * m_layer;

private:
	LayerProperty();
	LayerProperty(const LayerProperty&);
	LayerProperty& operator= (const LayerProperty&);
};


// ======================================================================

inline TerrainGenerator::Layer * LayerProperty::getLayer() const
{
	return m_layer;
}

// ======================================================================


#endif

