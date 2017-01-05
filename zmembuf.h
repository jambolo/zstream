/** @file *//********************************************************************************************************
 
                                                     zmembuf.h
 
						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------
 
	$Header: //depot/Libraries/zstream/zmembuf.h#4 $
 
	$NoKeywords: $
 
*********************************************************************************************************************/

#pragma once

#include "zlib/zlib.h"
#include <vector>
#include <streambuf>


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! A memory stream buffer that compresses and decompresses data using @c zlib

class zmembuf : public std::basic_streambuf< unsigned char, std::char_traits< unsigned char > >
{

public:
	typedef unsigned char							char_type;		//!< Element type
	typedef std::char_traits< char_type >			traits_type;	//!< The element's traits

	typedef zmembuf											_Myt;			//!< This class
	typedef std::basic_streambuf< char_type, traits_type >	_Mysb;			//!< The streambuf base class
	typedef std::vector< char_type >						_Mydata;		//!< The data container class

	typedef traits_type::int_type					int_type;		//!< Holds values not representable by char_type
	typedef traits_type::pos_type				 	pos_type;		//!< Holds a buffer position
	typedef traits_type::off_type				 	off_type;		//!< Holds a buffer offset

	//! Stream state bits
	enum StreamStateBit
	{
	    _Read		= 0, 	//!< Character array nonmutable
	    _Write		= 1		//!< Character array cannot be read
	};
	typedef int _Strstate;	//!< Stream state

	//! Constructor
	explicit zmembuf( std::ios_base::openmode _Mode );

	//! Constructor
	zmembuf( _Mydata const & _Newdata, std::ios_base::openmode _Mode );

	//! Constructor
	zmembuf( char_type const * _Newdata, size_t _Size, std::ios_base::openmode _Mode );

	//! Destructor
	virtual ~zmembuf();

	//! Returns a reference to the data in the buffer.
	_Mydata const & buffer() const;

	//! Replaces the current data in the buffer.
	void buffer( _Mydata const & _Newdata );

	//! Replaces the current data in the buffer.
	void buffer( char_type const * _Newdata, size_t _Size );

	//! Sets the compression level.
	void set_compression( int level );

protected:

	//! @name Overrides basic_streambuf
	//@{

	//! Inserts the character into the buffer. Returns the character or traits_type::eof() if it failed.
	virtual int_type overflow( int_type _Meta = traits_type::eof() );

	//! Puts a character back into the buffer. Returns the character or traits_type::eof() if it failed.
	virtual int_type pbackfail( int_type _Meta = traits_type::eof() );

	virtual std::streamsize showmanyc();

	//! Returns the current character from the buffer (primarily when it is empty).
	virtual int_type underflow();

//	virtual int_type uflow();

	//! Reads @a _Count uncompressed characters from the buffer. Returns the number of characters actually read.
	virtual std::streamsize xsgetn( char_type * _Ptr, std::streamsize _Count);

	//! Writes @a _Count uncompressed characters to the buffer. Returns the number of characters actually written.
	virtual std::streamsize xsputn( char_type const * _Ptr, std::streamsize _Count);

	//! Sets the current position relative to a specific point in the buffer. Returns the new position.
	virtual pos_type seekoff( off_type _Off,
	                          std::ios_base::seekdir _Way,
	                          std::ios_base::openmode = ( std::ios_base::openmode ) ( std::ios_base::in | std::ios_base::out ) );

	//! Sets the current position to a previous location. Returns the new position.
	virtual pos_type seekpos( pos_type _Pos,
	                          std::ios_base::openmode = ( std::ios_base::openmode ) ( std::ios_base::in | std::ios_base::out ) );

//	virtual _Mysb * setbuf( char_type * _Buffer, std::streamsize _Count );

	//! Syncs the buffer, ensuring that all the data has been compressed/decompressed. Returns -1 if it failed.
	virtual int sync() const;

//	virtual void imbue(const locale&);

	//@}

	//! Initialization
	void _Init( _Mydata const & _Newbuf, _Strstate _State );

	//! Cleanup
	void _Tidy();

private:

	// Returns a stream state converted from an open mode
	_Strstate _Getstate( std::ios_base::openmode _Mode );

	// Make sure there is enough room in the container to hold this many bytes of additional output
	void grow( size_t size );

	void flushinput();

	_Strstate		_Mystate;	// The stream state
	_Mydata			_Data;		// Memory buffer holding the compressed/decompressed data
	z_stream		_zstream;	// The zlib stream state
};
