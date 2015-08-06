// ScriptMethodsDataTable.cpp 
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "fileInterface/StdioFile.h"
#include "sharedDebug/Profiler.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

using namespace JNIWrappersNamespace;


//------------------------------------------------------------------------
//lint -e818 //env could be declared const

// ======================================================================

#ifdef _DEBUG
static char const *findNextChar(char const *s, char c)
{
	char const *tmp = s;
	while (tmp && *tmp != '\0')
	{
		if (*tmp == c)
			return tmp;
		++tmp;
	}
	return 0;
}
#endif

// ----------------------------------------------------------------------

//ifdef out because this is currently only used in debug builds, and breaks the release build
#ifdef _DEBUG
static void parseTabbedLine(char const *line, int length, std::vector<std::string> &results)
{
	char const *currentPos = line;
	char const *tab = findNextChar(currentPos, '\t');
	char const *lineEnd = line+length;
	if (lineEnd > line && *(lineEnd-1) == '\r')
		--lineEnd;
	while (tab && tab < lineEnd)
	{
		results.push_back(std::string(currentPos, tab-currentPos));
		currentPos = tab+1;
		tab = findNextChar(currentPos, '\t');
	}
	results.push_back(std::string(currentPos, lineEnd-currentPos));
}
#endif

// ======================================================================
// ScriptMethodsDataTableNamespace
// ======================================================================

namespace ScriptMethodsDataTableNamespace
{
	bool install();

