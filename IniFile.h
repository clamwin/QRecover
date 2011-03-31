#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
using namespace std;

// A function to trim whitespace from both sides of a given tstring
inline void Trim(tstring& str, const tstring & ChrsToTrim = _T(" \t\n\r"), int TrimDir = 0)
{
    size_t startIndex = str.find_first_not_of(ChrsToTrim);
    if (startIndex == tstring::npos){str.erase(); return;}
    if (TrimDir < 2) str = str.substr(startIndex, str.size()-startIndex);
    if (TrimDir!=1) str = str.substr(0, str.find_last_not_of(ChrsToTrim) + 1);
}


class CIniFile
{
public:
	struct Record
	{
		tstring Comments;
		TCHAR Commented;
		tstring Section;
		tstring Key;
		tstring Value;
	};

	enum CommentChar
	{
		Pound = '#',
		SemiColon = ';'
	};

	CIniFile(void);
	virtual ~CIniFile(void);

	static bool AddSection(tstring SectionName, tstring FileName);
	static bool CommentRecord(CommentChar cc, tstring KeyName,tstring SectionName,tstring FileName);
	static bool CommentSection(char CommentChar, tstring SectionName, tstring FileName);
	static tstring Content(tstring FileName);
	static bool Create(tstring FileName);
	static bool DeleteRecord(tstring KeyName, tstring SectionName, tstring FileName);
	static bool DeleteSection(tstring SectionName, tstring FileName);
	static vector<Record> GetRecord(tstring KeyName, tstring SectionName, tstring FileName);
	static vector<Record> GetSection(tstring SectionName, tstring FileName);
	static vector<tstring> GetSectionNames(tstring FileName);
	static tstring GetValue(tstring KeyName, tstring SectionName, tstring FileName);
	static bool RecordExists(tstring KeyName, tstring SectionName, tstring FileName);
	static bool RenameSection(tstring OldSectionName, tstring NewSectionName, tstring FileName);
	static bool SectionExists(tstring SectionName, tstring FileName);
	static bool SetRecordComments(tstring Comments, tstring KeyName, tstring SectionName, tstring FileName);
	static bool SetSectionComments(tstring Comments, tstring SectionName, tstring FileName);
	static bool SetValue(tstring KeyName, tstring Value, tstring SectionName, tstring FileName);
	static bool Sort(tstring FileName, bool Descending);
	static bool UnCommentRecord(tstring KeyName,tstring SectionName,tstring FileName);
	static bool UnCommentSection(tstring SectionName, tstring FileName);

private:
	static vector<Record> GetSections(tstring FileName);
	static bool Load(tstring FileName, vector<Record>& content);	
	static bool Save(tstring FileName, vector<Record>& content);

	struct RecordSectionIs : std::unary_function<Record, bool>
	{
		tstring section_;

		RecordSectionIs(const tstring& section): section_(section){}

		bool operator()( const Record& rec ) const
		{
			return rec.Section == section_;
		}
	};

	struct RecordSectionKeyIs : std::unary_function<Record, bool>
	{
		tstring section_;
		tstring key_;

		RecordSectionKeyIs(const tstring& section, const tstring& key): section_(section),key_(key){}

		bool operator()( const Record& rec ) const
		{
			tstring s(rec.Section.c_str()), k(rec.Key.c_str());
			tstring s1(section_.c_str()), k1(key_.c_str());
			std::transform(s.begin(), s.end(), s.begin(), tolower);
			std::transform(k.begin(), k.end(), k.begin(), tolower);
			std::transform(s1.begin(), s1.end(), s1.begin(), tolower);
			std::transform(k1.begin(), k1.end(), k1.begin(), tolower);
			Trim(s);
			Trim(s1);
			Trim(k);
			Trim(k1);

			return ((s == s1)&&(k == k1));
		}
	};

	struct AscendingSectionSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Section < End.Section;
		}
	};

	struct DescendingSectionSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Section > End.Section;
		}
	};

	struct AscendingRecordSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Key < End.Key;
		}
	};

	struct DescendingRecordSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Key > End.Key;
		}
	};
};
