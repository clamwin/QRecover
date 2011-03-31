#include "StdAfx.h"
#include "inifile.h"

CIniFile::CIniFile(void)													// Default constructor
{
}

CIniFile::~CIniFile(void)
{
}

//inline void TrimRight(tstring& str, const tstring & ChrsToTrim = _T(" \t\n\r"))
//{
//    Trim(str, ChrsToTrim, 2);
//}

//inline void TrimLeft(tstring& str, const tstring & ChrsToTrim = _T(" \t\n\r"))
//{
//    Trim(str, ChrsToTrim, 1);
//}

// A function to transform a tstring to uppercase if neccessary
//void UCase(tstring& str, bool ucase)
//{
//	if(ucase) transform(str.begin(), str.end(), str.begin(), toupper);
//}

bool CIniFile::Load(tstring FileName, vector<Record>& content)
{
	tstring s;																// Holds the current line from the ini file
	tstring CurrentSection;													// Holds the current section name

	tifstream inFile (FileName.c_str());										// Create an input filestream
	if (!inFile.is_open()) return false;									// If the input file doesn't open, then return
	content.clear();														// Clear the content vector

	tstring comments = _T("");													// A tstring to store comments in

	while(!std::getline(inFile, s).eof())									// Read until the end of the file
	{
		Trim(s);															// Trim whitespace from the ends
		if(!s.empty())														// Make sure its not a blank line
		{
			Record r;														// Define a new record

			if((s[0]==_T('#'))||(s[0]==_T(';')))									// Is this a commented line?
			{
				if ((s.find(_T('['))==tstring::npos)&&							// If there is no [ or =
					(s.find(_T('='))==tstring::npos))							// Then it's a comment
				{
					comments += s + _T('\n');									// Add the comment to the current comments tstring
				} else {
					r.Commented = s[0];										// Save the comment character
					s.erase(s.begin());										// Remove the comment for further processing
					Trim(s);
				}// Remove any more whitespace
			} else r.Commented = _T(' ');										// else mark it as not being a comment

			if(s.find(_T('['))!=tstring::npos)									// Is this line a section?
			{		
				s.erase(s.begin());											// Erase the leading bracket
				s.erase(s.find(_T(']')));										// Erase the trailing bracket
				r.Comments = comments;										// Add the comments tstring (if any)
				comments = _T("");												// Clear the comments for re-use
				r.Section = s;												// Set the Section value
				r.Key = _T("");													// Set the Key value
				r.Value = _T("");												// Set the Value value
				CurrentSection = s;
			}

			if(s.find(_T('='))!=tstring::npos)									// Is this line a Key/Value?
			{
				r.Comments = comments;										// Add the comments tstring (if any)
				comments = _T("");												// Clear the comments for re-use
				r.Section = CurrentSection;									// Set the section to the current Section
				r.Key = s.substr(0,s.find(_T('=')));							// Set the Key value to everything before the = sign
				r.Value = s.substr(s.find(_T('='))+1);							// Set the Value to everything after the = sign
			}
			if(comments == _T(""))												// Don't add a record yet if its a comment line
				content.push_back(r);										// Add the record to content
		}
	}
	
	inFile.close();															// Close the file
	return true;
}

bool CIniFile::Save(tstring FileName, vector<Record>& content)
{
	tofstream outFile (FileName.c_str());									// Create an output filestream
	if (!outFile.is_open()) return false;									// If the output file doesn't open, then return

	for (int i=0;i<(int)content.size();i++)									// Loop through each vector
	{
		outFile << content[i].Comments;										// Write out the comments
		if(content[i].Key == _T(""))											// Is this a section?
			outFile << content[i].Commented << _T("[") 
			<< content[i].Section << _T("]") << endl;							// Then format the section
		else
			outFile << content[i].Commented << content[i].Key  
			<< _T("=") << content[i].Value << endl;								// Else format a key/value
	}

	outFile.close();														// Close the file
	return true;
}

tstring CIniFile::Content(tstring FileName)
{
	tstring s=_T("");															// Hold our return tstring
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file loads
	{
		for (int i=0;i<(int)content.size();i++)								// Loop through the content
		{
			if(content[i].Comments != _T("")) s += content[i].Comments;			// Add the comments
			if(content[i].Commented != _T(' ')) s += content[i].Commented;		// If this is commented, then add it
			if((content[i].Key == _T("")))										// Is this a section?
				s += _T('[') + content[i].Section + _T(']');						// Add the section
			else s += content[i].Key + _T('=') + content[i].Value;				// Or the Key value to the return srting

			if (i != content.size()) s += _T('\n');								// If this is not the last line, add a CrLf
		}
		return s;															// Return the contents
	}

	return _T("");
}

