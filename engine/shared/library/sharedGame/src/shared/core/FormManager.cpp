//======================================================================
//
// FormManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/FormManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "UnicodeUtils.h"

#include <stdio.h>

#include <algorithm>
#include <map>
#include <vector>

//======================================================================

namespace FormManagerNamespace
{
	bool s_installed = false;
	bool s_fatalOnBadFormData = true;
	bool s_tablesLoaded = false;

	std::string const cs_baseDir ("datatables/form/");
	std::string const cs_filenameExt (".iff");
	std::string const cs_fileTableName ("datatables/form/filelist.iff");
	std::string const cs_formMapTableName ("datatables/form/formmap.iff");

	std::string const cs_commandName ("sendFormObjectData");

	Unicode::String const cs_space(Unicode::narrowToWide(" "));
	Unicode::String const cs_comma(Unicode::narrowToWide(","));

	std::string const cs_emptyStringPlaceholder("<none>");
}

using namespace FormManagerNamespace;

//----------------------------------------------------------------------

std::map<std::string, FormManager::Form*>* FormManager::ms_forms;
std::map<std::string, const FormManager::Form*> * FormManager::ms_serverObjectTemplateToForms;
std::map<std::string, const FormManager::Form*> * FormManager::ms_sharedObjectTemplateToForms;
std::map<std::string, bool> * FormManager::ms_automaticallyCreateObjectForServerObjectTemplate;

//======================================================================

FormManager::Field::Field(Form const * const parent)
: m_name(),
  m_type(INT),
  m_externalName(),
  m_defaultValue(),
  m_minimumValue(),
  m_maximumValue(),
  m_minimumValueSet(false),
  m_maximumValueSet(false),
  m_choices(new std::vector<std::string>),
  m_mustUseList(false),
  m_validationFilename(),
  m_validationFilenameColumn(-1),
  m_otherValidationRules(new std::vector<OtherValidationRules>),
  m_parentForm(parent),
  m_objvarBinding()
{
}

//----------------------------------------------------------------------

FormManager::Field::~Field()
{
	delete m_choices;
	m_choices = nullptr;
	delete m_otherValidationRules;
	m_otherValidationRules = nullptr;
	m_parentForm = nullptr;
}

//----------------------------------------------------------------------

bool FormManager::Field::isIntType() const
{
	return (m_type == FormManager::INT || m_type == FormManager::INTARRAY);
}

//----------------------------------------------------------------------

bool FormManager::Field::isFloatType() const
{
	return (m_type == FormManager::FLOAT || m_type == FormManager::FLOATARRAY);
}

//----------------------------------------------------------------------

bool FormManager::Field::isStringType() const
{
	return (m_type == FormManager::STRING || m_type == FormManager::STRINGARRAY);
}

//----------------------------------------------------------------------

bool FormManager::Field::isArrayType() const
{
	return (m_type == FormManager::INTARRAY || m_type == FormManager::FLOATARRAY || m_type == FormManager::STRINGARRAY);
}

//----------------------------------------------------------------------

int FormManager::Field::getMinimumIntValue(std::map<std::string, std::string> const & pendingData, std::string & validationField /*OUT*/) const
{
	if(!isIntType())
	{
		DEBUG_FATAL(true, ("not an int value"));
		return 0; //lint !e527 reachable in release
	}
	if(!m_minimumValueSet)
	{
		DEBUG_FATAL(true, ("no min value set"));
		return 0; //lint !e527 reachable in release
	}

	//try to turn the field value directly into a number
	int min = atoi(m_minimumValue.c_str());

	//see if the field value actually holds the name of another field, if so, find its value in the pending data set
	if(m_parentForm->isNameOfField(m_minimumValue))
	{
		std::map<std::string, std::string>::const_iterator i = pendingData.find(m_minimumValue);
		if(i != pendingData.end())
		{
			std::string const & fieldNameValueString = i->second;
			min = atoi(fieldNameValueString.c_str());
			validationField = i->first;
		}
		else
			return 0;
	}

	return min;
}

//----------------------------------------------------------------------