	jint         JNICALL  dataTableGetInt(JNIEnv *env, jobject self, jstring table, jint row, jstring column);
	jfloat       JNICALL  dataTableGetFloat(JNIEnv *env, jobject self, jstring table, jint row, jstring column);
	jstring      JNICALL  dataTableGetString(JNIEnv *env, jobject self, jstring table, jint row, jstring column);
	jintArray    JNICALL  dataTableGetIntColumnInternal(JNIEnv *env, jobject self, jstring table, jstring column, bool withDefaults);
	jintArray    JNICALL  dataTableGetIntColumn(JNIEnv *env, jobject self, jstring table, jstring column);
	jintArray    JNICALL  dataTableGetIntColumnNoDefaults(JNIEnv *env, jobject self, jstring table, jstring column);
	jfloatArray  JNICALL  dataTableGetFloatColumnInternal(JNIEnv *env, jobject self, jstring table, jstring column, bool withDefaults);
	jfloatArray  JNICALL  dataTableGetFloatColumn(JNIEnv *env, jobject self, jstring table, jstring column);
	jfloatArray  JNICALL  dataTableGetFloatColumnNoDefaults(JNIEnv *env, jobject self, jstring table, jstring column);
	jobjectArray JNICALL  dataTableGetStringColumnInternal(JNIEnv *env, jobject self, jstring table, jstring column, bool withDefaults);
	jobjectArray JNICALL  dataTableGetStringColumn(JNIEnv *env, jobject self, jstring table, jstring column);
	jobjectArray JNICALL  dataTableGetStringColumnNoDefaults(JNIEnv *env, jobject self, jstring table, jstring column);
	jint         JNICALL  dataTableGetIntA(JNIEnv *env, jobject self, jstring table, jint row, jint column);
	jfloat       JNICALL  dataTableGetFloatA(JNIEnv *env, jobject self, jstring table, jint row, jint column);
	jstring      JNICALL  dataTableGetStringA(JNIEnv *env, jobject self, jstring table, jint row, jint column);
	jintArray    JNICALL  dataTableGetIntColumnAInternal(JNIEnv *env, jobject self, jstring table, jint column, bool withDefaults);
	jintArray    JNICALL  dataTableGetIntColumnA(JNIEnv *env, jobject self, jstring table, jint column);
	jintArray    JNICALL  dataTableGetIntColumnANoDefaults(JNIEnv *env, jobject self, jstring table, jint column);
	jfloatArray  JNICALL  dataTableGetFloatColumnAInternal(JNIEnv *env, jobject self, jstring table, jint column, bool withDefaults);
	jfloatArray  JNICALL  dataTableGetFloatColumnA(JNIEnv *env, jobject self, jstring table, jint column);
	jfloatArray  JNICALL  dataTableGetFloatColumnANoDefaults(JNIEnv *env, jobject self, jstring table, jint column);
	jobjectArray JNICALL  dataTableGetStringColumnAInternal(JNIEnv *env, jobject self, jstring table, jint column, bool withDefaults);
	jobjectArray JNICALL  dataTableGetStringColumnA(JNIEnv *env, jobject self, jstring table, jint column);
	jobjectArray JNICALL  dataTableGetStringColumnANoDefaults(JNIEnv *env, jobject self, jstring table, jint column);
	jboolean     JNICALL  dataTableOpen(JNIEnv *env, jobject self, jstring table);
	jint         JNICALL  dataTableGetColumnType(JNIEnv *env, jobject self, jstring table, jstring column);
	jint         JNICALL  dataTableGetColumnTypeA(JNIEnv *env, jobject self, jstring table, jint column);
	jboolean     JNICALL  dataTableHasColumn(JNIEnv *env, jobject self, jstring table, jstring column);
	jint         JNICALL  dataTableGetNumColumns (JNIEnv *env, jobject self, jstring table);
	jint         JNICALL  dataTableGetNumRows (JNIEnv *env, jobject self, jstring table);
	jobject      JNICALL  dataTableGetRow(JNIEnv *env, jobject self, jstring table, jint row);
	jstring      JNICALL  dataTableGetColumnName(JNIEnv *env, jobject self, jstring table, jint column);
	jobjectArray JNICALL  dataTableGetColumnNames(JNIEnv *env, jobject self, jstring table);
	jint         JNICALL  dataTableSearchColumnForString(JNIEnv *env, jobject self, jstring entry, jstring column, jstring table);
	jint         JNICALL  dataTableSearchColumnForInt(JNIEnv *env, jobject self, jint entry, jstring column, jstring table);
	jint         JNICALL  dataTableSearchColumnForStringA(JNIEnv *env, jobject self, jstring entry, jint column, jstring table);
	jint         JNICALL  dataTableSearchColumnForIntA(JNIEnv *env, jobject self, jint entry, jint column, jstring table);
	jint         JNICALL  dataTableFindColumnNumber(JNIEnv *env, jobject self, jstring table, jstring column);
	void         JNICALL  dataTableAddRow(JNIEnv *env, jobject self, jstring table, jobject row);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsDataTableNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsDataTableNamespace::c)}
	JF("dataTableGetInt", "(Ljava/lang/String;ILjava/lang/String;)I", dataTableGetInt),
	JF("dataTableGetFloat", "(Ljava/lang/String;ILjava/lang/String;)F", dataTableGetFloat),
	JF("dataTableGetString", "(Ljava/lang/String;ILjava/lang/String;)Ljava/lang/String;", dataTableGetString),
	JF("dataTableGetInt", "(Ljava/lang/String;II)I", dataTableGetIntA),
	JF("dataTableGetFloat", "(Ljava/lang/String;II)F", dataTableGetFloatA),
	JF("dataTableGetString", "(Ljava/lang/String;II)Ljava/lang/String;", dataTableGetStringA),
	JF("dataTableGetIntColumn", "(Ljava/lang/String;Ljava/lang/String;)[I", dataTableGetIntColumn),
	JF("dataTableGetIntColumnNoDefaults", "(Ljava/lang/String;Ljava/lang/String;)[I", dataTableGetIntColumnNoDefaults),
	JF("dataTableGetFloatColumn", "(Ljava/lang/String;Ljava/lang/String;)[F", dataTableGetFloatColumn),
	JF("dataTableGetFloatColumnNoDefaults", "(Ljava/lang/String;Ljava/lang/String;)[F", dataTableGetFloatColumnNoDefaults),
	JF("dataTableGetStringColumn", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;", dataTableGetStringColumn),
	JF("dataTableGetStringColumnNoDefaults", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;", dataTableGetStringColumnNoDefaults),
	JF("dataTableGetIntColumn", "(Ljava/lang/String;I)[I", dataTableGetIntColumnA),
	JF("dataTableGetIntColumnNoDefaults", "(Ljava/lang/String;I)[I", dataTableGetIntColumnANoDefaults),
	JF("dataTableGetFloatColumn", "(Ljava/lang/String;I)[F", dataTableGetFloatColumnA),
	JF("dataTableGetFloatColumnNoDefaults", "(Ljava/lang/String;I)[F", dataTableGetFloatColumnANoDefaults),
	JF("dataTableGetStringColumn", "(Ljava/lang/String;I)[Ljava/lang/String;", dataTableGetStringColumnA),
	JF("dataTableGetStringColumnNoDefaults", "(Ljava/lang/String;I)[Ljava/lang/String;", dataTableGetStringColumnANoDefaults),
	JF("dataTableOpen", "(Ljava/lang/String;)Z", dataTableOpen),
	JF("dataTableGetColumnType", "(Ljava/lang/String;Ljava/lang/String;)I", dataTableGetColumnType),
	JF("dataTableGetColumnType", "(Ljava/lang/String;I)I", dataTableGetColumnTypeA),
	JF("dataTableHasColumn", "(Ljava/lang/String;Ljava/lang/String;)Z", dataTableHasColumn),
	JF("dataTableGetNumRows", "(Ljava/lang/String;)I", dataTableGetNumRows),
	JF("dataTableGetNumColumns", "(Ljava/lang/String;)I", dataTableGetNumColumns),
	JF("dataTableGetRow", "(Ljava/lang/String;I)Lscript/dictionary;", dataTableGetRow),
	JF("dataTableGetColumnName",  "(Ljava/lang/String;I)Ljava/lang/String;", dataTableGetColumnName),
	JF("dataTableGetColumnNames", "(Ljava/lang/String;)[Ljava/lang/String;", dataTableGetColumnNames),
	JF("dataTableSearchColumnForString", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", dataTableSearchColumnForString),
	JF("dataTableSearchColumnForInt", "(ILjava/lang/String;Ljava/lang/String;)I", dataTableSearchColumnForInt),
	JF("dataTableSearchColumnForString", "(Ljava/lang/String;ILjava/lang/String;)I", dataTableSearchColumnForStringA),
	JF("dataTableSearchColumnForInt", "(IILjava/lang/String;)I", dataTableSearchColumnForIntA),
	JF("dataTableFindColumnNumber", "(Ljava/lang/String;Ljava/lang/String;)I", dataTableFindColumnNumber),
	JF("dataTableAddRow", "(Ljava/lang/String;Lscript/dictionary;)V", dataTableAddRow),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================

jint JNICALL ScriptMethodsDataTableNamespace::dataTableGetInt(JNIEnv *env, jobject self, jstring table, jint row, jstring column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetInt");

	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	JavaStringParam localColumn(column);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetInt - Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (row < 0 || dt->getNumRows() <= row)
	{
		DEBUG_WARNING(true, ("JavaLibaray::dataTableGetInt [%s] - Invalid row %d of table %s.", dt->getName().c_str(), row, tableNameString.c_str()));
		return 0;
	}
	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetInt [%s] - Invalid column name <%s> of table %s.", dt->getName().c_str(), columnNameString.c_str(), tableNameString.c_str()));
		return 0;
	}

	if (dt->getDataTypeForColumn(columnNameString).getBasicType() != DataTableColumnType::DT_Int)
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetInt [%s] - Invalid type (not an int), column <%s> of table %s.", dt->getName().c_str(), columnNameString.c_str(), tableNameString.c_str()));
		return 0;
	}

	return dt->getIntValue(columnNameString, row);
}

//------------------------------------------------------------------------

jfloat JNICALL ScriptMethodsDataTableNamespace::dataTableGetFloat(JNIEnv *env, jobject self, jstring table, jint row, jstring column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetFloat");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	JavaStringParam localColumn(column);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetFloat - Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (row < 0 || dt->getNumRows() <= row)
	{
		DEBUG_WARNING(true, ("JavaLibaray::dataTableGetFloat [%s] - Invalid row %d of table %s.", dt->getName().c_str(), row, tableNameString.c_str()));
		return 0;
	}
	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetFloat [%s] - Invalid column name <%s> of table %s.", dt->getName().c_str(), columnNameString.c_str(), tableNameString.c_str()));
		return 0;
	}

	if (dt->getDataTypeForColumn(columnNameString).getBasicType() != DataTableColumnType::DT_Float)
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetFloat [%s] - Invalid type (not a float), column <%s> of table %s.", dt->getName().c_str(), columnNameString.c_str(), tableNameString.c_str()));
		return 0;
	}

	return dt->getFloatValue(columnNameString, row);
}

