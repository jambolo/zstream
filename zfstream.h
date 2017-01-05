/** @file *//********************************************************************************************************
 
                                                     zfstream.h
 
						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------
 
	$Header: //depot/Libraries/zstream/zfstream.h#6 $
 
	$NoKeywords: $
 
*********************************************************************************************************************/

#pragma once

#include "zfilebuf.h"

#include <istream>
#include <ostream>


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! An input stream that decompresses the data using @c zlib from a file.

class izfstream : public std::basic_istream< unsigned char, std::char_traits< unsigned char > >
{
public:
	typedef	unsigned char								char_type;		//!< Element type
	typedef std::char_traits< unsigned char >			traits_type;	//!< The element type's traits (not used,
																		//!  included for completeness)

	typedef izfstream									_Myt;			//!< This class
	typedef zfilebuf									_Myfb;			//!< The stream buffer class
	typedef std::basic_ios< char_type, traits_type >	_Myios;			//!< The ios class

	//! Constructor
	explicit izfstream( char const * _Filename = 0 );

	//! Destructor
	virtual ~izfstream();

	//! Returns a pointer to the file buffer
	_Myfb * rdbuf()					const	{ return const_cast< _Myfb * >( &_Filebuffer ); }

	//! Returns true if the file is open
	bool is_open()					const	{ return ( _Filebuffer.is_open() ); }

	//! Opens a file
	void open( const char *_Filename );

	//! Closes the file
	void close();

private:
	_Myfb	_Filebuffer;		//!< The file buffer
};


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! A output stream that compresses the data using @c zlib to a file.

class ozfstream : public std::basic_ostream< unsigned char, std::char_traits< unsigned char > >
{
public:
	typedef	unsigned char								char_type;		//!< Element type
	typedef std::char_traits< unsigned char >			traits_type;	//!< The element type's traits (not used,
																		//!  included for completeness)
														
	typedef ozfstream									_Myt;			//!< This class
	typedef zfilebuf									_Myfb;			//!< The stream buffer class
	typedef std::basic_ios< char_type, traits_type >	_Myios;			//!< The ios class

	//! Constructor
	explicit ozfstream( const char * _Filename = 0 );

	//! Destructor
	virtual ~ozfstream();

	//! Returns a pointer to filebuffer
	_Myfb * rdbuf()							const	{ return const_cast< _Myfb * >( &_Filebuffer ); }

	//! Returns true if a file is opened
	bool is_open()							const	{ return ( _Filebuffer.is_open() ); }

	//! Opens a file
	void open( char const * _Filename );

	//! Closes the file
	void close();

	//! Sets the compression level.

	//!
	//! @param	level	Compression level. 0 is no compression, 9 is maximum compression.
	void set_compression( int level )		{ _Filebuffer.set_compression( level );; }

private:
	_Myfb	_Filebuffer;	//!< The file buffer
};
