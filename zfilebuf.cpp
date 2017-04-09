/** @file *//********************************************************************************************************

                                                    zfilebuf.cpp

                                            Copyright 2003, John J. Bolton
    --------------------------------------------------------------------------------------------------------------

    $Header: //depot/Libraries/zstream/zfilebuf.cpp#4 $

    $NoKeywords: $

 *********************************************************************************************************************/

#include "zfilebuf.h"

#include "zlib/zlib.h"

#include <fstream>
#include <streambuf>

//!
//! @param  file
zfilebuf::zfilebuf(gzFile file /* = nullptr*/)
    : base_type()
{
    if (file)
    {
        initialize(file, NEW);
    }
}

zfilebuf::~zfilebuf()
{
    if (needsClose_)
    {
        close();
    }
}

//!
//! @param	level	Compression level. 0 is no compression, 9 is maximum compression.

void zfilebuf::set_compression(int level)
{
    if (level < 0)
    {
        level = 0;
    }
    else if (level > 9)
    {
        level = 9;
    }

    gzsetparams(file_, level, Z_DEFAULT_STRATEGY);
}

//! @param	name	Name of the file to open.
//! @param	mode	Open mode. Only <tt>std::ios_base::in</tt> and <tt>std::ios_base::out</tt> are valid. All
//!					others are ignored. <tt>std::ios_base::binary</tt> is assumed. If no mode is specified,
//!					<tt>std::ios_base::in</tt> is assumed.

zfilebuf * zfilebuf::open(char const * name, std::ios_base::openmode mode)
{
    gzFile file;
    char const * const modeString = ((mode & std::ios_base::out) != 0) ? "wb" : "rb";

    if (file_ != 0 || (file = gzopen(name, modeString)) == NULL)
    {
        return 0;
    }

    initialize(file, OPENED);

    return this;
}

zfilebuf * zfilebuf::close()
{
    if (!file_ || gzclose(file_) != 0)
    {
        return 0;
    }

    initialize(0, CLOSED);

    return this;
}

//! @param	meta	Value to insert
//!
//! @return     The character or <tt>traits_type::eof()</tt> if it failed.

zfilebuf::int_type zfilebuf::overflow(int_type meta /* = traits_type::eof()*/)
{
    // If inserting EOF, then do nothing and return success
    if (meta == traits_type::eof())
    {
        return traits_type::not_eof(meta);
    }

    // Otherwise, if the file is not open, return error

    if (!file_)
    {
        return traits_type::eof();
    }

    // Otherwise, write a byte to the file
    return (gzputc(file_, meta) != -1) ? meta : traits_type::eof();
}

//! @param	meta	Value to put back. If it is <tt>traits_type::eof()</tt>, then put back the value that was read
//!					last (if possible).
//! @return the character or <tt>traits_type::eof()</tt> if it failed.

zfilebuf::int_type zfilebuf::pbackfail(int_type meta /* = traits_type::eof()*/)
{
    // If there is a input buffer and there is data before the current position, and meta is EOF or the same as
    // the previous character in the input buffer, just back up the current position
    if (base_type::gptr() != 0 && base_type::eback() < base_type::gptr() &&
        (meta == traits_type::eof() || int_type(base_type::gptr() [-1]) == meta))
    {
        base_type::_Gndec();
        return traits_type::not_eof(meta);
    }

    // Otherwise, if the file isn't open or a EOF is put back return error.
    else if (!file_ || meta == traits_type::eof())
    {
        return traits_type::eof();
    }

    // Otherwise, put the data in the putback buffer
    else
    {
        putback_ = char_type(meta);
        base_type::setg(&putback_, &putback_, &putback_ + 1);

        return meta;
    }
}

zfilebuf::int_type zfilebuf::underflow()
{
    int_type meta;

    // If there is data in the input buffer, get it without incrementing the pointer
    if (base_type::gptr() != 0 && base_type::gptr() < base_type::egptr())
    {
        meta = int_type(*base_type::gptr());
    }

    // Otherwise, get a byte from the file
    else
    {
        meta = uflow();

        // If it did not fail, the put the byte back
        if (meta != traits_type::eof())
        {
            pbackfail(meta);
        }
    }

    return meta;
}

zfilebuf::int_type zfilebuf::uflow()
{
    int_type meta;

    // If there is data in the input buffer, return it.
    if (base_type::gptr() != 0 && base_type::gptr() < base_type::egptr())
    {
        meta = int_type(*base_type::_Gninc());
    }

    // Otherwise, if no file is open, return error
    else if (!file_)
    {
        meta = traits_type::eof();
    }

    // Otherwise, get a byte from the file
    else
    {
        int const _Ch = gzgetc(file_);
        meta = (_Ch != -1) ? (int_type)_Ch : traits_type::eof();
    }

    return meta;
}

