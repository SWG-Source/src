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
		DB::BindableInt32 const *column=nullptr;
		switch(position)
		{
			case 0:
				column=&row->attribute_0;
				break;
			case 1:
				column=&row->attribute_1;
				break;
			case 2:
				column=&row->attribute_2;
				break;
			case 3:
				column=&row->attribute_3;
				break;
			case 4:
				column=&row->attribute_4;
				break;
			case 5:
				column=&row->attribute_5;
				break;
			case 6:
				column=&row->attribute_6;
				break;
			case 7:
				column=&row->attribute_7;
				break;
			case 8:
				column=&row->attribute_8;
				break;
			case 9:
				column=&row->attribute_9;
				break;
			case 10:
				column=&row->attribute_10;
				break;
			case 11:
				column=&row->attribute_11;
				break;
			case 12:
				column=&row->attribute_12;
				break;
			case 13:
				column=&row->attribute_13;
				break;
			case 14:
				column=&row->attribute_14;
				break;
			case 15:
				column=&row->attribute_15;
				break;
			case 16:
				column=&row->attribute_16;
				break;
			case 17:
				column=&row->attribute_17;
				break;
			case 18:
				column=&row->attribute_18;
				break;
			case 19:
				column=&row->attribute_19;
				break;
			case 20:
				column=&row->attribute_20;
				break;
			case 21:
				column=&row->attribute_21;
				break;
			case 22:
				column=&row->attribute_22;
				break;
			case 23:
				column=&row->attribute_23;
				break;
			case 24:
				column=&row->attribute_24;
				break;
			case 25:
				column=&row->attribute_25;
				break;
			case 26:
				column=&row->attribute_26;
				break;

			default:
				FATAL(true,("Programmer bug:  getAttributesForObject(%s, &values, %i, %i):  attempted to read attribute %i, which is out of range",objectId.getValueString().c_str(),offset, howMany, position));
			}
		int value=100;
		if (column->isNull())
			WARNING(true,("Object %s had nullptr attribute %i, defaulting to 100",objectId.getValueString().c_str(), position));
		else
			value=column->getValue();
		
		values.push_back(static_cast<Attributes::Value>(value));
	}
}

// ======================================================================