int FormManager::Field::getMaximumIntValue(std::map<std::string, std::string> const & pendingData, std::string & validationField /*OUT*/) const
{
	if(!isIntType())
	{
		DEBUG_FATAL(true, ("not an int value"));
		return 0; //lint !e527 reachable in release
	}
	if(!m_maximumValueSet)
	{
		DEBUG_FATAL(true, ("no max value set"));
		return 0; //lint !e527 reachable in release
	}

	//try to turn the field value directly into a number
	int max = atoi(m_maximumValue.c_str());

	//see if the field value actually holds the name of another field, if so, find its value in the pending data set
	if(m_parentForm->isNameOfField(m_minimumValue))
	{
		std::map<std::string, std::string>::const_iterator i = pendingData.find(m_maximumValue);
		if(i != pendingData.end())
		{
			std::string const & fieldNameValueString = i->second;
			max = atoi(fieldNameValueString.c_str());
			validationField = i->first;
		}
		else
			return 0;
	}

	return max;
}

//----------------------------------------------------------------------

float FormManager::Field::getMinimumFloatValue(std::map<std::string, std::string> const & pendingData, std::string & validationField /*OUT*/) const
{
	if(!isFloatType())
	{
		DEBUG_FATAL(true, ("not an int value"));
		return 0; //lint !e527 reachable in release
	}
	if(!m_minimumValueSet)
	{
		DEBUG_FATAL(true, ("no min value set"));
		return 0; //lint !e527 reachable in release
	}

	//try to turn the field value directly into a number
	float min = static_cast<float>(atof(m_minimumValue.c_str()));

	//see if the field value actually holds the name of another field, if so, find its value in the pending data set
	if(m_parentForm->isNameOfField(m_minimumValue))
	{
		std::map<std::string, std::string>::const_iterator i = pendingData.find(m_minimumValue);
		if(i != pendingData.end())
		{
			std::string const & fieldNameValueString = i->second;
			min = static_cast<float>(atof(fieldNameValueString.c_str()));
			validationField = i->first;
		}
		else
			return 0;
	}

	return min;
}

//----------------------------------------------------------------------

float FormManager::Field::getMaximumFloatValue(std::map<std::string, std::string> const & pendingData, std::string & validationField /*OUT*/) const
{
	if(!isFloatType())
	{
		DEBUG_FATAL(true, ("not an int value"));
		return 0; //lint !e527 reachable in release
	}
	if(!m_maximumValueSet)
	{
		DEBUG_FATAL(true, ("no max value set"));
		return 0; //lint !e527 reachable in release
	}

	//try to turn the field value directly into a number
	float max = static_cast<float>(atof(m_maximumValue.c_str()));

	//see if the field value actually holds the name of another field, if so, find its value in the pending data set
	if(m_parentForm->isNameOfField(m_minimumValue))
	{
		std::map<std::string, std::string>::const_iterator i = pendingData.find(m_maximumValue);
		if(i != pendingData.end())
		{
			std::string const & fieldNameValueString = i->second;
			max = static_cast<float>(atof(fieldNameValueString.c_str()));
			validationField = i->first;
		}
		else
			return 0;
	}

	return max;
}

//----------------------------------------------------------------------