//! @param	off		    Number of uncompressed bytes to move the pointer
//! @param	way		    Location to start seek. <tt>std::ios_base::end</tt> is not supported. Valid values are:
//!							- <tt>std::ios_base::beg</tt>
//!							- <tt>std::ios_base::cur</tt>
//! @param	openmode    Ignored (both in and out pointers are moved)
//!
//! @note	There are restrictions imposed by @c zlib:
//!				-#	<tt>std::ios_base::end</tt> is not supported as a start location
//!				-#	Only forward seeks are allowed in output buffers

zfilebuf::pos_type zfilebuf::seekoff(off_type                off,
                                     std::ios_base::seekdir  way,
                                     std::ios_base::openmode openmode /*= (std::ios_base::openmode)
                                                                         (std::ios_base::in|std::ios_base::out)*/)
{
    int mode;

    if (!file_)
    {
        return pos_type(std::_BADOFF);      // report failure
    }

    // There are restrictions with seeking:
    //
    //	std::ios_base::end is not supported as a start location
    //	Only forward seeks are allowed in output buffers, but that is not checked here. The seek will
    //	return an error.
    if (way == std::ios_base::end)
    {
        return pos_type(std::_BADOFF);      // report failure
    }
    else if (way == std::ios_base::cur)
    {
        mode = SEEK_CUR;
    }
    else
    {
        mode = SEEK_SET;
    }

    // If there is putback data, then the actual file pointer will be different from ours. This must be accounted
    // for if the seek mode is std::ios_base::cur.
    if (base_type::gptr() == &putback_ && base_type::gptr() < base_type::egptr() && way == std::ios_base::cur)
    {
        off -= off_type(sizeof(putback_));
    }

    // Do the seek
    z_off_t const _Fileposition = gzseek(file_, (z_off_t)off, mode);
    if ((int_type)_Fileposition == traits_type::eof())
    {
        return pos_type(std::_BADOFF);      // report failure
    }

    // If there is putback data, discard it
    if (base_type::gptr() == &putback_)
    {
        base_type::setg(&putback_, &putback_, &putback_);
    }

    return pos_type(_Fileposition);
}

//! @param	pos		    Location to move the pointer
//! @param	which       Ignored (both in and out pointers are moved)
//! @note	There are restrictions imposed by @c zlib:
//!				-#	<tt>std::ios_base::end</tt> is not supported as a start location
//!				-#	Only forward seeks are allowed in output buffers

zfilebuf::pos_type zfilebuf::seekpos(pos_type                pos,
                                     std::ios_base::openmode which /* = (std::ios_base::openmode) (std::ios_base::in |
                                                                      std::ios_base::out)*/)
{
    return seekoff(pos, std::ios_base::beg);
}

//! @param	s	Address of buffer
//! @param	n	Size of the buffer
//
//! @note	This function does nothing since the stream is unbuffered. The return value is always 0.

zfilebuf::base_type * zfilebuf::setbuf(char_type * s, std::streamsize n)
{
    return this;    // This function does nothing
}

int zfilebuf::sync()
{
    // No file is open, return success
    if (!file_)
    {
        return 0;
    }

    // Overflow cannot write any more data, return success
    if (overflow() != traits_type::eof())
    {
        return 0;
    }

    // Flush and return status
    return (gzflush(file_, Z_SYNC_FLUSH) >= 0) ? 0 : -1;
}

std::streamsize zfilebuf::xsgetn(char_type * s, std::streamsize n)
{
    // If the file is not open, return error
    if (!file_)
    {
        return std::streamsize(0);
    }

    int total = 0;

    // If there is data in the input buffer, return it first.
    if (base_type::gptr() != 0)
    {
        while (base_type::gptr() < base_type::egptr() && n > 0)
        {
            *s++ = *base_type::_Gninc();
            --n;
            ++total;
        }
    }

    // Read the rest from the file
    total += gzread(file_, s, (unsigned int)n);

    return std::streamsize(total);
}

std::streamsize zfilebuf::xsputn(char_type const * s, std::streamsize n)
{
    // If the file is not open, return error
    if (!file_)
    {
        return 0;
    }

    // Otherwise, write to the file
    return std::streamsize(gzwrite(file_, s, (unsigned int)n));
}

//! @param	file	File pointer of opened file
//! @param	reason	What is happening. Valid values are:
//!					- NEW
//!					- OPENED
//!					- CLOSED

void zfilebuf::initialize(gzFile file, InitializeReason reason)
{
    // If this is an open, then remember that the file must be closed
    needsClose_ = (reason == OPENED);

    // Save the file pointer
    file_ = file;
}
