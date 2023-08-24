
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <mylcd.h>
#include <psapi.h>
#include "scriptval.h"



void MakeIntValue (ScriptVal *sv, int64_t value)
{
	memset(sv, 0, sizeof(ScriptVal));
	sv->intValue = value;
	sv->type = SCRIPT_INT;
}

void MakeDoubleValue (ScriptVal *sv, double value)
{
	memset(sv, 0, sizeof(ScriptVal));
	sv->doubleValue = value;
	sv->type = SCRIPT_DOUBLE;
}

int MakeStringValueW (ScriptVal *sv, wchar_t *value)
{
	memset(sv, 0, sizeof(ScriptVal));
	if (!(sv->wideStringValue.value = wcsdup(value))) return 0;
	sv->wideStringValue.len = (int)wcslen(value);
	sv->flags = FLAG_NEED_FREE_DLL;
	sv->type = SCRIPT_WIDE_STRING;
	return 1;
}

int MakeStringValue (ScriptVal *sv, char *value)
{
	memset(sv, 0, sizeof(ScriptVal));
	if (!(sv->stringValue.value = (unsigned char*)strdup((char*)value))) return 0;
	sv->stringValue.len = (int)strlen((char*)value);
	sv->flags = FLAG_NEED_FREE_DLL;
	sv->type = SCRIPT_STRING;
	return 1;
}

void MakeDictValue (ScriptVal *sv)
{
	memset(sv, 0, sizeof(ScriptVal));
	sv->flags = FLAG_NEED_FREE_DLL;
	sv->dictValue.numEntries = 0;
	sv->dictValue.entries = 0;
	sv->type = SCRIPT_DICT;
}

int ScriptDictAddSV (ScriptDict *sd, ScriptVal *key, ScriptVal *val)
{
	ScriptDictEntry *temp = (ScriptDictEntry*)realloc(sd->entries, sizeof(ScriptDictEntry)*(sd->numEntries+1));
	if (!temp) return 0;

	sd->entries = temp;
	sd->entries[sd->numEntries].key = *key;
	sd->entries[sd->numEntries].value = *val;
	sd->numEntries++;
	return 1;
}

void MakeListValue (ScriptVal *sv)
{
	memset(sv, 0, sizeof(ScriptVal));
	sv->flags = FLAG_NEED_FREE_DLL;
	sv->listValue.numVals = 0;
	sv->listValue.vals = 0;
	sv->type = SCRIPT_LIST;
}

int ScriptListPushBackSV (ScriptList *sl, ScriptVal *value)
{
	ScriptVal *temp = (ScriptVal*)realloc(sl->vals, sizeof(ScriptVal)*(sl->numVals+1));
	if (!temp) return 0;
	sl->vals = temp;
	sl->vals[sl->numVals] = *value;
	sl->numVals++;
	return 1;
}

int ScriptListPushBackInt64 (ScriptList *sl, int64_t i)
{
	ScriptVal val;
	MakeIntValue(&val, i);
	return ScriptListPushBackSV(sl, &val);
}

void MakeNullValue (ScriptVal *sv)
{
	sv->type = SCRIPT_NULL;
	sv->intValue = 0;
}
