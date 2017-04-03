/** @file *//********************************************************************************************************
 
                                                     zmstream.h
 
						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------
 
	$Header: //depot/Libraries/zstream/zmstream.h#5 $
 
	$NoKeywords: $
 
*********************************************************************************************************************/

#pragma once

#include "zmembuf.h"

#include <istream>
#include <ostream>
#include <vector>
#include <iomanip>

//! An input stream that decompresses the data using @c zlib from a buffer

class izmstream : public std::basic_istream< unsigned char, std::char_traits< unsigned char > >
{
public:

	typedef unsigned char			char_type;			//!< Element type
	typedef zmembuf					zmembuf;				//!< The stream buffer class
	typedef zmembuf::container_type		_Mydata;			//!< The container class

	//! Constructor
	izmstream();

	//!Constructor
	explicit izmstream( _Mydata const &	_Newbuf );

	//! Destructor
	virtual ~izmstream();

	//! Returns a pointer to the stream buffer.
	zmembuf *rdbuf()							const	{ return const_cast< zmembuf * >( &_Membuffer ); }

	//! Returns the contents of the memory buffer.
	_Mydata const & buffer()				const	{ return _Membuffer.buffer(); }

	//! Replaces the contents of the memory buffer.
	void buffer( _Mydata const & _Newbuf )			{ _Membuffer.buffer( _Newbuf ); }

private:

	zmembuf _Membuffer;		//!< The memory buffer
};


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! An output stream that compresses the data using @c zlib into a memory buffer

class ozmstream : public std::basic_ostream< unsigned char, std::char_traits< unsigned char > >
{
public:

	typedef unsigned char			char_type;			//!< Element type
	typedef zmembuf					_Mysb;				//!< The stream buffer class
	typedef zmembuf::container_type		_Mydata;			//!< The container class

	//! Constructor
	ozmstream();

	//! Destructor
	virtual ~ozmstream();

	//! Returns a pointer to the stream buffer.
	_Mysb * rdbuf()							const	{ return const_cast< _Mysb * >( &_Membuffer ); }

	//! Returns the contents of the memory buffer.
	_Mydata const & buffer()				const	{ return _Membuffer.buffer(); }

	//! Replaces the contents of the memory buffer.
	void buffer( _Mydata const & _Newbuf )			{ _Membuffer.buffer( _Newbuf ); }

	//! Sets the compression level.

	//!
	//! @param	level	Compression level. 0 is no compression, 9 is maximum compression.
	void set_compression( int level )				{ _Membuffer.set_compression( level ); }

private:

	_Mysb _Membuffer;		//!< The memory buffer
};
