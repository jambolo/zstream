/** @file *//********************************************************************************************************

                                                    zfstream.cpp

    --------------------------------------------------------------------------------------------------------------

    $Header: //depot/Libraries/zstream/zfstream.cpp#2 $

    $NoKeywords: $

 *********************************************************************************************************************/

#include "zfstream.h"

#include "zlib/zlib.h"

#include <istream>
#include <ostream>

//!
//! @param	name	Name of the file to be opened for input, or 0

izfstream::izfstream(char const * name /* = nullptr*/)
    : base_type(&fileBuffer_)
{
    if (name && !fileBuffer_.open(name, std::ios_base::in))
    {
        ios_type::setstate(std::ios_base::failbit);
    }
}

//!
//! @param	name	Name of the file to be opened for input

void izfstream::open(char const * name)
{
    if (!fileBuffer_.open(name, std::ios_base::in))
    {
        ios_type::setstate(std::ios_base::failbit);
    }
}

void izfstream::close()
{
    if (!fileBuffer_.close())
    {
        ios_type::setstate(ios_base::failbit);
    }
}

//!
//! @param	name	Name of the file to be opened for output

ozfstream::ozfstream(const char * name /* = nullptr*/)
    : std::basic_ostream<char_type, traits_type>(&fileBuffer_)
{
    if (name && !fileBuffer_.open(name, std::ios_base::out))
    {
        ios_type::setstate(std::ios_base::failbit);
    }
}

//!
//! @param	name	Name of the file to be opened for output

void ozfstream::open(char const * name)
{
    if (!fileBuffer_.open(name, std::ios_base::out))
    {
        ios_type::setstate(std::ios_base::failbit);
    }
}

void ozfstream::close()
{
    if (!fileBuffer_.close())
    {
        ios_type::setstate(std::ios_base::failbit);
    }
}