//------------------------------------------------------------------------

jstring JNICALL ScriptMethodsDataTableNamespace::dataTableGetString(JNIEnv *env, jobject self, jstring table, jint row, jstring column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetString");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	JavaStringParam localColumn(column);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetString - Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (row < 0 || dt->getNumRows() <= row)
	{
		DEBUG_WARNING(true, ("JavaLibaray::dataTableGetString [%s] - Invalid row %d of table %s.", dt->getName().c_str(), row, tableNameString.c_str()));
		return 0;
	}
	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetString [%s] - Invalid column name <%s> of table %s.", dt->getName().c_str(), columnNameString.c_str(), tableNameString.c_str()));
		return 0;
	}

	if (dt->getDataTypeForColumn(columnNameString).getBasicType() != DataTableColumnType::DT_String)
	{
		DEBUG_WARNING(true, ("JavaLibrary::dataTableGetString [%s] - Invalid type (not a string), column <%s> of table %s.", dt->getName().c_str(), columnNameString.c_str(), tableNameString.c_str()));
		return 0;
	}

	JavaString javaString(dt->getStringValue(columnNameString, row));
	return javaString.getReturnValue();
}

//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableGetIntA(JNIEnv *env, jobject self, jstring table, jint row, jint column)
{
	UNREF(env);
	UNREF(self);

	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetIntA");
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (row < 0 || dt->getNumRows() <= row)
	{
		DEBUG_WARNING(true, ("Invalid row %d.", row));
		return 0;
	}
	if (column < 0 || dt->getNumColumns() <= column)
	{
		DEBUG_WARNING(true, ("Invalid column %d.", column));
		return 0;
	}

	if (dt->getDataTypeForColumn(column).getBasicType() != DataTableColumnType::DT_Int)
	{
		DEBUG_WARNING(true, ("Invalid type (not an int)."));
		return 0;
	}

	return dt->getIntValue(column, row);
}

//------------------------------------------------------------------------

jfloat JNICALL ScriptMethodsDataTableNamespace::dataTableGetFloatA(JNIEnv *env, jobject self, jstring table, jint row, jint column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetFloatA");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (row < 0 || dt->getNumRows() <= row)
	{
		DEBUG_WARNING(true, ("Invalid row %d.", row));
		return 0;
	}
	if (column < 0 || dt->getNumColumns() <= column)
	{
		DEBUG_WARNING(true, ("Invalid column %d.", column));
		return 0;
	}
	if (dt->getDataTypeForColumn(column).getBasicType() != DataTableColumnType::DT_Float)
	{
		DEBUG_WARNING(true, ("Invalid type (not an float)."));
		return 0;
	}
	return dt->getFloatValue(column, row);
}

//------------------------------------------------------------------------