vector<tstring> CIniFile::GetSectionNames(tstring FileName)
{
	vector<tstring> data;													// Holds the return data
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for (int i=0;i<(int)content.size();i++)								// Loop through the content
		{
			if(content[i].Key ==_T(""))											// If there is no key value, then its a section
				data.push_back(content[i].Section);							// Add the section to the return data
		}
	}

	return data;															// Return the data
}

vector<CIniFile::Record> CIniFile::GetSection(tstring SectionName, tstring FileName)
{
	vector<Record> data;													// Holds the return data
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for (int i=0;i<(int)content.size();i++)								// Loop through the content
		{
			if((content[i].Section == SectionName) &&						// If this is the section name we want
				(content[i].Key != _T("")))										// but not the section name itself
				data.push_back(content[i]);									// Add the record to the return data
		}
	}
	
	return data;															// Return the data
}

bool CIniFile::RecordExists(tstring KeyName, tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found
	}
	return true;															// The Section/Key was found
}

bool CIniFile::SectionExists(tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionIs(SectionName));					// Locate the Section

		if (iter == content.end()) return false;							// The Section was not found
	}
	return true;															// The Section was found
}

vector<CIniFile::Record> CIniFile::GetRecord(tstring KeyName, tstring SectionName, tstring FileName)
{
	vector<Record> data;													// Holds the return data
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Record

		if (iter == content.end()) return data;								// The Record was not found

		data.push_back (*iter);												// The Record was found
	}
	return data;															// Return the Record
}

tstring CIniFile::GetValue(tstring KeyName, tstring SectionName, tstring FileName)
{
	vector<Record> content = GetRecord(KeyName,SectionName, FileName);		// Get the Record

	if(!content.empty())													// Make sure there is a value to return
		return content[0].Value;											// And return the value

	return _T("");																// No value was found
}

bool CIniFile::SetValue(tstring KeyName, tstring Value, tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		if(!SectionExists(SectionName,FileName))							// If the Section doesn't exist
		{
			Record s = {_T(""),_T(' '),SectionName,_T(""),_T("")};							// Define a new section
			Record r = {_T(""),_T(' '),SectionName,KeyName,Value};					// Define a new record
			content.push_back(s);											// Add the section
			content.push_back(r);											// Add the record
			return Save(FileName,content);									// Save
		}

		if(!RecordExists(KeyName,SectionName,FileName))						// If the Key doesn't exist
		{
			vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionIs(SectionName));					// Locate the Section
			iter++;															// Advance just past the section
			Record r = {_T(""),_T(' '),SectionName,KeyName,Value};						// Define a new record
			content.insert(iter,r);											// Add the record
			return Save(FileName,content);									// Save
		}

		vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Record

		iter->Value = Value;												// Insert the correct value
		return Save(FileName,content);										// Save
	}

	return false;															// In the event the file does not load
}

bool CIniFile::RenameSection(tstring OldSectionName, tstring NewSectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(vector<Record>::iterator iter = content.begin(); 
			iter < content.end(); iter++)									// Loop through the records
		{
			if(iter->Section == OldSectionName)								// Is this the OldSectionName?
				iter->Section = NewSectionName;								// Now its the NewSectionName
		}
		return Save(FileName,content);										// Save
	}

	return false;															// In the event the file does not load
}

bool CIniFile::CommentRecord(CommentChar cc, tstring KeyName,tstring SectionName,tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found
	
		iter->Commented = cc;										// Change the Comment value
		return Save(FileName,content);										// Save

	}
	return false;															// In the event the file does not load
}

bool CIniFile::UnCommentRecord(tstring KeyName,tstring SectionName,tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found
	
		iter->Commented = ' ';												// Remove the Comment value
		return Save(FileName,content);										// Save

	}
	return false;															// In the event the file does not load
}

bool CIniFile::CommentSection(char CommentChar, tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(vector<Record>::iterator iter = content.begin(); iter < content.end(); iter++)
		{
			if(iter->Section == SectionName)								// Is this the right section?
				iter->Commented = CommentChar;								// Change the comment value
		}
		return Save(FileName,content);										// Save
	}

	return false;															// In the event the file does not load
}

