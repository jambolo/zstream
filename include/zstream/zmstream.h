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

//! An input stream that decompresses the data from a buffer using @c zlib.

class izmstream : public std::basic_istream<unsigned char, std::char_traits<unsigned char> >
{
public:

    typedef unsigned char char_type;                    //!< Element type
    typedef zmembuf::container_type container_type;     //!< The container class

    // Constructor
    izmstream();

    // Constructor
    //!
    //! @param   buf     buffer to decompress
    explicit izmstream(container_type const & buf);

    //! Returns a pointer to the stream buffer.
    zmembuf * rdbuf() const { return const_cast<zmembuf *>(&membuf_); }

    //! Returns the contents of the memory buffer.
    container_type const & buffer() const { return membuf_.buffer(); }

    //! Replaces the contents of the memory buffer.
    void buffer(container_type const & buf) { membuf_.buffer(buf); }

private:

    zmembuf membuf_;     // The memory buffer
};

//! An output stream that compresses the data into a buffer using @c zlib

class ozmstream : public std::basic_ostream<unsigned char, std::char_traits<unsigned char> >
{
public:

    typedef unsigned char char_type;                    //!< Element type
    typedef zmembuf::container_type container_type;     //!< The container class

    //! Constructor
    ozmstream();

    //! Returns a pointer to the stream buffer.
    zmembuf * rdbuf() const { return const_cast<zmembuf *>(&membuf_); }

    //! Returns the contents of the memory buffer.
    container_type const & buffer() const { return membuf_.buffer(); }

    //! Replaces the contents of the memory buffer.
    void buffer(container_type const & buf)  { membuf_.buffer(buf); }

    //! Sets the compression level.
    //!
    //! @param	level	Compression level. 0 is no compression, 9 is maximum compression.
    void set_compression(int level) { membuf_.set_compression(level); }

private:

    zmembuf membuf_;     // The memory buffer
};
