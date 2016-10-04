// ======================================================================
//
// CreatureObjectBuffer.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/CreatureObjectBuffer.h"
#include "serverNetworkMessages/UpdateObjectPositionMessage.h"
#include "sharedLog/Log.h"

// ======================================================================

CreatureObjectBuffer::CreatureObjectBuffer(DB::ModeQuery::Mode mode) :
		IndexedNetworkTableBuffer<DBSchema::CreatureObjectBufferRow, DBSchema::CreatureObjectRow, DBQuery::CreatureObjectQuery, DBQuery::CreatureObjectQuerySelect>(mode)
{
}


// ----------------------------------------------------------------------

bool CreatureObjectBuffer::handleUpdateObjectPosition(const UpdateObjectPositionMessage &message)
{

        DBSchema::CreatureObjectBufferRow *row=dynamic_cast<DBSchema::CreatureObjectBufferRow*>(findRowByIndex(message.getNetworkId()));
        if (row==0)
        {
                row=addEmptyRow(message.getNetworkId());
        }

        Vector v = message.getWorldspaceTransform().getPosition_p();
        row->ws_x = static_cast<double>(v.x);
        row->ws_y = static_cast<double>(v.y);
        row->ws_z = static_cast<double>(v.z);

        return true;
}


// ----------------------------------------------------------------------

void CreatureObjectBuffer::setAttribute(const NetworkId &objectId, Attributes::Enumerator type, Attributes::Value value)
{
	DBSchema::CreatureObjectBufferRow *row=dynamic_cast<DBSchema::CreatureObjectBufferRow*>(findRowByIndex(objectId));
	if (row==0)
	{
		row=addEmptyRow(objectId);
	}
	
	switch (type)
	{
		case 0:
			row->attribute_0=value;
			break;
		case 1:
			row->attribute_1=value;
			break;
		case 2:
			row->attribute_2=value;
			break;
		case 3:
			row->attribute_3=value;
			break;
		case 4:
			row->attribute_4=value;
			break;
		case 5:
			row->attribute_5=value;
			break;
		case 6:
			row->attribute_6=value;
			break;
		case 7:
			row->attribute_7=value;
			break;
		case 8:
			row->attribute_8=value;
			break;
		case 9:
			row->attribute_9=value;
			break;
		case 10:
			row->attribute_10=value;
			break;
		case 11:
			row->attribute_11=value;
			break;
		case 12:
			row->attribute_12=value;
			break;
		case 13:
			row->attribute_13=value;
			break;
		case 14:
			row->attribute_14=value;
			break;
		case 15:
			row->attribute_15=value;
			break;
		case 16:
			row->attribute_16=value;
			break;
		case 17:
			row->attribute_17=value;
			break;
		case 18:
			row->attribute_18=value;
			break;
		case 19:
			row->attribute_19=value;
			break;
		case 20:
			row->attribute_20=value;
			break;
		case 21:
			row->attribute_21=value;
			break;
		case 22:
			row->attribute_22=value;
			break;
		case 23:
			row->attribute_23=value;
			break;
		case 24:
			row->attribute_24=value;
			break;
		case 25:
			row->attribute_25=value;
			break;
		case 26:
			row->attribute_26=value;
			break;
		
		default:
			WARNING_STRICT_FATAL(true,("Programmer bug:  setAttribute(%s, %i, %i):  %i is not a valid attribute identifier",objectId.getValueString().c_str(), type, value, type));
	}
}

// ----------------------------------------------------------------------

/**
 * Retreive a range of attributes for an object
 */
void CreatureObjectBuffer::getAttributesForObject(const NetworkId &objectId, std::vector<Attributes::Value> &values, int offset, int howMany) const
{
	const DBSchema::CreatureObjectBufferRow *row=dynamic_cast<const DBSchema::CreatureObjectBufferRow*>(findConstRowByIndex(objectId));

	for (int position=offset; position<offset+howMany; ++position)
	{
		int value=0;
		switch(position)
		{
			case 0:
				row->attribute_0.getValue(value);
				break;
			case 1:
				row->attribute_1.getValue(value);
				break;
			case 2:
				row->attribute_2.getValue(value);
				break;
			case 3:
				row->attribute_3.getValue(value);
				break;
			case 4:
				row->attribute_4.getValue(value);
				break;
			case 5:
				row->attribute_5.getValue(value);
				break;
			case 6:
				row->attribute_6.getValue(value);
				break;
			case 7:
				row->attribute_7.getValue(value);
				break;
			case 8:
				row->attribute_8.getValue(value);
				break;
			case 9:
				row->attribute_9.getValue(value);
				break;
			case 10:
				row->attribute_10.getValue(value);
				break;
			case 11:
				row->attribute_11.getValue(value);
				break;
			case 12:
				row->attribute_12.getValue(value);
				break;
			case 13:
				row->attribute_13.getValue(value);
				break;
			case 14:
				row->attribute_14.getValue(value);
				break;
			case 15:
				row->attribute_15.getValue(value);
				break;
			case 16:
				row->attribute_16.getValue(value);
				break;
			case 17:
				row->attribute_17.getValue(value);
				break;
			case 18:
				row->attribute_18.getValue(value);
				break;
			case 19:
				row->attribute_19.getValue(value);
				break;
			case 20:
				row->attribute_20.getValue(value);
				break;
			case 21:
				row->attribute_21.getValue(value);
				break;
			case 22:
				row->attribute_22.getValue(value);
				break;
			case 23:
				row->attribute_23.getValue(value);
				break;
			case 24:
				row->attribute_24.getValue(value);
				break;
			case 25:
				row->attribute_25.getValue(value);
				break;
			case 26:
				row->attribute_26.getValue(value);
				break;

			default:
				FATAL(true,("Programmer bug:  getAttributesForObject(%s, &values, %i, %i):  attempted to read attribute %i, which is out of range",objectId.getValueString().c_str(),offset, howMany, position));
			}
		if (value == -999)
		{
			WARNING(true,("Object %s had nullptr attribute %i, defaulting to 100",objectId.getValueString().c_str(), position));
			value = 100;
		}
		
		values.push_back(static_cast<Attributes::Value>(value));
	}
}

// ======================================================================