jstring JNICALL ScriptMethodsDataTableNamespace::dataTableGetStringA(JNIEnv *env, jobject self, jstring table, jint row, jint column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetStringA");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	
	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (row < 0 || dt->getNumRows() <= row)
	{
		DEBUG_WARNING(true, ("Invalid row %d.", row));
		return 0;
	}
	if (column < 0 || dt->getNumColumns() <= column)
	{
		DEBUG_WARNING(true, ("Invalid column %d.", column));
		return 0;
	}
	if (dt->getDataTypeForColumn(column).getBasicType() != DataTableColumnType::DT_String)
	{
		DEBUG_WARNING(true, ("Invalid type (not a string)."));
		return 0;
	}
	JavaString javaString(dt->getStringValue(column, row));
	return javaString.getReturnValue();
}

//------------------------------------------------------------------------

jintArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetIntColumnInternal(JNIEnv *env, jobject self, jstring table, jstring column, bool withDefaults)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetIntColInt");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	JavaStringParam localColumn(column);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("[%s] Invalid column name <%s>.", dt->getName().c_str(), columnNameString.c_str()));
		return 0;
	}

	if (dt->getDataTypeForColumn(columnNameString).getBasicType() != DataTableColumnType::DT_Int)
	{
		DEBUG_WARNING(true, ("[%s] DataTable: Invalid type (not a int).", dt->getName().c_str()));
		return 0;
	}
	
	std::vector<int> ret;
	if (withDefaults)
	{
		dt->getIntColumn(columnNameString, ret);
	}
	else
	{
		int defaultValue = dt->getIntDefaultForColumn(columnNameString);
		std::vector<int> v;
		dt->getIntColumn(columnNameString, v);
		for (std::vector<int>::const_iterator i = v.begin(); i != v.end(); ++i)
			if ((*i) != defaultValue)
				ret.push_back(*i);
	}

	LocalIntArrayRefPtr valueArray = createNewIntArray(static_cast<long>(ret.size()));
	if (ret.size() > 0)
	{
		setIntArrayRegion(*valueArray, 0, static_cast<long>(ret.size()), reinterpret_cast<jint *>
			(const_cast<int *>(&ret[0])));
	}
	return valueArray->getReturnValue();
}

//------------------------------------------------------------------------

jintArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetIntColumn(JNIEnv *env, jobject self, jstring table, jstring column)
{
	return dataTableGetIntColumnInternal(env, self, table, column, true);
}

//------------------------------------------------------------------------

jintArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetIntColumnNoDefaults(JNIEnv *env, jobject self, jstring table, jstring column)
{
	return dataTableGetIntColumnInternal(env, self, table, column, false);
}

//------------------------------------------------------------------------

jfloatArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetFloatColumnInternal(JNIEnv *env, jobject self, jstring table, jstring column, bool withDefaults)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetFloatColInt");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	JavaStringParam localColumn(column);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}


	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("[%s] Invalid column name <%s>.", dt->getName().c_str(), columnNameString.c_str()));
		return 0;
	}

	if (dt->getDataTypeForColumn(columnNameString).getBasicType() != DataTableColumnType::DT_Float)
	{
		DEBUG_WARNING(true, ("[%s] Wrong type (not an float column)", dt->getName().c_str()));
		return 0;
	}
	
	std::vector<float> ret;
	if (withDefaults)
	{
		dt->getFloatColumn(columnNameString, ret);
	}
	else
	{
		float defaultValue = dt->getFloatDefaultForColumn(columnNameString);
		std::vector<float> v;
		dt->getFloatColumn(columnNameString, v);
		for (std::vector<float>::const_iterator i = v.begin(); i != v.end(); ++i)
			if ((*i) != defaultValue)   //lint !e777 //testing floats for equality
				ret.push_back(*i);
	}

	LocalFloatArrayRefPtr valueArray = createNewFloatArray(static_cast<jsize>(ret.size()));
	if (ret.size() > 0 && valueArray != LocalFloatArrayRef::cms_nullPtr)
	{
		setFloatArrayRegion(*valueArray, 0, static_cast<long>(ret.size()), reinterpret_cast<jfloat *>
			(const_cast<float *>(&ret[0])));
	}
	return valueArray->getReturnValue();

}

//------------------------------------------------------------------------

jfloatArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetFloatColumn(JNIEnv *env, jobject self, jstring table, jstring column)
{
	return dataTableGetFloatColumnInternal(env, self, table, column, true);
}

//------------------------------------------------------------------------

jfloatArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetFloatColumnNoDefaults(JNIEnv *env, jobject self, jstring table, jstring column)
{
	return dataTableGetFloatColumnInternal(env, self, table, column, false);
}

//------------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetStringColumnInternal(JNIEnv *env, jobject self, jstring table, jstring column, bool withDefaults)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetStringColInt");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	JavaStringParam localColumn(column);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableGetStringColumnInternal: could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableGetStringColumnInternal: could not convert column name from java string to string for table %s.", tableNameString.c_str()));
		return 0;
	}

	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("[%s] Invalid column name <%s>.", dt->getName().c_str(), columnNameString.c_str()));
		return 0;
	}

	if (dt->getDataTypeForColumn(columnNameString).getBasicType() != DataTableColumnType::DT_String)
	{
		DEBUG_WARNING(true, ("[%s] Wrong type (not an string column)", dt->getName().c_str()));
		return 0;
	}
	
	std::vector<const char *> ret;
	if (withDefaults)
	{
		dt->getStringColumn(columnNameString, ret);
	}
	else
	{
		std::string defaultValue(dt->getStringDefaultForColumn(columnNameString));
		std::vector<const char *> v;
		dt->getStringColumn(columnNameString, v);
		for (std::vector<const char *>::const_iterator i = v.begin(); i != v.end(); ++i)
		{
			const char *const string = *i;

			if (strcmp(string, defaultValue.c_str())!=0)
			{
				ret.push_back(*i);
			}
		}
	}

	size_t count = ret.size();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(static_cast<long>(count), JavaLibrary::getClsString());
	for (size_t i = 0; i < count; ++i)
	{
		const char *const string = ret[i];
		JavaString jval(string);
		setObjectArrayElement(*valueArray, static_cast<long>(i), jval);
	}
	return valueArray->getReturnValue();
}

