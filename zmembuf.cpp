/** @file *//********************************************************************************************************

                                                    zmembuf.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/Libraries/zstream/zmembuf.cpp#5 $

	$NoKeywords: $

*********************************************************************************************************************/

#include "zmembuf.h"

#include "zlib/zlib.h"

#include <streambuf>
#include <vector>
#include <cassert>



/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Mode	Direction of the stream
//!					- <tt>std::ios_base::in</tt> signifies an input buffer. Data is decompressed as it is streamed
//!						from of this buffer. You must initialize the contents of the buffer before streaming.
//!					- <tt>std::ios_base::out</tt> signifies an ouput buffer. Data is compressed as it is streamed
//!						to this buffer. The buffer will grow as data is streamed to it.

zmembuf::zmembuf( std::ios_base::openmode _Mode )
{
	_Init( _Mydata(), _Getstate( _Mode ) );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Data	Initial contents of the buffer.
//! @param	_Mode	Direction of the stream
//!					- <tt>std::ios_base::in</tt> signifies an input buffer. The contents are decompressed as they
//!						are streamed from the buffer.
//!					- <tt>std::ios_base::out</tt> signifies an ouput buffer. Data streamed to this buffer is
//!						compressed and appended to the initial contents.

zmembuf::zmembuf( _Mydata const &			_Data,
				  std::ios_base::openmode	_Mode )
{
	_Init( _Data, _Getstate( _Mode ) );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

zmembuf::~zmembuf()
{
	_Tidy();
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @warning	The returned data is valid only until the next operation on the buffer.
//! @note		This function forces zlib to flush its buffers. Frequent calls to this function will degrade
//!				performance.

zmembuf::_Mydata const & zmembuf::buffer() const
{
	// Make sure the buffer is synced before giving access to it.

	sync();

	return _Data;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//!
//! @param	_Newdata	Data replacing the current contents of the buffer.

void zmembuf::buffer( _Mydata const & _Newdata )
{
	_Tidy();
	_Init( _Newdata, _Mystate );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Newdata	Data replacing the current contents of the buffer.
//! @param	size		size of the data (in bytes)

void zmembuf::buffer( char_type const * _Newdata, size_t size )
{
	_Tidy();
	_Init( _Mydata( &_Newdata[0], &_Newdata[size] ), _Mystate );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//!
//! @param	level	Compression level. 0 is no compression, 9 is maximum compression.

void zmembuf::set_compression( int level )
{
	if ( level < 0 )
	{
		level = 0;
	}
	else if ( level > 9 )
	{
		level = 9;
	}

	deflateParams( &_zstream, level, Z_DEFAULT_STRATEGY );
}

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//!
//! @param	_Meta	Value to put into the buffer

zmembuf::int_type zmembuf::overflow( int_type _Meta/* = traits_type::eof()*/ )
{
	// If the character to store is EOF, then do nothing and return success

	if ( traits_type::eof() == _Meta )
	{
		return traits_type::not_eof( _Meta );
	}

	// Otherwise, if the data is read-only, then return fail

	else if ( _Mystate & _Read )
	{
		return traits_type::eof();
	}

	// Otherwise, add the value to the data

	else
	{
		// First make sure that any unprocessed data has been processed

		flushinput();

		// Send the value to the compressor

		Bytef c	= (char_type)_Meta;
		_zstream.next_in = &c;
		_zstream.avail_in = sizeof( c );

		grow( sizeof( c ) + 1 );
		deflate( &_zstream, 0 );

		return _Meta;
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Meta	Value to put back. If it is <tt>traits_type::eof()</tt>, then put back the character was read
//!					last (if possible).
//!
//! @note	The character put back becomes the current character

zmembuf::int_type zmembuf::pbackfail( int_type _Meta/* = traits_type::eof()*/ )
{
	// If there is a input buffer and there is data before the current positon, and _Meta is EOF or the same as
	// the previous character in the input buffer, just back up the current position 

	if ( _Mysb::gptr() != 0 && _Mysb::eback() < _Mysb::gptr() &&
		 ( _Meta == traits_type::eof() || int_type( _Mysb::gptr() [ -1 ] ) == _Meta ) )
	{
		_Mysb::_Gndec();
		return traits_type::not_eof( _Meta );
	}

	// Otherwise, if the file isn't open or a EOF is put back return error.

	else if ( _Myfile == 0 || _Meta == traits_type::eof() )
	{
		return traits_type::eof();
	}

	// Otherwise, put the data in the putback buffer

	else
	{
		_Mychar = char_type( _Meta );
		_Mysb::setg( &_Mychar, &_Mychar, &_Mychar + 1 );

		return _Meta;
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

std::streamsize zmembuf::showmanyc()
{
	if ( _Mysb::gptr() != 0 )
	{
		return ( _Mysb::egptr() - _Mysb::gptr() );
	}
	else
	{
		return 0;
	}
}

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

zmembuf::int_type zmembuf::underflow()
{
	int_type	_Meta;

	// If there is data in the input buffer, get it without incrementing the pointer

	if ( _Mysb::gptr() != 0 && _Mysb::gptr() < _Mysb::egptr() )
	{
		_Meta = int_type( *_Mysb::gptr() );
	}

	// Otherwise, get a byte from zlib

	else
	{
		_Meta = uflow();
		
		// If it did not fail, the put the byte back

		if ( _Meta != traits_type::eof() )
		{
			pbackfail( _Meta );
		}

	}

	return _Meta;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Ptr	buffer to store streamed data
//! @param	_Count	Number of uncompressed bytes to get

std::streamsize zmembuf::xsgetn( char_type * _Ptr, std::streamsize _Count )
{
	return _Count;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Ptr	Uncompressed data to put
//! @param	_Count	Number of uncompressed bytes to put

std::streamsize zmembuf::xsputn( char_type const * _Ptr, std::streamsize _Count )
{
	// First make sure that any unprocessed data has been processed

	flushinput();

	// Send the data to zlib

	_zstream.next_in = reinterpret_cast<Bytef const *>( _Ptr );
	_zstream.avail_in = _Count;

	grow( _Count + 1 );
	deflate( &_zstream, 0 );

	return _Count;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Off	Number of uncompressed bytes to move the pointer
//! @param	_Way	Location to start seek. <tt>std::ios_base::end</tt> is not supported. Valid values are:
//!						- <tt>std::ios_base::beg</tt>
//!						- <tt>std::ios_base::cur</tt>
//! @param	_Which	Ignored (both in and out pointers are moved)
//! @note	There are restrictions imposed by @c zlib:
//!				-#	<tt>std::ios_base::end</tt> is not supported as a start location
//!				-#	Only forward seeks are allowed in output buffers

zmembuf::pos_type zmembuf::seekoff( off_type				_Off,
									std::ios_base::seekdir	_Way,
									std::ios_base::openmode	_Which	/* = std::ios_base::in | std::ios_base::out*/ )
{
	pos_type	_Pos;

	// Make sure all output is done before trying to position the pointer

	sync();

	// position within read buffer

	if ( _Mysb::gptr() != 0 )
	{
		if ( _Way == std::ios_base::end )
		{
			_Off = std::_BADOFF;
		}
		else if ( _Way == std::ios_base::cur )
		{
			_Off += off_type( _Mysb::gptr() - _Mysb::eback() );
		}
		else if ( _Way == std::ios_base::beg )
		{
		}
		else
		{
			_Off = std::_BADOFF;
		}

		// change read position
		if ( 0 <= _Off && _Off <= _Mysb::egptr() - _Mysb::eback() )
		{
			_Mysb::gbump( ( int ) ( _Mysb::eback() + _Off - _Mysb::gptr() ) );
		}
		else
		{
			_Off = std::_BADOFF;
		}
	}

	// Otherwise, if this is a write buffer, position the pointer

	else if ( _Mysb::pptr() != 0 )
	{
		// Figure out the offset from the beginning of the buffer and adjust _Off so that it is the number of bytes
		// from the current position.

		if ( _Way == std::ios_base::beg )
		{
			_Pos = pos_type( _Off );
			_Off -= off_type( _zstream.total_out );
		}
		else if ( _Way == std::ios_base::cur )
		{
			_Pos = pos_type( _Off + off_type( _zstream.total_out ) );
			_Off += 0;
		}
		else
		{
			_Pos = pos_type( std::_BADOFF );
			_Off = -1;
		}

		// Change write position by inserting _Off bytes of zeroe. Only forward seeks are allowed.

		if ( 0 <= _Off )
		{
			// Send _Off bytes of dummy data to zlib

			grow( _Off + 1 );
			while( _Off > 0 )
			{
				Bytef	dummy	= 0;
				_zstream.next_in = &dummy;
				_zstream.avail_in = 1;

				deflate( &_zstream, 0 );

				--_Off;
			}
		}
		else
		{
			_Pos = pos_type( std::_BADOFF );
		}
	}
	else
	{
		_Pos = pos_type( std::_BADOFF );	// neither read nor write buffer selected, fail
	}

	return _Pos;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Pos	Location to move the pointer
//! @param	_Mode	Ignored (both in and out pointers are moved)
//! @note	There are restrictions imposed by @c zlib:
//!				-#	<tt>std::ios_base::end</tt> is not supported as a start location
//!				-#	Only forward seeks are allowed in output buffers

zmembuf::pos_type zmembuf::seekpos( pos_type				_Ptr,
									std::ios_base::openmode	_Mode	/* = std::ios_base::in | std::ios_base::out*/ )
{
	off_type	_Off	= off_type( _Ptr );

	// position within read buffer

	if ( _Mysb::gptr() != 0 )
	{
		if ( 0 <= _Off && _Off <= _Mysb::egptr() - _Mysb::eback() )
		{
			_Mysb::gbump( ( int ) ( _Mysb::eback() + _Off - _Mysb::gptr() ) );
		}
		else
		{
			_Off = std::_BADOFF;
		}
	}
	// position within write buffer
	else if ( _Mysb::pptr() != 0 )
	{
		if ( 0 <= _Off && _Off <= _Mysb::epptr() - _Mysb::pbase() )
		{
			_Mysb::pbump( ( int ) ( _Mysb::pbase() + _Off - _Mysb::pptr() ) );
		}
		else
		{
			_Off = std::_BADOFF;
		}
	}
	else
	{
		_Off = std::_BADOFF;	// neither read nor write buffer selected, fail
	}

	return pos_type( _Off );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

int zmembuf::sync() const
{
	if ( _Mystate == _Read )
	{
		inflateEnd();
	}
	else
	{
		deflateEnd();
	}

	_Data.resize( _zstream.total_out );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! @param	_Newdata	Initial contents of the buffer
//! @param	_State		Read or write state of the buffer

void zmembuf::_Init( _Mydata const & _Newdata, _Strstate _State )
{
	_Mystate = _State;

	_Data.clear();
	setg( 0, 0, 0 );
	setp( 0, 0, 0 );

	if ( _Newdata.size() > 0 )
	{
		_Data.assign( _Newdata.begin(), _Newdata.end()-1 );

		if ( _Mystate == _Read )
		{
			int		ok;

			// Initialize zlib for deflating (use default memory allocation)

			_zstream.zalloc	= Z_NULL;
			_zstream.zfree	= Z_NULL;
			_zstream.opaque	= Z_NULL;
			ok = inflateInit( &_zstream );
			assert( ok );

			// Set the stream buffer pointers

			_Mysb::setg( &*_Data.begin(), &*_Data.begin(), &*_Data.end() );
		}
		else // if ( _Mystate == _Write )
		{
			int		ok;

			// Initialize zlib for deflating (use default memory allocation)

			_zstream.zalloc	= Z_NULL;
			_zstream.zfree	= Z_NULL;
			_zstream.opaque	= Z_NULL;
			ok = deflateInit( &_zstream, Z_DEFAULT_COMPRESSION );
			assert( ok );

			// Set the stream buffer pointers

			_Mysb::setp( &*_Data.begin(), &*_Data.end(), &*_Data.end() );
		}
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void zmembuf::_Tidy()
{
	// Make sure the data is synced before shutting it down or replacing it.

	sync();
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//!
//! @param	_Mode	Open mode

zmembuf::_Strstate zmembuf::_Getstate( std::ios_base::openmode _Mode )
{
	return ( ( ( _Mode & std::ios_base::out ) != 0 ) ? _Write : _Read );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//!
//! @param	size	Number of bytes to make room for

void zmembuf::grow( size_t size )
{
	_Data.resize( _zstream.total_out + size );

	_zstream.next_out = &_Data[ _zstream.total_out ];
	_zstream.avail_out = size;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void zmembuf::flushinput()
{
	while ( _zstream.avail_in > 0 )
	{
		grow( _zstream.avail_in + 1 );
		deflate( _zstream, 0 );
	}
}