bool CIniFile::UnCommentSection(tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(vector<Record>::iterator iter = content.begin(); iter < content.end(); iter++)
		{
			if(iter->Section == SectionName)								// Is this the right section?
				iter->Commented = ' ';										// Remove the comment value
		}																	
		return Save(FileName,content);										// Save
	}

	return false;															// In the event the file does not load
}

bool CIniFile::DeleteRecord(tstring KeyName, tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found
	
		content.erase(iter);												// Remove the Record
		return Save(FileName,content);										// Save

	}
	
	return false;															// In the event the file does not load
}

bool CIniFile::DeleteSection(tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(int i=(int)content.size()-1;i>-1;i--)								// Iterate backwards through the content
		{							
			if(content[i].Section == SectionName)							// Is this related to the Section?
				content.erase (content.begin()+i);							// Then erase it
		}

		return Save(FileName,content);										// Save
	}
	return false;															// In the event the file does not load
}

bool CIniFile::SetSectionComments(tstring Comments, tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for(vector<Record>::iterator iter = content.begin(); iter < content.end(); iter++)									// Loop through the records
		{
			if((iter->Section == SectionName) &&							// Is this the Section?
				(iter->Key == _T("")))											// And not a record
			{	
				if (Comments.size() >= 2)									// Is there a comment?
				{
					if (Comments.substr(Comments.size()-2) != _T("\n"))		// Does the tstring end in a newline?
						Comments += _T("\n");								// If not, add one
				}
				iter->Comments = Comments;								// Set the comments
					
				return Save(FileName,content);							// Save
			}
		}
	}
	return false;															// In the event the file does not load
}

bool CIniFile::SetRecordComments(tstring Comments, tstring KeyName, tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		vector<Record>::iterator iter = std::find_if(content.begin(), 
				content.end(), 
				CIniFile::RecordSectionKeyIs(SectionName,KeyName));			// Locate the Section/Key

		if (iter == content.end()) return false;							// The Section/Key was not found
	
		if (Comments.size() >= 2)											// Is there a comment?
		{
			if (Comments.substr(Comments.size()-2) != _T("\n"))					// Does the tstring end in a newline?
				Comments += _T("\n");											// If not, add one
		}
		iter->Comments = Comments;											// Set the comments
		return Save(FileName,content);										// Save

	}
	
	return false;															// In the event the file does not load
}

vector<CIniFile::Record> CIniFile::GetSections(tstring FileName)
{
	vector<Record> data;													// Holds the return data
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		for (int i=0;i<(int)content.size();i++)								// Loop through the content
		{
			if(content[i].Key == _T(""))										// If this is a section 
				data.push_back(content[i]);									// Add the record to the return data
		}
	}
	
	return data;															// Return the data
}

bool CIniFile::Sort(tstring FileName, bool Descending)
{
	vector<CIniFile::Record> content;										// Used to hold the sorted content
	vector<CIniFile::Record> sections = GetSections(FileName);				// Get a list of Sections

	if(!sections.empty())													// Is there anything to process?
	{

		if(Descending)														// Descending or Ascending?
			std::sort(sections.begin(), sections.end(), DescendingSectionSort());
		else																// Sort the Sections
			std::sort(sections.begin(), sections.end(), AscendingSectionSort());

		for(vector<Record>::iterator iter = sections.begin(); iter < sections.end(); iter++) // For each Section
		{																		
			content.push_back(*iter);										// Add the sorted Section to the content

			vector<CIniFile::Record> records = GetSection(iter->Section ,FileName); // Get a list of Records for this section

			if(Descending)													// Descending or Ascending?
				std::sort(records.begin(), records.end(), DescendingRecordSort());
			else															// Sort the Records
				std::sort(records.begin(), records.end(), AscendingRecordSort());

			for(vector<Record>::iterator it = records.begin(); it < records.end(); it++) // For each Record
				content.push_back(*it);										// Add the sorted Record to the content
		}
		
		return Save(FileName,content);										// Save
		}

	return false;															// There were no sections
}

bool CIniFile::AddSection(tstring SectionName, tstring FileName)
{
	vector<Record> content;													// Holds the current record													// Holds the current record

	if (Load(FileName, content))											// Make sure the file is loaded
	{
		Record s = {_T(""),_T(' '),SectionName,_T(""),_T("")};								// Define a new section
		content.push_back(s);												// Add the section
		return Save(FileName,content);										// Save
	}

	return false;															// The file did not open
}

bool CIniFile::Create(tstring FileName)
{
	vector<Record> content;													// Create empty content
	return Save(FileName,content);											// Save
}

