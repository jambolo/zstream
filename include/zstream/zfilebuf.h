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

//! A file stream buffer that compresses and decompresses the data using @c zlib.
class zfilebuf : public std::basic_streambuf<unsigned char, std::char_traits<unsigned char> >
{
public:
    typedef unsigned char char_type;                                    //!< Element type
    typedef std::char_traits<unsigned char> traits_type;                //!< The element's traits (not used, included for completeness.
    typedef std::basic_streambuf<char_type, traits_type>  base_type;    //!< The streambuf base class

    typedef traits_type::int_type int_type;     //!< Holds info not representable by char_type
    typedef traits_type::pos_type pos_type;     //!< Holds a buffer position
    typedef traits_type::off_type off_type;     //!< Holds a buffer offset

    //! Reasons for a call to _Init
    enum InitializeReason
    {
        NEW,    //!< Initialization
        OPENED, //!< File opened
        CLOSED  //!< File closed
    };

    // Constructor
    zfilebuf(gzFile file = nullptr);

    // Destructor
    virtual ~zfilebuf();

    //! Returns @c true if the file has been opened
    bool is_open() const
    {
        return file_ != 0;
    }

    //! Opens a file. Returns @c this.
    zfilebuf * open(char const * name, std::ios_base::openmode mode);

    //! Closes the file. Returns @c this.
    zfilebuf * close();

    //! Sets the compression level.
    void set_compression(int level);

protected:

    //! @name Overrides basic_streambuf
    //@{

    //! Inserts the character into the buffer (primarily when it is full).
    virtual int_type overflow(int_type meta = traits_type::eof()) override;

    //! Puts a character back into the buffer, then makes it the current character.
    virtual int_type pbackfail(int_type meta = traits_type::eof()) override;

    //	virtual std::streamsize showmanyc() override;

    //! Returns the current character from the buffer (primarily when it is empty).
    virtual int_type underflow() override;

    //! Returns the current character from the buffer (primarily when it is empty) and points to the next character.
    virtual int_type uflow() override;

    //! Reads @p n characters from the buffer. Returns the number of characters actually read.
    virtual std::streamsize xsgetn(char_type * s, std::streamsize n) override;

    //! Writes @p n characters to the buffer. Returns the number of characters actually written.
    virtual std::streamsize xsputn(char_type const * s, std::streamsize n) override;

    //! Sets the current position relative to a specific point in the file. Returns the new position.
    virtual pos_type seekoff(off_type off,
                             std::ios_base::seekdir way,
                             std::ios_base::openmode which = (std::ios_base::openmode)(std::ios_base::in | std::ios_base::out)) override;

    //! Sets the current position to a previous location. Returns the new position.
    virtual pos_type seekpos(pos_type pos,
                             std::ios_base::openmode which = (std::ios_base::openmode)(std::ios_base::in | std::ios_base::out)) override;

    //! Provides a buffer. Returns @c this.
    virtual base_type * setbuf(char_type * s, std::streamsize n) override;

    //! Synchronizes the buffer with the file.
    virtual int sync() override;

    //	virtual void imbue(const locale&) override;

    //@}

    //! Initialization.
    void initialize(gzFile file, InitializeReason reason);

private:
    char_type putback_; // putback buffer
    bool needsClose_;   // True if file must be closed
    gzFile file_;       // gz file pointer
};