bool FormManager::Field::isValidValue(int value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const
{
	if(!isIntType())
	{
		DEBUG_FATAL(true, ("FormManager::Field::isValidValue(int) called on a non-int field"));
		return false; //lint !e527 reachable in release
	}

	std::string validationField;
	std::string displayField;
	//check against any minimum value if there is one
	if(m_minimumValueSet)
	{
		int const minimumValue = getMinimumIntValue(pendingData, validationField);
		//if the value is invalid, fill out the error string with the appropriate data
		if(value < minimumValue)
		{
			if(!validationField.empty())
			{
				Field const * f = m_parentForm->getField(validationField);
				if(f)
					displayField = f->m_externalName;
				else
					displayField = validationField;
				errorString =  std::string("Field: \"") + m_externalName + std::string("\" must be >= \"") + displayField + std::string("\".");
			}
			else
			{
				char buf[256];
				IGNORE_RETURN(_itoa(minimumValue, buf, 10));
				errorString = std::string("Field: \"") + m_externalName + std::string("\" must be >= ") + buf + std::string(".");
			}
			return false;
		}
	}

	//check against any maximum value if there is one
	if(m_maximumValueSet)
	{
		int const maximumValue = getMaximumIntValue(pendingData, validationField);
		//if the value is invalid, fill out the error string with the appropriate data
		if(value > maximumValue)
		{
			if(!validationField.empty())
			{
				Field const * f = m_parentForm->getField(validationField);
				if(f)
					displayField = f->m_externalName;
				else
					displayField = validationField;
				errorString =  std::string("Field: \"") + m_externalName + std::string("\" must be <= \"") + displayField + std::string("\".");
			}
			else
			{
				char buf[256];
				IGNORE_RETURN(_itoa(maximumValue, buf, 10));
				errorString =  std::string("Field: \"") + m_externalName + std::string("\" must <= ") + buf + std::string(".");
			}
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------

bool FormManager::Field::isValidValue(float value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const
{
	if(!isFloatType())
	{
		DEBUG_FATAL(true, ("FormManager::Field::isValidValue(float) called on a non-float field"));
		return false; //lint !e527 reachable in release
	}

	std::string validationField;
	std::string displayField;
	//check against any minimum value if there is one
	if(m_minimumValueSet)
	{
		float const minimumValue = getMinimumFloatValue(pendingData, validationField);
		//if the value is invalid, fill out the error string with the appropriate data
		if(value < minimumValue)
		{
			if(!validationField.empty())
			{
				Field const * f = m_parentForm->getField(validationField);
				if(f)
					displayField = f->m_externalName;
				else
					displayField = validationField;
				errorString =  std::string("Field: \"") + m_externalName + std::string("\" must be >= \"") + displayField + std::string("\".");
			}
			else
			{
				char buf[1024];
				IGNORE_RETURN(snprintf(buf, sizeof(buf), "%f", minimumValue));
				errorString = std::string("Field: \"") + m_externalName + std::string("\" must be >= ") + buf + std::string(".");
			}
			return false;
		}
	}

	//check against any maximum value if there is one
	if(m_maximumValueSet)
	{
		float const maximumValue = getMaximumFloatValue(pendingData, validationField);
		//if the value is invalid, fill out the error string with the appropriate data
		if(value > maximumValue)
		{
			if(!validationField.empty())
			{
				Field const * f = m_parentForm->getField(validationField);
				if(f)
					displayField = f->m_externalName;
				else
					displayField = validationField;
				errorString =  std::string("Field: \"") + m_externalName + std::string("\" must be <= \"") + displayField + std::string("\".");
			}
			else
			{
				char buf[1024];
				IGNORE_RETURN(snprintf(buf, sizeof(buf), "%f", maximumValue));
				errorString =  std::string("Field: \"") + m_externalName + std::string("\" must be <= ") + buf + std::string(".");
			}
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------

bool FormManager::Field::isValidValue(std::string const & value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const
{
	if(!isStringType())
	{
		DEBUG_FATAL(true, ("FormManager::Field::isValidValue(string) called on a non-string field"));
		return false; //lint !e527 reachable in release
	}

	UNREF(value);
	UNREF(errorString);
	UNREF(pendingData);
	return true;
}

//----------------------------------------------------------------------

std::string const & FormManager::Field::getName() const
{
	return m_name;
}

//----------------------------------------------------------------------

std::string const & FormManager::Field::getExternalName() const
{
	return m_externalName;
}

//----------------------------------------------------------------------

std::vector<FormManager::OtherValidationRules> const & FormManager::Field::getOtherValidationRules() const
{
	return *m_otherValidationRules;
}

//----------------------------------------------------------------------

std::string const & FormManager::Field::getDefaultValue() const
{
	return m_defaultValue;
}

//----------------------------------------------------------------------

std::string const & FormManager::Field::getObjvarBinding() const
{
	return m_objvarBinding;
}

//======================================================================

FormManager::Form::Form()
: m_name(),
  m_fields(new std::map<std::string, FormManager::Field*>),
  m_orderedFieldList(new std::vector<Field const *>)
{
}

//----------------------------------------------------------------------

FormManager::Form::~Form()
{
	//this vector does NOT own the pointers
	delete m_orderedFieldList;
	m_orderedFieldList = nullptr;

	//this list owns the pointers, so release it
	std::for_each(m_fields->begin(), m_fields->end(), PointerDeleterPairSecond());
	delete m_fields;
	m_fields = nullptr;
}

//----------------------------------------------------------------------

bool FormManager::Form::isNameOfField(std::string const & fieldName) const
{
	std::map<std::string, Field *>::iterator i = m_fields->find(fieldName);
	return(i != m_fields->end());
}

//----------------------------------------------------------------------

FormManager::Field const * FormManager::Form::getField(std::string const & fieldName) const
{
	std::map<std::string, Field *>::iterator i = m_fields->find(fieldName);
	if(i != m_fields->end())
		return i->second;
	else
		return nullptr;
}

//----------------------------------------------------------------------

std::string const & FormManager::Form::getName() const
{
	return m_name;
}

//----------------------------------------------------------------------

std::vector<FormManager::Field const *> const & FormManager::Form::getOrderedFieldList() const
{
	return *m_orderedFieldList;
}

//======================================================================

void FormManager::install ()
{
	InstallTimer const installTimer("FormManager::install ");

	DEBUG_FATAL (s_installed, ("installed"));
	if(s_installed)
		return;

	s_installed = true;

	s_fatalOnBadFormData = ConfigSharedGame::getFatalOnBadFormData();

	ExitChain::add(remove,"FormManager::remove");

	ms_forms                       = new std::map<std::string, FormManager::Form*>;
	ms_serverObjectTemplateToForms = new std::map<std::string, const FormManager::Form*>;
	ms_sharedObjectTemplateToForms = new std::map<std::string, const FormManager::Form*>;
	ms_automaticallyCreateObjectForServerObjectTemplate = new std::map<std::string, bool>;

	s_tablesLoaded = false;

	loadData();
}

//----------------------------------------------------------------------

void FormManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	clearData();

	delete ms_forms;
	ms_forms = nullptr;
	delete ms_serverObjectTemplateToForms;
	ms_serverObjectTemplateToForms = nullptr;
	delete ms_sharedObjectTemplateToForms;
	ms_sharedObjectTemplateToForms = nullptr;
	delete ms_automaticallyCreateObjectForServerObjectTemplate;
	ms_automaticallyCreateObjectForServerObjectTemplate = nullptr;

	s_tablesLoaded = false;
	s_installed = false;
}

//----------------------------------------------------------------------

std::string const & FormManager::getFormCommandName()
{
	return cs_commandName;
}

//----------------------------------------------------------------------

void FormManager::reloadData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	clearData();
	loadData();
}

//----------------------------------------------------------------------

void FormManager::clearData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	//do NOT release the Forms that we point to, this map does not own them
	ms_serverObjectTemplateToForms->clear();
	ms_sharedObjectTemplateToForms->clear();
	ms_automaticallyCreateObjectForServerObjectTemplate->clear();

	//delete the Form pointers, since this map owns them
	std::for_each(ms_forms->begin(), ms_forms->end(), PointerDeleterPairSecond());
	ms_forms->clear();

	s_tablesLoaded = false;
}

//----------------------------------------------------------------------

void FormManager::loadData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	//read in the file list, read each file
	//reload the table if necessary in case we're rebuilding the form data 
	if(s_tablesLoaded)
		IGNORE_RETURN(DataTableManager::reload(cs_fileTableName));

	DataTable const * const fileTable = DataTableManager::getTable (cs_fileTableName.c_str(), true);
	if (fileTable)
	{
		int const numRows = fileTable->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			std::string const & formName = fileTable->getStringValue (0, i);
			std::string const & filename = fileTable->getStringValue (1, i);

			std::string const tableName = cs_baseDir + filename;
			if(s_tablesLoaded)
				IGNORE_RETURN(DataTableManager::reload(tableName));
			DataTable const * const table = DataTableManager::getTable (tableName.c_str(), true);
			if (table)
			{
				//reload the table first in case we're rebuilding the kb 
				loadTable(*table, tableName, formName);
			}
			else
			{
				DEBUG_FATAL(s_fatalOnBadFormData, ("Could not load form file %s", filename.c_str()));
			}
		}
	}

	const DataTable * formMapTable = DataTableManager::getTable(cs_formMapTableName.c_str(), true);
	if(formMapTable)
	{
		int const numRows = formMapTable->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			std::string const & serverTemplateName = formMapTable->getStringValue (0, i);
			std::string const & sharedTemplateName = formMapTable->getStringValue (1, i);
			ConstCharCrcString const & sharedCrcStr = ObjectTemplateList::lookUp(sharedTemplateName.c_str());
			uint32 const sharedCrc = sharedCrcStr.getCrc();
			if(sharedCrc == 0)
			{
				if(ConfigSharedGame::getFatalOnBadFormData())
				{
					DEBUG_FATAL(true, ("Malformed Form data: File %s row %d has has a sharedTemplate: %s which is not in the object template list", cs_formMapTableName.c_str(), i, sharedTemplateName.c_str()));
					continue;
				}
			}

			std::string const & form = formMapTable->getStringValue (2, i);
			int const autoCreateObject = formMapTable->getIntValue(3, i);

			if(autoCreateObject == 0)
				(*ms_automaticallyCreateObjectForServerObjectTemplate)[serverTemplateName] = false;
			else if(autoCreateObject == 1)
			{
				(*ms_automaticallyCreateObjectForServerObjectTemplate)[serverTemplateName] = true;
			}
			else
			{
				if(ConfigSharedGame::getFatalOnBadFormData())
				{
					DEBUG_FATAL(true, ("Malformed Form data: File %s row %d has neither 0 nor 1 for AUTOMATICALLY_CREATE_OBJECT", cs_formMapTableName.c_str(), i));
					(*ms_automaticallyCreateObjectForServerObjectTemplate)[serverTemplateName] = false;
				}
			}

			//make sure the template names aren't dupes of other rows in the table
			std::map<std::string, Form const *>::iterator i2 = ms_serverObjectTemplateToForms->find(serverTemplateName);
			if(i2 != ms_serverObjectTemplateToForms->end())
			{
				if(ConfigSharedGame::getFatalOnBadFormData())
				{
					DEBUG_FATAL(true, ("Malformed Form data: File %s row %d has a serverTemplate: %s already found in the table, this must be unique", cs_formMapTableName.c_str(), i, serverTemplateName.c_str()));
				}
				continue;
			}
			i2 = ms_sharedObjectTemplateToForms->find(sharedTemplateName);
			if(i2 != ms_sharedObjectTemplateToForms->end())
			{
				if(ConfigSharedGame::getFatalOnBadFormData())
				{
					DEBUG_FATAL(true, ("Malformed Form data: File %s row %d has a sharedTemplate: %s already found in the table, this must be unique", cs_formMapTableName.c_str(), i, sharedTemplateName.c_str()));
				}
				continue;
			}

			Form const * const f = getFormByName(form);
			if(f)
			{
				(*ms_serverObjectTemplateToForms)[serverTemplateName] = f;
				(*ms_sharedObjectTemplateToForms)[sharedTemplateName] = f;
			}
		}
	}

	if(!s_tablesLoaded)
		s_tablesLoaded = true;
}

//----------------------------------------------------------------------

void FormManager::loadTable(DataTable const & table, std::string const & tableName, std::string const & formName)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	UNREF(tableName);

	Form * const form = new Form();
	form->m_name = formName;

	int const numRows = table.getNumRows ();

	for (int i = 0; i < numRows; ++i)
	{
		std::string const & entryName                = table.getStringValue (0, i);
		int const           entryType                = table.getIntValue    (1, i);
		std::string const & externalName             = table.getStringValue (2, i);
		std::string const & defaultValue             = table.getStringValue (3, i);
		std::string const & minValue                 = table.getStringValue (4, i);
		std::string const & maxValue                 = table.getStringValue (5, i);
		std::string const & choiceList               = table.getStringValue (6, i);
		int const           mustUseList              = table.getIntValue    (7, i);
		std::string const & validationFilename       = table.getStringValue (8, i);
		int const           validationFilenameColumn = table.getIntValue    (9, i);
		std::string const & otherValidationRules     = table.getStringValue (10, i);
		std::string const & objvarBinding            = table.getStringValue (11, i);

		Field * const field = new Field(form);

		const FieldType fieldType = static_cast<FieldType>(entryType);

		field->m_name = entryName;
		field->m_type = fieldType;
		field->m_defaultValue = defaultValue;
		field->m_externalName = externalName;
		field->m_minimumValue = minValue;
		field->m_maximumValue = maxValue;
		if(!minValue.empty())
			field->m_minimumValueSet = true;
		if(!maxValue.empty())
			field->m_maximumValueSet = true;
		//TODO parse choices
		UNREF(choiceList);
		field->m_mustUseList = (mustUseList != 0 ? true : false);
		field->m_validationFilename = validationFilename;
		field->m_validationFilenameColumn = validationFilenameColumn;
		if(otherValidationRules == "NOT_EMPTY")
			field->m_otherValidationRules->push_back(FormManager::NOT_EMPTY);
		field->m_objvarBinding = objvarBinding;

		//put this field into the form
		std::map<std::string, Field*> * const fields = form->m_fields;
		if(fields)
			(*fields)[entryName] = field;
		std::vector<Field const *> * const orderedFields = form->m_orderedFieldList;
		if(orderedFields)
			orderedFields->push_back(field);
	}

	//put the form into our form list
	(*ms_forms)[formName] = form;
}

//----------------------------------------------------------------------

FormManager::Form const * FormManager::getFormByName(std::string const & formName)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return nullptr;

	std::map<std::string, Form*>::iterator i = ms_forms->find(formName);
	if(i != ms_forms->end())
		return i->second;
	else
		return nullptr;
}

//----------------------------------------------------------------------

FormManager::Form const * FormManager::getFormForServerObjectTemplate(std::string const & serverTemplateName)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return nullptr;

	std::map<std::string, const Form*>::iterator i = ms_serverObjectTemplateToForms->find(serverTemplateName);
	if(i != ms_serverObjectTemplateToForms->end())
		return i->second;
	else
		return nullptr;
}

