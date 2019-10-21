#include <data/ini.h>
#include <Windows.h>

namespace stlx {

int Ini::ReadInt(const char* key, int defaultValue, const char* section) const
{
	return GetPrivateProfileInt(GetSection(section), key, defaultValue, _fname.c_str());
}

float Ini::ReadFloat(const char* key, float defaultValue, const char* section) const
{
	char defaultVal[256];
	sprintf_s(defaultVal, "%f", defaultValue);

	char result[256];
	GetPrivateProfileString(GetSection(section), key, defaultVal, result, sizeof(result), _fname.c_str());

	return (float)atof(result);
}

bool Ini::ReadBool(const char* key, bool defaultValue, const char* section) const
{
	char defaultVal[256];
	sprintf_s(defaultVal, "%s", defaultValue ? "true" : "false");

	char result[256];
	GetPrivateProfileString(GetSection(section), key, defaultVal, result, sizeof(result), _fname.c_str());

	return (strcmp(result, "true") == 0) ? true : false;
}

String Ini::ReadString(const char* key, const char* defaultValue, const char* section) const
{
	char result[256];
	GetPrivateProfileString(GetSection(section), key, defaultValue, result, sizeof(result), _fname.c_str());
	return result;
}


void Ini::WriteInt(const char* key, int val, const char* section) const
{
	char _val[256];
	sprintf_s(_val, "%d", val);
	WritePrivateProfileString(GetSection(section), key, _val, _fname.c_str());
}

void Ini::WriteFloat(const char* key, float val, const char* section) const
{
	char _val[256];
	sprintf_s(_val, "%f", val);
	WritePrivateProfileString(GetSection(section), key, _val, _fname.c_str());
}

void Ini::WriteBool(const char* key, bool val, const char* section) const
{
	char _val[256];
	sprintf_s(_val, "%s", val ? "true" : "false");
	WritePrivateProfileString(GetSection(section), key, _val, _fname.c_str());
}

void Ini::WriteString(const char* key, char* val, const char* section) const
{
	WritePrivateProfileString(GetSection(section), key, val, _fname.c_str());
}

bool Ini::FileExist(const char * fname)
{
	auto attr = GetFileAttributes(fname);

	return (attr != INVALID_FILE_ATTRIBUTES &&
		!(attr & FILE_ATTRIBUTE_DIRECTORY));
}

void Ini::SetSection(const char * section) {
	_section = section;
}

};