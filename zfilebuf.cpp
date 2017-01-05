/** @file *//********************************************************************************************************

                                                    zfilebuf.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/Libraries/zstream/zfilebuf.cpp#4 $

	$NoKeywords: $

*********************************************************************************************************************/

#include "zfilebuf.h"

#include "zlib/zlib.h"

#include <streambuf>

#include <fstream>


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

/// Construct from pointer to a file
///
zfilebuf::zfilebuf( gzFile _File/* = 0*/ )
	: _Mysb()
{
	_Init( _File, _Newfl );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

/// Construct unitialized
///
zfilebuf::zfilebuf( std::_Uninitialized )
	: _Mysb( std::_Noinit )
{
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

zfilebuf::~zfilebuf()
{
	if ( _Closef )
	{
		close();
	}
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

///
/// @param	level	Compression level. 0 is no compression, 9 is maximum compression.

void zfilebuf::set_compression( int level )
{
	if ( level < 0 )
	{
		level = 0;
	}
	else if ( level > 9 )
	{
		level = 9;
	}

	gzsetparams( _Myfile, level, Z_DEFAULT_STRATEGY );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

/// @param	_Filename	Name of the file to open.
/// @param	_Mode		Open mode. Only <tt>std::ios_base::in</tt> and <tt>std::ios_base::out</tt> are valid. All
///						others are ignored. <tt>std::ios_base::binary</tt> is assumed. If no mode is specified,
///						<tt>std::ios_base::in</tt> is assumed.

zfilebuf::_Myt * zfilebuf::open( char const * _Filename, std::ios_base::openmode _Mode )
{
	gzFile				_File;
	char const * const	modeString	= ( ( _Mode & std::ios_base::out ) != 0 ) ? "wb" : "rb";

	if ( _Myfile != 0 || ( _File = gzopen( _Filename, modeString ) ) == NULL )
	{
		return 0;
	}

	_Init( _File, _Openfl );

	return this;
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

zfilebuf::_Myt * zfilebuf::close()
{
	if ( _Myfile == 0 || gzclose( _Myfile ) != 0 )
	{
		return 0;
	}

    _Init( 0, _Closefl );

	return this;
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

///
/// @param	_Meta	Value to insert

zfilebuf::int_type zfilebuf::overflow( int_type _Meta/* = traits_type::eof()*/ )
{
	// If inserting EOF, then do nothing and return success

	if ( _Meta == traits_type::eof() )
	{
		return traits_type::not_eof( _Meta );
	}

	// Otherwise, if the file is not open, return error

	if ( _Myfile == 0 )
	{
		return traits_type::eof();
	}

	// Otherwise, write a byte to the file

	return ( gzputc( _Myfile, _Meta ) != -1 ) ? _Meta : traits_type::eof();
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

///
/// @param	_Meta	Value to put back. If it is <tt>traits_type::eof()</tt>, then put back the value that was read
///					last (if possible).

zfilebuf::int_type zfilebuf::pbackfail( int_type _Meta/* = traits_type::eof()*/ )
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
/*																													*/
/********************************************************************************************************************/

zfilebuf::int_type zfilebuf::underflow()
{
	int_type	_Meta;

	// If there is data in the input buffer, get it without incrementing the pointer

	if ( _Mysb::gptr() != 0 && _Mysb::gptr() < _Mysb::egptr() )
	{
		_Meta = int_type( *_Mysb::gptr() );
	}

	// Otherwise, get a byte from the file

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
/*																													*/
/********************************************************************************************************************/

zfilebuf::int_type zfilebuf::uflow()
{
	int_type	_Meta;

	// If there is data in the input buffer, return it.

	if ( _Mysb::gptr() != 0 && _Mysb::gptr() < _Mysb::egptr() )
	{
		_Meta = int_type( *_Mysb::_Gninc() );
	}

	// Otherwise, if no file is open, return error

	else if ( _Myfile == 0 )
	{
		_Meta = traits_type::eof();
	}

	// Otherwise, get a byte from the file

	else
	{
		int const	_Ch	= gzgetc( _Myfile );
		_Meta = ( _Ch != -1 ) ? (int_type)_Ch : traits_type::eof();
	}

	return _Meta;
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

/// @param	_Off		Number of uncompressed bytes to move the pointer
/// @param	_Way		Location to start seek. <tt>std::ios_base::end</tt> is not supported. Valid values are:
///							- <tt>std::ios_base::beg</tt>
///							- <tt>std::ios_base::cur</tt>
/// @param	openmode	Ignored (both in and out pointers are moved)
/// @note	There are restrictions imposed by @c zlib:
///				-#	<tt>std::ios_base::end</tt> is not supported as a start location
///				-#	Only forward seeks are allowed in output buffers

zfilebuf::pos_type zfilebuf::seekoff( off_type					_Off,
						    		  std::ios_base::seekdir	_Way,
						    		  std::ios_base::openmode	openmode	/* = ( std::ios_base::openmode )
																		( std::ios_base::in | std::ios_base::out )*/ )
{
	int			mode;

	if ( _Myfile == 0 )
	{
		return pos_type( std::_BADOFF );	// report failure
	}

	// There are restrictions with seeking:
	//
	//	std::ios_base::end is not supported as a start location
	//	Only forward seeks are allowed in output buffers, but that is not checked here. The seek will
	//	return an error.

	if ( _Way == std::ios_base::end )
	{
		return pos_type( std::_BADOFF );	// report failure
	}
	else if ( _Way == std::ios_base::cur )
	{
		mode = SEEK_CUR;
	}
	else
	{
		mode = SEEK_SET;
	}

	// If there is putback data, then the actual file pointer will be different from ours. This must be accounted
	// for if the seek mode is std::ios_base::cur.

	if ( _Mysb::gptr() == &_Mychar && _Mysb::gptr() < _Mysb::egptr() && _Way == std::ios_base::cur )
	{
		_Off -= off_type( sizeof ( _Mychar ) );
	}

	// Do the seek

	z_off_t const	_Fileposition	= gzseek( _Myfile, ( z_off_t )_Off, mode );
	if ( (int_type)_Fileposition == traits_type::eof() )
	{
		return pos_type( std::_BADOFF );	// report failure
	}

	// If there is putback data, discard it

	if ( _Mysb::gptr() == &_Mychar )
	{
		_Mysb::setg( &_Mychar, &_Mychar, &_Mychar );
	}

	return pos_type( _Fileposition );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

/// @param	_Pos		Location to move the pointer
/// @param	openmode	Ignored (both in and out pointers are moved)
/// @note	There are restrictions imposed by @c zlib:
///				-#	<tt>std::ios_base::end</tt> is not supported as a start location
///				-#	Only forward seeks are allowed in output buffers

zfilebuf::pos_type zfilebuf::seekpos( pos_type					_Pos,
									  std::ios_base::openmode	openmode	/* = ( std::ios_base::openmode )
																		( std::ios_base::in | std::ios_base::out )*/ )
{
	return seekoff( _Pos, std::ios_base::beg );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

/// @param	_Buffer	Address of buffer
/// @param	_Count	Size of the buffer
//
/// @note	This function does nothing since the stream is unbuffered. The return value is always 0.

zfilebuf::_Mysb * zfilebuf::setbuf( char_type *_Buffer, std::streamsize _Count )
{
	return 0;	// This function does nothing
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

int zfilebuf::sync()
{
	// No file is open, return success

	if ( _Myfile == 0 )
	{
		return 0;
	}

	// Overflow cannot write any more data, return success

	if ( overflow() != traits_type::eof() )
	{
		return 0;
	}

	// Flush and return status

	return ( ( gzflush( _Myfile, Z_SYNC_FLUSH ) >= 0 ) ? 0 : -1 );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

std::streamsize zfilebuf::xsgetn( char_type * _Ptr, std::streamsize _Count)
{
	// If the file is not open, return error

	if ( _Myfile == 0 )
	{
		return std::streamsize( 0 );
	}

	int	total	= 0;

	// If there is data in the input buffer, return it first.

	if ( _Mysb::gptr() != 0 )
	{
		while ( _Mysb::gptr() < _Mysb::egptr() && _Count > 0 )
		{
			*_Ptr++ = *_Mysb::_Gninc();
			--_Count;
			++total;
		}
	}

	// Read the rest from the file

	total += gzread( _Myfile, _Ptr, _Count );

	return std::streamsize( total );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

std::streamsize zfilebuf::xsputn( char_type const * _Ptr, std::streamsize _Count)
{
	// If the file is not open, return error

	if ( _Myfile == 0 )
	{
		return 0;
	}

	// Otherwise, write to the file

	return std::streamsize( gzwrite( _Myfile, (voidp)_Ptr, _Count ) );
}


/********************************************************************************************************************/
/*																													*/
/*																													*/
/********************************************************************************************************************/

/// @param	_File	File pointer of opened file
/// @param	_Which	What is happening. Valid values are:
///					- _Newfl
///					- _Openfl
///					- _Closefl

void zfilebuf::_Init( gzFile _File, _Initfl _Which )
{
	// If this is an open, then remember that the file must be closed

	_Closef = ( _Which == _Openfl );

	// Initialize stream buffer base object

	_Mysb::_Init();

	// Save the file pointer

	_Myfile	= _File;
}