//----------------------------------------------------------------------

FormManager::Form const * FormManager::getFormForSharedObjectTemplate(std::string const & sharedTemplateName)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return nullptr;

	std::map<std::string, const Form*>::iterator i = ms_sharedObjectTemplateToForms->find(sharedTemplateName);
	if(i != ms_sharedObjectTemplateToForms->end())
		return i->second;
	else
		return nullptr;
}

//----------------------------------------------------------------------

FormManager::PackedFormData FormManager::packFormData(UnpackedFormData const & unpackedData)
{
	PackedFormData result;
	bool first = true;

	//turn a map<string, vector<string> > into a unicode::string formatted as "key1 value1,value2,value3 key2 value1 key3 value1,value2..."

	for(UnpackedFormData::const_iterator i = unpackedData.begin(); i != unpackedData.end(); ++i)
	{
		if(first)
			first = false;
		else
			result += cs_space;
		result += Unicode::narrowToWide(i->first) + cs_space;
		bool firstInList = true;
		for(std::vector<std::string>::const_iterator i2 = i->second.begin(); i2 != i->second.end(); ++i2)
		{
			if(firstInList)
				firstInList = false;
			else
				result += cs_comma;
			result += i2->empty() ? Unicode::narrowToWide(getEmptyStringPlaceholder()) : Unicode::narrowToWide(*i2);
		}
	}
	return result;
}