//------------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetStringColumn(JNIEnv *env, jobject self, jstring table, jstring column)
{
	return dataTableGetStringColumnInternal(env, self, table, column, true);
}

//------------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetStringColumnNoDefaults(JNIEnv *env, jobject self, jstring table, jstring column)
{
	return dataTableGetStringColumnInternal(env, self, table, column, false);
}

//------------------------------------------------------------------------

jintArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetIntColumnAInternal(JNIEnv *env, jobject self, jstring table, jint column, bool withDefaults)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetIntColIntA");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (column < 0 || dt->getNumColumns() < column)
	{
		DEBUG_WARNING(true, ("Invalid column %d", column));
		return 0;
	}
	if (dt->getDataTypeForColumn(column).getBasicType() != DataTableColumnType::DT_Int)
	{
		DEBUG_WARNING(true, ("Wrong type (not an int column)"));
		return 0;
	}
	
	std::vector<int> ret;
	if (withDefaults)
	{
		dt->getIntColumn(column, ret);
	}
	else
	{
		int defaultValue = dt->getIntDefaultForColumn(column);
		std::vector<int> v;
		dt->getIntColumn(column, v);
		for (std::vector<int>::const_iterator i = v.begin(); i != v.end(); ++i)
			if ((*i) != defaultValue)
				ret.push_back(*i);
	}

	LocalIntArrayRefPtr valueArray = createNewIntArray(static_cast<long>(ret.size()));
	if (ret.size() > 0)
	{
		setIntArrayRegion(*valueArray, 0, static_cast<long>(ret.size()), reinterpret_cast<jint *>
			(const_cast<int *>(&ret[0])));
	}
	return valueArray->getReturnValue();
}

//------------------------------------------------------------------------

jintArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetIntColumnA(JNIEnv *env, jobject self, jstring table, jint column)
{
	return dataTableGetIntColumnAInternal(env, self, table, column, true);
}

//------------------------------------------------------------------------

jintArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetIntColumnANoDefaults(JNIEnv *env, jobject self, jstring table, jint column)
{
	return dataTableGetIntColumnAInternal(env, self, table, column, false);
}

//------------------------------------------------------------------------

jfloatArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetFloatColumnAInternal(JNIEnv *env, jobject self, jstring table, jint column, bool withDefaults)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetFloatColIntA");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (column < 0 || dt->getNumColumns() < column)
	{
		DEBUG_WARNING(true, ("Invalid column %d", column));
		return 0;
	}
	if (dt->getDataTypeForColumn(column).getBasicType() != DataTableColumnType::DT_Float)
	{
		DEBUG_WARNING(true, ("Wrong type (not an float column)"));
		return 0;
	}
	
	std::vector<float> ret;
	if (withDefaults)
	{
		dt->getFloatColumn(column, ret);
	}
	else
	{
		float defaultValue = dt->getFloatDefaultForColumn(column);
		std::vector<float> v;
		dt->getFloatColumn(column, v);
		for (std::vector<float>::const_iterator i = v.begin(); i != v.end(); ++i)
			if ((*i) != defaultValue)   //lint !e777 //testing floats for equality
				ret.push_back(*i);
	}

	LocalFloatArrayRefPtr valueArray = createNewFloatArray(static_cast<jsize>(ret.size()));
	if (ret.size() > 0 && valueArray != LocalFloatArrayRef::cms_nullPtr)
	{
		setFloatArrayRegion(*valueArray, 0, static_cast<long>(ret.size()), reinterpret_cast<jfloat *>
			(const_cast<float *>(&ret[0])));
	}
	return valueArray->getReturnValue();
}

//------------------------------------------------------------------------

jfloatArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetFloatColumnA(JNIEnv *env, jobject self, jstring table, jint column)
{
	return dataTableGetFloatColumnAInternal(env, self, table, column, true);
}

//------------------------------------------------------------------------

jfloatArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetFloatColumnANoDefaults(JNIEnv *env, jobject self, jstring table, jint column)
{
	return dataTableGetFloatColumnAInternal(env, self, table, column, false);
}

