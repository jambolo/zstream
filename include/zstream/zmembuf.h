/** @file *//********************************************************************************************************

                                                     zmembuf.h

                                            Copyright 2003, John J. Bolton
    --------------------------------------------------------------------------------------------------------------

    $Header: //depot/Libraries/zstream/zmembuf.h#4 $

    $NoKeywords: $

 *********************************************************************************************************************/

#pragma once

#include "zlib/zlib.h"
#include <streambuf>
#include <vector>

//! A memory stream buffer that compresses and decompresses data using @c zlib
class zmembuf : public std::basic_streambuf<unsigned char, std::char_traits<unsigned char> >
{
public:
    typedef unsigned char char_type;                    //!< Element type
    typedef std::char_traits<char_type> traits_type;    //!< The element's traits

    typedef std::basic_streambuf<char_type, traits_type>    streambuf_type; //!< The streambuf base class
    typedef std::vector<char_type>                          container_type; //!< The data container class

    typedef traits_type::int_type int_type;     //!< Holds values not representable by char_type
    typedef traits_type::pos_type pos_type;     //!< Holds a buffer position
    typedef traits_type::off_type off_type;     //!< Holds a buffer offset

    // Constructor
    explicit zmembuf(std::ios_base::openmode mode);

    // Constructor
    zmembuf(container_type const & data, std::ios_base::openmode mode);

    // Constructor
    zmembuf(char_type const * data, size_t size, std::ios_base::openmode mode);

    // Destructor
    virtual ~zmembuf();

    //! Returns a reference to the data in the buffer.
    container_type const & buffer() const;

    //! Replaces the current data in the buffer.
    void buffer(container_type const & data);

    //! Replaces the current data in the buffer.
    void buffer(char_type const * data, size_t size);

    //! Sets the compression level.
    void set_compression(int level);

protected:

    //! @name Overrides basic_streambuf
    //@{

    //! Inserts the character into the buffer. Returns the character or traits_type::eof() if it failed.
    virtual int_type overflow(int_type meta = traits_type::eof()) override;

    //! Puts a character back into the buffer. Returns the character or traits_type::eof() if it failed.
    virtual int_type pbackfail(int_type meta = traits_type::eof()) override;

    virtual std::streamsize showmanyc() override;

    //! Returns the current character from the buffer (primarily when it is empty).
    virtual int_type underflow() override;

    //	virtual int_type uflow();

    //! Reads @a n uncompressed characters from the buffer. Returns the number of characters actually read.
    virtual std::streamsize xsgetn(char_type * s, std::streamsize n) override;

    //! Writes @a _Count uncompressed characters to the buffer. Returns the number of characters actually written.
    virtual std::streamsize xsputn(char_type const * s, std::streamsize n) override;

    //! Sets the current position relative to a specific point in the buffer. Returns the new position.
    virtual pos_type seekoff(off_type                _Off,
                             std::ios_base::seekdir  way,
                             std::ios_base::openmode which =
                                 (std::ios_base::openmode)(std::ios_base::in | std::ios_base::out)) override;

    //! Sets the current position to a previous location. Returns the new position.
    virtual pos_type seekpos(pos_type                _Pos,
                             std::ios_base::openmode which =
                                 (std::ios_base::openmode)(std::ios_base::in | std::ios_base::out)) override;

    //	virtual streambuf_type * setbuf( char_type * _Buffer, std::streamsize _Count ) override;

    //	virtual void imbue(const locale&);

    //@}

private:

    // Stream state bits
    enum StreamStateBit
    {
        RO_BIT = 0, // Character array is immutable
        WO_BIT = 1  // Character array cannot be read
    };
    typedef int StreamState;  //!< Stream state

    // Initialization
    void initialize(container_type const & data, StreamState state);

    // Cleanup
    void tidy();

    // Syncs the buffer, ensuring that all the data has been compressed/decompressed.
    void sync() const;

    // Returns a stream state converted from an open mode
    StreamState streamState(std::ios_base::openmode mode);

    // Make sure there is enough room in the container to hold this many bytes of additional output
    void grow(size_t size);

    void flushInput();

    StreamState state_;     // The stream state
    mutable container_type data_;   // Memory buffer holding the compressed/decompressed data
    mutable z_stream stream_;      // The zlib stream state
    char_type putback_;      // putback buffer
};
