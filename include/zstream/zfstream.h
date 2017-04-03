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

//! An input stream that decompresses the data using @c zlib from a file.
class izfstream : public std::basic_istream<unsigned char, std::char_traits<unsigned char> >
{
public:
    typedef unsigned char                               char_type;      //!< Element type
    typedef std::char_traits<unsigned char>             traits_type;    //!< The element type's traits (not used, included for completeness)
    typedef std::basic_istream<char_type, traits_type>  base_type;      //!< Base class type
    typedef std::basic_ios<char_type, traits_type>      ios_type;       //!< IOS type

    // Constructor
    explicit izfstream(char const * name = nullptr);

    //! Returns a pointer to the file buffer
    zfilebuf * rdbuf() const { return const_cast<zfilebuf *>(&fileBuffer_); }

    //! Returns true if the file is open
    bool is_open() const { return fileBuffer_.is_open();  }

    //! Opens a file
    void open(const char * name);

    //! Closes the file
    void close();

private:
    zfilebuf fileBuffer_;
};

//! A output stream that compresses the data using @c zlib to a file.
class ozfstream : public std::basic_ostream<unsigned char, std::char_traits<unsigned char> >
{
public:
    typedef unsigned char                   char_type;      //!< Element type
    typedef std::char_traits<unsigned char> traits_type;    //!< The element type's traits (not used, included for completeness)

    // Constructor
    explicit ozfstream(const char * name = nullptr);

    // Destructor
    virtual ~ozfstream();

    //! Returns a pointer to filebuffer
    zfilebuf * rdbuf() const { return const_cast<zfilebuf *>(&fileBuffer_); }

    //! Returns true if a file is opened
    bool is_open() const { return fileBuffer_.is_open();  }

    //! Opens a file
    void open(char const * name);

    //! Closes the file
    void close();

    //! Sets the compression level.
    //!
    //! @param	level	Compression level. 0 is no compression, 9 is maximum compression.
    void set_compression(int level) { fileBuffer_.set_compression(level); }

private:
    typedef std::basic_ios<char_type, traits_type> ios_type;

    zfilebuf fileBuffer_;
};
