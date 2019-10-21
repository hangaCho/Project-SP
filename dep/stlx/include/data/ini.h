#pragma once

#include <stlxtypes.h>

namespace stlx {

class Ini {
public:
	Ini(const char * fileName, const char * section = "")
		: _fname(fileName), _section(section) {
	}
	~Ini() {}

	static bool FileExist(const char * fname);
	char * GetSection(const char * section = nullptr) const { return (char*)(section ? section : _section.c_str()); }
	void SetSection(const char * section);
	char * GetName() const { return (char*)_fname.c_str(); }

	int ReadInt(const char * key, int defaultValue, const char* section = nullptr) const;
	float ReadFloat(const char * key, float defaultValue, const char* section = nullptr) const;
	bool ReadBool(const char * key, bool defaultValue, const char* section = nullptr) const;
	String ReadString(const char * key, const char * defaultValue, const char* section = nullptr) const;

	void WriteInt(const char * key, int val, const char* section = nullptr) const;
	void WriteFloat(const char * key, float val, const char* section = nullptr) const;
	void WriteBool(const char * key, bool val, const char* section = nullptr) const;
	void WriteString(const char * key, char * val, const char* section = nullptr) const;

private:
	const String _fname;
	String _section;
};

};