//------------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetStringColumnAInternal(JNIEnv *env, jobject self, jstring table, jint column, bool withDefaults)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetStringColIntA");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (column < 0 || dt->getNumColumns() < column)
	{
		DEBUG_WARNING(true, ("Invalid column %d", column));
		return 0;
	}
	if (dt->getDataTypeForColumn(column).getBasicType() != DataTableColumnType::DT_String)
	{
		DEBUG_WARNING(true, ("Wrong type (not an string column)"));
		return 0;
	}

	std::vector<const char *> ret;
	if (withDefaults)
	{
		dt->getStringColumn(column, ret);
	}
	else
	{
		std::string defaultValue(dt->getStringDefaultForColumn(column));
		std::vector<const char *> v;
		dt->getStringColumn(column, v);
		for (std::vector<const char *>::const_iterator i = v.begin(); i != v.end(); ++i)
		{
			const char *const string = *i;
			if (strcmp(string, defaultValue.c_str())!=0)
			{
				ret.push_back(*i);
			}
		}
	}

	size_t count = ret.size();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(static_cast<long>(count), JavaLibrary::getClsString());
	for (size_t i = 0; i < count; ++i)
	{
		const char *const string = ret[i];
		JavaString jval(string);
		setObjectArrayElement(*valueArray, static_cast<long>(i), jval);
	}
	return valueArray->getReturnValue();
}

//------------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetStringColumnA(JNIEnv *env, jobject self, jstring table, jint column)
{
	return dataTableGetStringColumnAInternal(env, self, table, column, true);
}

//------------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetStringColumnANoDefaults(JNIEnv *env, jobject self, jstring table, jint column)
{
	return dataTableGetStringColumnAInternal(env, self, table, column, false);
}

//------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsDataTableNamespace::dataTableOpen(JNIEnv *env, jobject self, jstring table)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableOpen");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return JNI_FALSE;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return JNI_FALSE;
	}
	return JNI_TRUE;
}

//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableGetColumnType(JNIEnv *env, jobject self, jstring table, jstring column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetColType");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	JavaStringParam localColumn(column);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("[%s] Invalid column name <%s>.", dt->getName().c_str(), columnNameString.c_str()));
		return 0;
	}
	return static_cast<jint>(dt->getDataTypeForColumn(columnNameString).getBasicType());
}

//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableGetColumnTypeA(JNIEnv *env, jobject self, jstring table, jint column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetColTypeA");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}
	if (column < 0 || dt->getNumColumns() < column)
	{
		DEBUG_WARNING(true, ("Invalid column %d", column));
		return -1;
	}
	return static_cast<jint>(dt->getDataTypeForColumn(column).getBasicType());
}
//------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsDataTableNamespace::dataTableHasColumn(JNIEnv *env, jobject self, jstring table, jstring column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableHasCol");
	UNREF(env);
	UNREF(self);
	
	JavaStringParam localTable(table);
	JavaStringParam localColumn(column);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}
	return dt->doesColumnExist(columnNameString);
}
//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableGetNumColumns (JNIEnv *env, jobject self, jstring table)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetNumColumns");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	return dt->getNumColumns();
}
//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableGetNumRows (JNIEnv *env, jobject self, jstring table)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetNumRows");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	return dt->getNumRows();

}
//------------------------------------------------------------------------

jobject JNICALL ScriptMethodsDataTableNamespace::dataTableGetRow(JNIEnv *env, jobject self, jstring table, jint row)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetRow");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (row < 0 || dt->getNumRows() <= row)
	{
		DEBUG_WARNING(true, ("Invalid row %d passed to dataTableGetInt.", row));
		JavaLibrary::throwInternalScriptError("Java script attempted to get an invalid row from a datatable");
		return 0;
	}

	
	LocalRefPtr target = createNewObject(JavaLibrary::getClsDictionary(), JavaLibrary::getMidDictionary());
	if (target == LocalRef::cms_nullPtr)
		return JNI_FALSE;

	for (int i = 0; i < dt->getNumColumns(); ++i)
	{
		JavaString jcolumnName(dt->getColumnName(i).c_str());
		switch (dt->getDataTypeForColumn(i).getBasicType())
		{
		case DataTableColumnType::DT_HashString:
		case DataTableColumnType::DT_Enum:
		case DataTableColumnType::DT_Bool:
		case DataTableColumnType::DT_Int:
		case DataTableColumnType::DT_BitVector:
			IGNORE_RETURN( callObjectMethod(*target, JavaLibrary::getMidDictionaryPutInt(), jcolumnName.getValue(), dt->getIntValue(i, row)) );
			break;
		case DataTableColumnType::DT_Float:
			IGNORE_RETURN( callObjectMethod(*target, JavaLibrary::getMidDictionaryPutFloat(), jcolumnName.getValue(), dt->getFloatValue(i, row)) );
			break;
		case DataTableColumnType::DT_String:
			{
				JavaString jtmpString(dt->getStringValue(i, row));
				IGNORE_RETURN ( callObjectMethod(*target, JavaLibrary::getMidDictionaryPut(), jcolumnName.getValue(), jtmpString.getValue()) );
				break;
			}
		case DataTableColumnType::DT_Unknown:
		case DataTableColumnType::DT_Comment:
		default:
			WARNING_STRICT_FATAL(true, ("Unhandled type in table %s row %d col %d", tableNameString.c_str(), row, i));
			return 0;
			break; //lint !e527 //-- unreachable statement
		}
	}
	return target->getReturnValue();
}

//------------------------------------------------------------------------

jstring JNICALL ScriptMethodsDataTableNamespace::dataTableGetColumnName(JNIEnv *env, jobject self, jstring table, jint column)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetColName");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	if (column < 0 || dt->getNumColumns() <= column)
	{
		DEBUG_WARNING(true, ("Invalid column %d", column));
		return 0;
	}
	JavaString javaString(dt->getColumnName(column).c_str());
	return javaString.getReturnValue();
}

