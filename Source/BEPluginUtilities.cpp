/*
 BEPluginUtilities.cpp
 BaseElements Plug-In
 
 Copyright 2010-2011 Goya. All rights reserved.
 For conditions of distribution and use please see the copyright notice in BEPlugin.cpp
 
 http://www.goya.com.au/baseelements/plugin
 
 */


#include "BEPluginGlobalDefines.h"
#include "BEPluginUtilities.h"


#if defined(FMX_WIN_TARGET)

	#include "afx.h"
	#include "resource.h"

#endif


#if defined(FMX_MAC_TARGET)

	#include <CoreServices/CoreServices.h>
	#include <Carbon/Carbon.h>

#endif 


#include "FMWrapper/FMXData.h"
#include "FMWrapper/FMXFixPt.h"

#include "boost/format.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

#include <iostream>
#include <iconv.h>


using namespace boost::filesystem;


#pragma mark -
#pragma mark Plug-In Utilities
#pragma mark -

// convenience functions that handle most of the work needed to return text from a function

fmx::errcode TextConstantFunction ( wstring text, fmx::Data& results )
{
	
	WStringAutoPtr text_constant ( new wstring ( text ) );
	
	return TextConstantFunction ( text_constant, results );	
		
} // TextConstantFunction


fmx::errcode TextConstantFunction ( WStringAutoPtr text, fmx::Data& results )
{
	
	fmx::errcode error_result = kNoError;
	
	try {
		
		fmx::TextAutoPtr result_text;
		result_text->AssignWide ( text->c_str() );
		
		fmx::LocaleAutoPtr default_locale;
		results.SetAsText ( *result_text, *default_locale );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // TextConstantFunction


// get parameters and set function results

void SetNumericResult ( long number, fmx::Data& results )
{
	fmx::FixPtAutoPtr numeric_result;
	numeric_result->AssignInt ( number );
	results.SetAsNumber ( *numeric_result );
}


void SetUTF8Result ( StringAutoPtr text, fmx::Data& results )
{
	fmx::TextAutoPtr result_text;
	result_text->Assign ( text->c_str(), fmx::Text::kEncoding_UTF8 );			
	fmx::LocaleAutoPtr default_locale;
	results.SetAsText ( *result_text, *default_locale );
}


void SetWideResult ( WStringAutoPtr text, fmx::Data& results )
{
	fmx::TextAutoPtr result_text;
	result_text->AssignWide ( text->c_str() );			
	fmx::LocaleAutoPtr default_locale;
	results.SetAsText ( *result_text, *default_locale );
}


bool ParameterAsBoolean ( const fmx::DataVect& data_vect, const unsigned long which, const bool default_value )
{
	try {
		return data_vect.AtAsBoolean ( which );
	} catch ( exception e ) {
		return default_value;
	}

}
	
	
long ParameterAsLong ( const fmx::DataVect& data_vect, const unsigned long which, const unsigned long default_value )
{
	try {
		return data_vect.AtAsNumber ( which ).AsLong();
	} catch ( exception e ) {
		return default_value;
	}
	
}


StringAutoPtr ParameterAsUTF8String ( const fmx::DataVect& data_vect, unsigned long which )
{	
	
	StringAutoPtr result ( new string );
	
	try {
		
		fmx::TextAutoPtr raw_data;
		raw_data->SetText ( data_vect.AtAsText ( which ) );
		
		fmx::ulong text_size = (2*(raw_data->GetSize())) + 1;
		char * text = new char [ text_size ];
		raw_data->GetBytes ( text, text_size, 0, (ulong)fmx::Text::kSize_End, fmx::Text::kEncoding_UTF8 );
		result->assign ( text );
		delete [] text;
		
	} catch ( exception e ) {
		;	// return an empty string
	}
	
	return result;
	
} // ParameterAsUTF8String



WStringAutoPtr ParameterAsWideString ( const fmx::DataVect& data_vect, unsigned long which )
{	
	
	WStringAutoPtr result ( new wstring );
	
	try {
		
		fmx::TextAutoPtr raw_data;
		raw_data->SetText ( data_vect.AtAsText(which) );
		
		long text_size = raw_data->GetSize();
		fmx::ushort * text = new fmx::ushort [ text_size + 1 ];
		raw_data->GetUnicode ( text, 0, text_size );
        text[text_size] = 0x0000;

		// wchar_t is 4 bytes on OS X and 2 on Windows
		
		#if defined(FMX_MAC_TARGET)
		
			wchar_t * parameter = new wchar_t [ text_size + 1 ];
			for ( long i = 0 ; i <= text_size ; i++ ) {
				parameter[i] = (wchar_t)text[i];
			}
			delete [] text;
		
		#endif 
		
		#if defined(FMX_WIN_TARGET)
		
			wchar_t * parameter = (wchar_t*)text;
		
		#endif
		
		result->append ( parameter );
		delete [] parameter; // parameter == text on Windows
				
	} catch ( exception e ) {
		;	// return an empty string
	}
	
	return result;
	
} // ParameterAsUnicodeString


#pragma mark -
#pragma mark Files
#pragma mark -

StringAutoPtr ReadFileAsUTF8 ( WStringAutoPtr path )
{
	
	boost::filesystem::path filesystem_path = *path;
	size_t length = (size_t)file_size ( filesystem_path ); // boost::uintmax_t

	boost::filesystem::ifstream inFile ( filesystem_path, ios_base::in | ios_base::binary | ios_base::ate );
	inFile.seekg ( 0, ios::beg );

	// slurp up the file contents
	char * buffer = new char [length + 1];
	inFile.read ( buffer, length );
	inFile.close ();
	
	buffer[length] = '\0';
	
	// convert the text in the file to utf-8 if possible
	StringAutoPtr result = ConvertToUTF8 ( buffer, length );
	if ( result->length() == 0 ) {
		result->assign ( buffer );
	}
	delete [] buffer;
	
	return result;

} // ReadFileAsUTF8


#pragma mark -
#pragma mark Unicode
#pragma mark -

// convert text to utf-8
// currently handles utf-16, ascii and utf-8 text

StringAutoPtr ConvertToUTF8 ( char * in, size_t length )
{
	size_t utf8_length = (length * 2) + 1;	// worst case for utf-16 to utf-8
	char * utf8 = new char [utf8_length]();	// value-initialization (to zero)… we crash otherwise
	
	vector<string> codesets;
	codesets.push_back ( "UTF-8" );
	codesets.push_back ( "UTF-16" );

	int error_result = -1;
	
	/*
	 there no clean way to determine the codeset of the supplied text so
	 try each converting from each of the codesets in turn
	 */
	
	vector<string>::iterator it = codesets.begin();
	while ( error_result == -1 && it != codesets.end() ) {

		char * start = in;
		size_t start_length = length;
		char * utf8_start = utf8;
	
		iconv_t conversion = iconv_open ( "UTF-8", it->c_str() );
		error_result = iconv ( conversion, &start, &start_length, &utf8_start, &utf8_length );
		iconv_close ( conversion );
	
		++it;
	}
	
	StringAutoPtr out ( new string ( utf8 ) );
	delete[] utf8;
	
	return out;
	
} // ConvertToUTF8


#pragma mark -
#pragma mark Other
#pragma mark -

/*
 The following functions are modifications of ones included in the Example plug-in
 shipped by FileMaker Inc. with the External Plug-In API on the CDs for 
 FileMaker Developer/Pro Advance versions 7 through 10 and are used by permission.
 */

void Do_GetString(unsigned long whichString, FMX_ULong /* winLangID */, FMX_Long resultsize, FMX_Unichar* string)
{
	
	switch ( whichString )
	{
		case kFMXT_OptionsStr:
		{
#if defined(FMX_WIN_TARGET)
			LoadStringW( (HINSTANCE)(gFMX_ExternCallPtr->instanceID), kBE_OptionsStringID, (LPWSTR)string, resultsize);
#endif
			
#if defined(FMX_MAC_TARGET)
			Sub_OSXLoadString(kBE_OptionsStringID, string, resultsize);
#endif

//			processedSpecialStringID = true;
			break;
		}
		default:
#if defined(FMX_WIN_TARGET)
			LoadStringW( (HINSTANCE)(gFMX_ExternCallPtr->instanceID), (unsigned int)whichString, (LPWSTR)string, resultsize);
#endif
				
#if defined(FMX_MAC_TARGET)
			Sub_OSXLoadString(whichString, string, resultsize);
#endif
	
	} // switch (whichString)
	
	
} // Do_GetString (FMX_Unichar* version)


void Do_GetString(unsigned long whichStringID, fmx::TextAutoPtr& intoHere, bool stripFunctionParams)
{
	FMX_Unichar			tempBuffer[kBE_GetStringMaxBufferSize];
	
	Do_GetString ( whichStringID, 0, kBE_GetStringMaxBufferSize, tempBuffer );
	intoHere->AssignUnicode(tempBuffer);
	
	if(stripFunctionParams) {
		
		// The string for this whichStringID is a Function Prototype, but all the plug-in needs now is the Function Name by itself.
		
		fmx::TextAutoPtr		parenToken;
		parenToken->Assign ( " (" );
		
		unsigned long		originalSize = intoHere->GetSize();
		unsigned long		firstParenLocation; 
		firstParenLocation = intoHere->Find(*parenToken, 0);
		
		intoHere->DeleteText(firstParenLocation, originalSize-firstParenLocation);
		
	} // stripFunctionParams
	
} // Do_GetString (TextAutoPtr version)



#if defined(FMX_MAC_TARGET)

unsigned long Sub_OSXLoadString(unsigned long stringID, FMX_Unichar* intoHere, long intoHereMax)
{
	unsigned long		returnResult = 0;
	
	
	if( (intoHere != NULL) && (intoHereMax > 1) ) {
		// Turn stringID to a textual identifier, then get the string from the .strings file as a null-term unichar array.
		CFStringRef 	strIdStr = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("%d"), stringID );
		
		// Note: The plug-in must be explicit about the bundle and file it wants to pull the string from.
		CFStringRef 	osxStr = CFBundleCopyLocalizedString( reinterpret_cast<CFBundleRef>(gFMX_ExternCallPtr->instanceID), strIdStr, strIdStr, CFSTR( PLUGIN_NAME ) );
		
		if((osxStr != NULL) && (osxStr != strIdStr)) {
			
			long	osxStrLen = CFStringGetLength(osxStr);
			
			if( osxStrLen < (intoHereMax-1) ) {
				
				CFRange		allChars;
				allChars.location = 0;
				allChars.length = osxStrLen;
				
				CFStringGetCharacters(osxStr, allChars, (UniChar*)(intoHere));
				intoHere[osxStrLen] = 0x0000;
				returnResult = (unsigned long)osxStrLen;
				
			} // osxStrLen
			
			CFRelease( osxStr );
			
		} // osxStr
		
		CFRelease( strIdStr );
		
	} // intoHere
	
	return(returnResult);
	
} // Sub_OSXLoadString

#endif


