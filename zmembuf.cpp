/** @file *//********************************************************************************************************

                                                    zmembuf.cpp

                                            Copyright 2003, John J. Bolton
    --------------------------------------------------------------------------------------------------------------

    $Header: //depot/Libraries/zstream/zmembuf.cpp#5 $

    $NoKeywords: $

 *********************************************************************************************************************/

#include "zmembuf.h"

#include "zlib/zlib.h"

#include <algorithm>
#include <cassert>
#include <streambuf>
#include <vector>

//! @param	mode	Direction of the stream
//!					- <tt>std::ios_base::in</tt> signifies an input buffer. Data is decompressed as it is streamed
//!						from of this buffer. You must initialize the contents of the buffer before streaming.
//!					- <tt>std::ios_base::out</tt> signifies an ouput buffer. Data is compressed as it is streamed
//!						to this buffer. The buffer will grow as data is streamed to it.

zmembuf::zmembuf(std::ios_base::openmode mode)
{
    initialize(container_type(), streamState(mode));
}

//! @param	data	Initial contents of the buffer.
//! @param	mode	Direction of the stream
//!					- <tt>std::ios_base::in</tt> signifies an input buffer. The contents are decompressed as they
//!						are streamed from the buffer.
//!					- <tt>std::ios_base::out</tt> signifies an ouput buffer. Data streamed to this buffer is
//!						compressed and appended to the initial contents.

zmembuf::zmembuf(container_type const &  data,
                 std::ios_base::openmode mode)
{
    initialize(data, streamState(mode));
}

//! @param	data	Initial contents of the buffer.
//! @param  size    Size of the buffer
//! @param	mode	Direction of the stream
//!					- <tt>std::ios_base::in</tt> signifies an input buffer. The contents are decompressed as they
//!						are streamed from the buffer.
//!					- <tt>std::ios_base::out</tt> signifies an ouput buffer. Data streamed to this buffer is
//!						compressed and appended to the initial contents.

zmembuf::zmembuf(char_type const * data, size_t size, std::ios_base::openmode mode)
{
    initialize(container_type(data, data + size), mode);
}

zmembuf::~zmembuf()
{
    tidy();
}

//! @warning	The returned data is valid only until the next operation on the buffer.
//! @note		This function forces zlib to flush its buffers. Frequent calls to this function will degrade
//!				performance.

zmembuf::container_type const & zmembuf::buffer() const
{
    // Make sure the buffer is synced before giving access to it.
    sync();

    return data_;
}

//!
//! @param	data	Data replacing the current contents of the buffer.

void zmembuf::buffer(container_type const & data)
{
    tidy();
    initialize(data, state_);
}

//! @param	data	Data replacing the current contents of the buffer.
//! @param	size		size of the data (in bytes)

void zmembuf::buffer(char_type const * data, size_t size)
{
    tidy();
    initialize(container_type(&data[0], &data[size]), state_);
}

//!
//! @param	level	Compression level. 0 is no compression, 9 is maximum compression.

void zmembuf::set_compression(int level)
{
    if (level < 0)
    {
        level = 0;
    }
    else if (level > 9)
    {
        level = 9;
    }

    deflateParams(&stream_, level, Z_DEFAULT_STRATEGY);
}

//!
//! @param	meta	Value to put into the buffer

zmembuf::int_type zmembuf::overflow(int_type meta /* = traits_type::eof()*/)
{
    // If the character to store is EOF, then do nothing and return success
    if (traits_type::eof() == meta)
    {
        return traits_type::not_eof(meta);
    }

    // Otherwise, if the data is read-only, then return fail
    else if (state_ & RO_BIT)
    {
        return traits_type::eof();
    }

    // Otherwise, add the value to the data
    else
    {
        // First make sure that any unprocessed data has been processed
        flushInput();

        // Send the value to the compressor
        Bytef c = (char_type)meta;
        stream_.next_in  = &c;
        stream_.avail_in = sizeof(c);

        grow(sizeof(c) + 1);
        deflate(&stream_, 0);

        return meta;
    }
}