//------------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsDataTableNamespace::dataTableGetColumnNames(JNIEnv *env, jobject self, jstring table)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetColNames");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);

	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return 0;
	}
	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return 0;
	}

	int numColumns = dt->getNumColumns();
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(numColumns, JavaLibrary::getClsString());
	for (int i = 0; i < numColumns; ++i)
	{
		const std::string & string = dt->getColumnName(i);
		JavaString jval(string.c_str());
		setObjectArrayElement(*valueArray, i, jval);
	}
	return valueArray->getReturnValue();
}

//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableSearchColumnForString(JNIEnv *env, jobject self, jstring entry, jstring column, jstring table)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableGetSearchForString");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return -1;
	}

	JavaStringParam localColumn(column);
	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return -1;
	}

	JavaStringParam localEntry(entry);
	std::string target;
	if (!JavaLibrary::convert(localEntry, target))
	{
		DEBUG_WARNING(true, ("Could not convert search name from java string to string."));
		return -1;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return -1;
	}

	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("[%s] Invalid column name <%s>.", dt->getName().c_str(), columnNameString.c_str()));
		return -1;
	}

	return dt->searchColumnString(dt->findColumnNumber(columnNameString), target);
}

//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableSearchColumnForStringA(JNIEnv *env, jobject self, jstring entry, jint column, jstring table)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableSearchForStringA");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return -1;
	}

	JavaStringParam localEntry(entry);
	std::string target;
	if (!JavaLibrary::convert(localEntry, target))
	{
		DEBUG_WARNING(true, ("Could not convert search name from java string to string."));
		return -1;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return -1;
	}

	return dt->searchColumnString(column, target);
}

//------------------------------------------------------------------------
jint JNICALL ScriptMethodsDataTableNamespace::dataTableSearchColumnForInt(JNIEnv *env, jobject self, jint entry, jstring column, jstring table)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableSearchForInt");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return -1;
	}

	JavaStringParam localColumn(column);
	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return -1;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return -1;
	}

	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("[%s] Invalid column name <%s>.", dt->getName().c_str(), columnNameString.c_str()));
		return -1;
	}

	return dt->searchColumnInt(dt->findColumnNumber(columnNameString), entry);
}

//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableSearchColumnForIntA(JNIEnv *env, jobject self, jint entry, jint column, jstring table)
{
	PROFILER_AUTO_BLOCK_DEFINE("JNI::dataTableSearchForIntA");
	UNREF(env);
	UNREF(self);
	JavaStringParam localTable(table);
	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return -1;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return -1;
	}

	return dt->searchColumnInt(column, entry);
}

//------------------------------------------------------------------------

jint JNICALL ScriptMethodsDataTableNamespace::dataTableFindColumnNumber(JNIEnv *env, jobject self, jstring table, jstring column)
{
	JavaStringParam localTable(table);
	std::string tableNameString;
	if (!JavaLibrary::convert(localTable, tableNameString))
	{
		DEBUG_WARNING(true, ("Could not convert table name from java string to string."));
		return -1;
	}

	JavaStringParam localColumn(column);
	std::string columnNameString;
	if (!JavaLibrary::convert(localColumn, columnNameString))
	{
		DEBUG_WARNING(true, ("Could not convert column name from java string to string."));
		return -1;
	}

	DataTable* dt = DataTableManager::getTable(tableNameString, true);
	if (!dt)
	{
		DEBUG_WARNING(true, ("Could not find table %s.", tableNameString.c_str()));
		return -1;
	}

	if (!dt->doesColumnExist(columnNameString))
	{
		DEBUG_WARNING(true, ("[%s] Invalid column name <%s>.", dt->getName().c_str(), columnNameString.c_str()));
		return -1;
	}

	return dt->findColumnNumber(columnNameString);
}

//------------------------------------------------------------------------

