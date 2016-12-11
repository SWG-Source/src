//======================================================================
//
// FormManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_FormManager_H
#define INCLUDED_FormManager_H

//======================================================================

class DataTable;

//#include "sharedFoundation/StlForwardDeclaration.h"

//----------------------------------------------------------------------

class FormManager
{
public:
	typedef Unicode::String PackedFormData;
	typedef std::map<std::string, std::vector<std::string> > UnpackedFormData;

	//the various types of fields that the form can support
	enum FieldType
	{
		INT,
		FLOAT,
		STRING,
		INTARRAY,
		FLOATARRAY,
		STRINGARRAY
	};

	//validation rules not expressed elsewhere in the data
	enum OtherValidationRules
	{
		NOT_EMPTY
	};

	//the list of commands that are sent from the client to the server
	enum Command
	{
		CREATE_OBJECT,
		EDIT_OBJECT,
		REQUEST_EDIT_OBJECT
	};

	class Form;

	/**A field inside the form.  It represents one logical field, and can validate its own data
	*/
	class Field
	{
		//allow fellow inner class and containing manager to access privates
		friend class Form;
		friend class FormManager;

		public:
			Field(Form const * const parent);
			~Field();

			bool isIntType() const;
			bool isFloatType() const;
			bool isStringType() const;
			bool isArrayType() const;
			bool isValidValue(std::string const & value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const;
			bool isValidValue(int value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const;
			bool isValidValue(float value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const;
			int getMinimumIntValue(std::map<std::string, std::string> const & pendingData, std::string & validationField /*OUT*/) const;
			int getMaximumIntValue(std::map<std::string, std::string> const & pendingData, std::string & validationField /*OUT*/) const;
			float getMinimumFloatValue(std::map<std::string, std::string> const & pendingData, std::string & validationField /*OUT*/) const;
			float getMaximumFloatValue(std::map<std::string, std::string> const & pendingData, std::string & validationField /*OUT*/) const;

			std::string const & getName() const;
			std::string const & getExternalName() const;
			std::vector<OtherValidationRules> const & getOtherValidationRules() const;
			std::string const & getDefaultValue() const;
			std::string const & getObjvarBinding() const;

		private:
			Field();
			Field (const Field &);
			Field & operator= (const Field &);

		private:
			std::string m_name;
			FieldType   m_type;
			std::string m_externalName;
			std::string m_defaultValue;
			std::string m_minimumValue;
			std::string m_maximumValue;
			bool m_minimumValueSet;
			bool m_maximumValueSet;
			std::vector<std::string> * m_choices;
			bool        m_mustUseList;
			std::string m_validationFilename;
			int         m_validationFilenameColumn;
			std::vector<OtherValidationRules> * m_otherValidationRules;
			Form const * m_parentForm;
			std::string m_objvarBinding;
	};

	/**Representation of the form.  It holds a set of fields
	*/
	class Form
	{
		//allow containing manager to access privates
		friend class FormManager;

		public:
			bool isNameOfField(std::string const & fieldName) const;
			Field const * getField(std::string const & fieldName) const;
			std::string const & getName() const;
			std::vector<Field const *> const & getOrderedFieldList() const;

			Form();
			virtual ~Form();

		private:
			Form (const Form &);
			Form & operator= (const Form &);

		private:
			std::string m_name;
			std::map<std::string, Field *>* m_fields;
			std::vector<Field const *>* m_orderedFieldList;
	};

//This is an "abstract" static manager, which is instanciated by clientGame/FormManagerClient and serverGame/FormManagerServer
protected:
	static void install ();
	static void remove  ();
	virtual ~FormManager();

public:
	static void reloadData();
	static Form const * getFormByName(std::string const & formName);
	static Form const * getFormForServerObjectTemplate(std::string const & serverObjectTemplate);
	static Form const * getFormForSharedObjectTemplate(std::string const & sharedObjectTemplate);
	static bool getAutomaticallyCreateObject(std::string const & serverObjectTemplate);
	static std::string const & getEmptyStringPlaceholder();

	static PackedFormData packFormData(UnpackedFormData const & unpackedData);
	static UnpackedFormData unpackFormData(PackedFormData const & packedData);

	static std::string const & getFormCommandName();

private:
	static void clearData();
	static void loadData();
	static void validateData();
	static void loadTable(DataTable const & table, std::string const & tableName, std::string const & formName);

private:
	//this map OWNS the Form pointers
	static std::map<std::string, Form *> * ms_forms;
	//this map hold pointers to Forms, but does NOT own them
	static std::map<std::string, Form const *> * ms_serverObjectTemplateToForms;
	//this map hold pointers to Forms, but does NOT own them
	static std::map<std::string, Form const *> * ms_sharedObjectTemplateToForms;
	//this map stores whether a particular server template was mapped for an "auto-object-creating" form in formmap.tab
	static std::map<std::string, bool> * ms_automaticallyCreateObjectForServerObjectTemplate;
};

//======================================================================

#endif