//----------------------------------------------------------------------

FormManager::UnpackedFormData FormManager::unpackFormData(PackedFormData const & packedData)
{
	UnpackedFormData result;

	//turn unicode::string formatted as "key1 value1,value2,value3 key2 value1 key3 value1,value2..." into a map<string, vector<string> > 

	Unicode::UnicodeStringVector tokens;
	IGNORE_RETURN(Unicode::tokenize(packedData, tokens));

	//must have an even number of tokens (key/value pairs)
	if(tokens.size() % 2 != 0)
		return result;

	std::string key;
	std::string value;
	std::vector<std::string> values;
	for(unsigned int i = 0; i + 1 < tokens.size(); i += 2)
	{
		key = Unicode::wideToNarrow(tokens[i]);
		value = Unicode::wideToNarrow(tokens[i + 1]);
		if(value == getEmptyStringPlaceholder())
			value.clear();

		values.clear();
		//TODO parse out comma lists here
		values.push_back(value);
		result[key] = values;
	}

	return result;
}

//----------------------------------------------------------------------

bool FormManager::getAutomaticallyCreateObject(std::string const & serverObjectTemplate)
{
	std::map<std::string, bool>::iterator i = ms_automaticallyCreateObjectForServerObjectTemplate->find(serverObjectTemplate);
	if(i != ms_automaticallyCreateObjectForServerObjectTemplate->end())
		return i->second;
	else
		return false;
}

//----------------------------------------------------------------------

std::string const & FormManager::getEmptyStringPlaceholder()
{
	return cs_emptyStringPlaceholder;
}

//======================================================================