void JNICALL ScriptMethodsDataTableNamespace::dataTableAddRow(JNIEnv *env, jobject self, jstring table, jobject row)
{
#ifndef _DEBUG
	JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow is not allowed except in debug mode!"));
	return;
#else
	JavaStringParam localTable(table);
	std::string tableNameStr;
	if (!JavaLibrary::convert(localTable, tableNameStr))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow bad table name"));
		return;
	}
	if (!env->IsInstanceOf(row, JavaLibrary::getClsDictionary()))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow bad row dictionary, table %s", tableNameStr.c_str()));
		return;
	}

	std::vector<std::string> columnNames;
	std::vector<std::string> columnTypes;

	std::string fileName(tableNameStr);
	bool dosLinefeeds = false;

	// locate the table file and load column names and types
	{
		StdioFile inputFile(fileName.c_str(), "r");
		if (!inputFile.isOpen())
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow failed to open datatable file %s for reading", fileName.c_str()));
			return;
		}
		int fileLength = inputFile.length();
		char *buffer = new char[fileLength+2]; // assure at least 2 '\0's on the end so we always have 2 strings
		memset(buffer, 0, fileLength+2);
		int bytes_read = inputFile.read(buffer, fileLength);
		memset(buffer + bytes_read, 0, fileLength - bytes_read + 2);
		char const *namesLineEnd = findNextChar(buffer, '\n');
		if (namesLineEnd > buffer && *(namesLineEnd-1) == '\r')
			dosLinefeeds = true;
		if (!namesLineEnd)
		{
			delete [] buffer;
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow malformed column names row in datatable %s", tableNameStr.c_str()));
			return;
		}
		parseTabbedLine(buffer, namesLineEnd-buffer, columnNames);
		char const *typesLineEnd = findNextChar(namesLineEnd+1, '\n');
		if (!typesLineEnd)
		{
			delete [] buffer;
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow malformed column types row in datatable %s", tableNameStr.c_str()));
			return;
		}
		parseTabbedLine(namesLineEnd+1, typesLineEnd-namesLineEnd-1, columnTypes);
		delete [] buffer;
	}

	// pull the column names and types from the table file
	std::vector<std::string> addRowValues;

	if (columnNames.size() != columnTypes.size())
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow datatable column name or type is missing, table %s", tableNameStr.c_str()));
		return;
	}

	// resize the row values appropriately to account for the real number of columns
	addRowValues.resize(columnNames.size());

	LocalRefPtr keyEnumeration = callObjectMethod(LocalRefParam(row), JavaLibrary::getMidDictionaryKeys());
	while (callBooleanMethod(*keyEnumeration, JavaLibrary::getMidEnumerationHasMoreElements()))
	{
		LocalRefPtr key = callObjectMethod(*keyEnumeration, JavaLibrary::getMidEnumerationNextElement());
		if (!isInstanceOf(*key, JavaLibrary::getClsString()))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow dictionary has bad key, table %s", tableNameStr.c_str()));
			return;
		}
		JavaStringParam localKeyStr(*key);
		std::string keyStr;
		JavaLibrary::convert(localKeyStr, keyStr);
		// find the column
		unsigned int column;
		for (column = 0; column < columnNames.size(); ++column)
			if (columnNames[column] == keyStr)
				break;
		if (column == columnNames.size())
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow unknown column %s for datatable %s", keyStr.c_str(), tableNameStr.c_str()));
			return;
		}
		LocalRefPtr value = callObjectMethod(LocalRefParam(row), JavaLibrary::getMidDictionaryGet(), key->getValue());
		// verify the value is of an appropriate type, and set appropriately in the new row
		switch (columnTypes[column].c_str()[0])
		{
		case 'e':
			// TODO: validate enumerations here if desired
		case 'i':
			{
				if (!isInstanceOf(*value, JavaLibrary::getClsInteger()))
				{
					JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow non-int value for column %s of datatable %s", keyStr.c_str(), tableNameStr.c_str()));
					return;
				}
				jint intValue = callIntMethod(*value, JavaLibrary::getMidIntegerIntValue());
				char buf[64];
				snprintf(buf, sizeof(buf)-1, "%d", intValue);
				buf[sizeof(buf)-1] = '\0';
				addRowValues[column] = buf;
			}
			break;
		case 'f':
			{
				if (!isInstanceOf(*value, JavaLibrary::getClsFloat()))
				{
					JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow non-float value for column %s of datatable %s", keyStr.c_str(), tableNameStr.c_str()));
					return;
				}
				jfloat floatValue = callFloatMethod(*value, JavaLibrary::getMidFloatFloatValue());
				char buf[64];
				snprintf(buf, sizeof(buf)-1, "%12g", static_cast<double>(floatValue));
				buf[sizeof(buf)-1] = '\0';
				addRowValues[column] = buf;
			}
			break;
		case 'b':
			{
				if (!isInstanceOf(*value, JavaLibrary::getClsBoolean()))
				{
					JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow non-float value for column %s of datatable %s", keyStr.c_str(), tableNameStr.c_str()));
					return;
				}
				jboolean booleanValue = callBooleanMethod(*value, JavaLibrary::getMidBooleanBooleanValue());
				addRowValues[column] = booleanValue ? "1" : "0";
			}
			break;
		case 's':
		case 'h':
		case 'c':
		case 'p':
			{
				if (!isInstanceOf(*value, JavaLibrary::getClsString()))
				{
					JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow non-string value for column %s of datatable %s", keyStr.c_str(), tableNameStr.c_str()));
					return;
				}
				JavaStringParam localValueStr(*value);
				std::string valueStr;
				JavaLibrary::convert(localValueStr, valueStr);
				addRowValues[column] = valueStr;
			}
			break;
		default:
			{
				JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow unsupported type %s for column %s of datatable %s", columnTypes[column].c_str(), keyStr.c_str(), tableNameStr.c_str()));
			}
			return;
		}
	}

	// append the new row to the datatable
	{
		StdioFile outputFile(fileName.c_str(), "a");
		if (!outputFile.isOpen())
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::dataTableAddRow failed to open datatable file %s for appending", fileName.c_str()));
			return;
		}

		for (unsigned int i = 0; i < addRowValues.size(); ++i)
		{
			int len = addRowValues[i].length();
			if (len)
				outputFile.write(len, addRowValues[i].c_str());
			if (i < addRowValues.size()-1)
				outputFile.write(1, "\t");
		}
		if (dosLinefeeds)
			outputFile.write(1, "\r");
		outputFile.write(1, "\n");
	}
#endif
}

//------------------------------------------------------------------------

