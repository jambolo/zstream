/** @file *//********************************************************************************************************

                                                     zfilebuf.h

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/Libraries/zstream/zfilebuf.h#5 $

	$NoKeywords: $


*********************************************************************************************************************/

#pragma once

#include "zlib/zlib.h"
#include <streambuf>


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! A file stream buffer that compresses and decompresses the data using @c zlib

class zfilebuf : public std::basic_streambuf< unsigned char, std::char_traits< unsigned char > >
{

public:
	typedef unsigned char							char_type;		//!< Element type
	typedef std::char_traits< unsigned char >		traits_type;	//!< The element's traits (not used, included
																	//!  for completeness.
																								
	typedef zfilebuf										_Myt;	//!< This class
	typedef std::basic_streambuf< char_type, traits_type >	_Mysb;	//!< The streambuf base class

	typedef traits_type::int_type					int_type;		//!< Holds info not representable by char_type
	typedef traits_type::pos_type					pos_type;		//!< Holds a buffer position
	typedef traits_type::off_type					off_type;		//!< Holds a buffer offset

	//! Reasons for a call to _Init
	enum _Initfl
	{
	    _Newfl,			//!< Initialization
		_Openfl,		//!< File opened
		_Closefl		//!< File closed
	};

	//! Constructor
	zfilebuf( gzFile _File = 0 );

	//! Constructor
	zfilebuf( std::_Uninitialized );

	//! Destructor
	virtual ~zfilebuf();

	//! Returns @c true if the file has been opened
	bool is_open() const
	{
		return ( _Myfile != 0 );
	}

	//! Opens a file. Returns @c this.
	_Myt * open( char const *_Filename, std::ios_base::openmode _Mode );

	//! Closes the file. Returns @c this.
	_Myt * close();

	//! Sets the compression level. Returns @c *this.
	void set_compression( int level );

protected:

	//! @name Overrides basic_streambuf
	//@{

	//! Inserts the character into the buffer (primarily when it is full). Returns the character or
	//! <tt>traits_type::eof()</tt> if it failed.
	virtual int_type overflow( int_type _Meta = traits_type::eof() );

	//! Puts a character back into the buffer, then makes it the current character. Returns the character or
	//! <tt>traits_type::eof()</tt> if it failed.
	virtual int_type pbackfail( int_type _Meta = traits_type::eof() );

//	virtual std::streamsize showmanyc();

	//! Returns the current character from the buffer (primarily when it is empty).
	virtual int_type underflow();

	//! Returns the current character from the buffer (primarily when it is empty) and points to the next character.
	virtual int_type uflow();

	//! Reads @p _Count characters from the buffer. Returns the number of characters actually read.
	virtual std::streamsize xsgetn( char_type * _Ptr, std::streamsize _Count);

	//! Writes @p _Count characters to the buffer. Returns the number of characters actually written.
	virtual std::streamsize xsputn( char_type const * _Ptr, std::streamsize _Count);

	//! Sets the current position relative to a specific point in the file. Returns the new position.
	virtual pos_type seekoff( off_type _Off,
	                          std::ios_base::seekdir _Way,
	                          std::ios_base::openmode = ( std::ios_base::openmode ) ( std::ios_base::in | std::ios_base::out ) );

	//! Sets the current position to a previous location. Returns the new position.
	virtual pos_type seekpos( pos_type _Pos,
	                          std::ios_base::openmode = ( std::ios_base::openmode ) ( std::ios_base::in | std::ios_base::out ) );

	//! Provides a buffer. Returns @c this.
	virtual _Mysb * setbuf( char_type * _Buffer, std::streamsize _Count );

	//! Synchronizes the buffer with the file.
	virtual int sync();

//	virtual void imbue(const locale&);

	//@}

	//! Initialization.
	void _Init( gzFile _File, _Initfl _Which );

private:
	char_type				_Mychar;	//!< Putback character
	bool					_Closef;	//!< True if file must be closed
	gzFile					_Myfile;	//!< gz file pointer
};