//! @param	meta	Value to put back. If it is <tt>traits_type::eof()</tt>, then put back the character was read
//!					last (if possible).
//!
//! @note	The character put back becomes the current character

zmembuf::int_type zmembuf::pbackfail(int_type meta /* = traits_type::eof()*/)
{
    // If there is a input buffer and there is data before the current position, and meta is EOF or the same as
    // the previous character in the input buffer, just back up the current position
    if (!gptr() && eback() < gptr() &&
        (meta == traits_type::eof() || int_type(gptr() [-1]) == meta))
    {
        _Gndec();
        return traits_type::not_eof(meta);
    }

    // Otherwise, if the file isn't open or a EOF is put back return error.
    else if (meta == traits_type::eof())
    {
        return traits_type::eof();
    }

    // Otherwise, put the data in the putback buffer
    else
    {
        putback_ = char_type(meta);
        setg(&putback_, &putback_, &putback_ + 1);

        return meta;
    }
}

std::streamsize zmembuf::showmanyc()
{
    if (!gptr())
    {
        return egptr() - gptr();
    }
    else
    {
        return 0;
    }
}

zmembuf::int_type zmembuf::underflow()
{
    int_type meta;

    // If there is data in the input buffer, get it without incrementing the pointer
    if (!gptr() && gptr() < egptr())
    {
        meta = int_type(*gptr());
    }

    // Otherwise, get a byte from zlib

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

//! @param	s	buffer to store streamed data
//! @param	n	Number of uncompressed bytes to get

std::streamsize zmembuf::xsgetn(char_type * s, std::streamsize n)
{
    return n;
}

//! @param	s	Uncompressed data to put
//! @param	n	Number of uncompressed bytes to put

std::streamsize zmembuf::xsputn(char_type const * s, std::streamsize n)
{
    // First make sure that any unprocessed data has been processed
    flushInput();

    // Send the data to zlib
    while (n > 0)
    {
        uInt block = (uInt)std::min(n, (std::streamsize)std::numeric_limits<uInt>::max());
        stream_.next_in  = s;
        stream_.avail_in = block;
        grow(block + 1);
        deflate(&stream_, 0);
        n -= block;
    }

    return n;
}

//! @param	off	    Number of uncompressed bytes to move the pointer
//! @param	way	    Location to start seek. <tt>std::ios_base::end</tt> is not supported. Valid values are:
//!						- <tt>std::ios_base::beg</tt>
//!						- <tt>std::ios_base::cur</tt>
//! @param	which	Ignored (both in and out pointers are moved)
//! @note	There are restrictions imposed by @c zlib:
//!				-#	<tt>std::ios_base::end</tt> is not supported as a start location
//!				-#	Only forward seeks are allowed in output buffers

zmembuf::pos_type zmembuf::seekoff(off_type                off,
                                   std::ios_base::seekdir  way,
                                   std::ios_base::openmode which /* = std::ios_base::in | std::ios_base::out*/)
{
    pos_type _Pos;

    // Make sure all output is done before trying to position the pointer

    sync();

    // position within read buffer

    if (!gptr())
    {
        if (way == std::ios_base::end)
        {
            off = std::_BADOFF;
        }
        else if (way == std::ios_base::cur)
        {
            off += off_type(gptr() - eback());
        }
        else if (way == std::ios_base::beg)
        {
        }
        else
        {
            off = std::_BADOFF;
        }

        // change read position
        if (0 <= off && off <= egptr() - eback())
        {
            gbump((int)(eback() + off - gptr()));
        }
        else
        {
            off = std::_BADOFF;
        }
    }

    // Otherwise, if this is a write buffer, position the pointer
    else if (pptr() != 0)
    {
        // Figure out the offset from the beginning of the buffer and adjust off so that it is the number of bytes
        // from the current position.

        if (way == std::ios_base::beg)
        {
            _Pos = pos_type(off);
            off -= off_type(stream_.total_out);
        }
        else if (way == std::ios_base::cur)
        {
            _Pos = pos_type(off + off_type(stream_.total_out));
            off += 0;
        }
        else
        {
            _Pos = pos_type(std::_BADOFF);
            off  = -1;
        }

        // Change write position by inserting off bytes of zeroe. Only forward seeks are allowed.
        if (0 <= off)
        {
            // Send off bytes of dummy data to zlib

            grow(off + 1);
            while (off > 0)
            {
                Bytef dummy = 0;
                stream_.next_in  = &dummy;
                stream_.avail_in = 1;

                deflate(&stream_, 0);

                --off;
            }
        }
        else
        {
            _Pos = pos_type(std::_BADOFF);
        }
    }
    else
    {
        _Pos = pos_type(std::_BADOFF);      // neither read nor write buffer selected, fail
    }

    return _Pos;
}

//! @param	pos	Location to move the pointer
//! @param	mode	Ignored (both in and out pointers are moved)
//! @note	There are restrictions imposed by @c zlib:
//!				-#	<tt>std::ios_base::end</tt> is not supported as a start location
//!				-#	Only forward seeks are allowed in output buffers

zmembuf::pos_type zmembuf::seekpos(pos_type                pos,
                                   std::ios_base::openmode mode /* = std::ios_base::in | std::ios_base::out*/)
{
    off_type off = off_type(pos);

    // position within read buffer
    if (!gptr())
    {
        if (0 <= off && off <= egptr() - eback())
        {
            gbump((int)(eback() + off - gptr()));
        }
        else
        {
            off = std::_BADOFF;
        }
    }
    // position within write buffer
    else if (pptr() != 0)
    {
        if (0 <= off && off <= epptr() - pbase())
        {
            pbump((int)(pbase() + off - pptr()));
        }
        else
        {
            off = std::_BADOFF;
        }
    }
    else
    {
        off = std::_BADOFF;    // neither read nor write buffer selected, fail
    }

    return pos_type(off);
}

void zmembuf::sync() const
{
    if (state_ == 0)
    {
        inflateEnd(&stream_);
    }
    else
    {
        deflateEnd(&stream_);
    }

    data_.resize(stream_.total_out);
}

//! @param	data	Initial contents of the buffer
//! @param	state	Read or write state of the buffer

void zmembuf::initialize(container_type const & data, StreamState state)
{
    state_ = state;

    data_.clear();
    setg(0, 0, 0);
    setp(0, 0, 0);

    if (data.size() > 0)
    {
        data_.assign(data.begin(), data.end() - 1);

        if (state_ == 0)
        {
            int ok;

            // Initialize zlib for inflating (use default memory allocation)
            stream_.zalloc = Z_NULL;
            stream_.zfree  = Z_NULL;
            stream_.opaque = Z_NULL;
            ok = inflateInit(&stream_);
            assert(ok);

            // Set the stream buffer pointers

            setg(&*data_.begin(), &*data_.begin(), &*data_.end());
        }
        else // if ( state_ == 1 )
        {
            int ok;

            // Initialize zlib for deflating (use default memory allocation)
            stream_.zalloc = Z_NULL;
            stream_.zfree  = Z_NULL;
            stream_.opaque = Z_NULL;
            ok = deflateInit(&stream_, Z_DEFAULT_COMPRESSION);
            assert(ok);

            // Set the stream buffer pointers

            setp(&*data_.begin(), &*data_.end(), &*data_.end());
        }
    }
}

void zmembuf::tidy()
{
    // Make sure the data is synced before shutting it down or replacing it.
    sync();
}

//!
//! @param	mode	Open mode

zmembuf::StreamState zmembuf::streamState(std::ios_base::openmode mode)
{
    return ((mode & std::ios_base::out) != 0) ? 1 : 0;
}

//!
//! @param	size	Number of bytes to make room for

void zmembuf::grow(size_t size)
{
    assert(size <= std::numeric_limits<uInt>::max());
    data_.resize(stream_.total_out + size);

    stream_.next_out  = &data_[stream_.total_out];
    stream_.avail_out = (uInt)size;
}

void zmembuf::flushInput()
{
    while (stream_.avail_in > 0)
    {
        grow(stream_.avail_in + 1);
        deflate(&stream_, 0);
    }
}
