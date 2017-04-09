/** @file *//********************************************************************************************************

                                                    zmstream.cpp

                                            Copyright 2003, John J. Bolton
    --------------------------------------------------------------------------------------------------------------

    $Header: //depot/Libraries/zstream/zmstream.cpp#4 $

    $NoKeywords: $

 *********************************************************************************************************************/

#include "zmstream.h"

izmstream::izmstream()
    : std::basic_istream<unsigned char, std::char_traits<unsigned char> >(&membuf_)
    , membuf_(std::ios_base::in)
{
}

izmstream::izmstream(container_type const & buf)
    : std::basic_istream<unsigned char, std::char_traits<unsigned char> >(&membuf_)
    , membuf_(buf, std::ios_base::in)
{
}

ozmstream::ozmstream()
    : std::basic_ostream<unsigned char, std::char_traits<unsigned char> >(&membuf_)
    , membuf_(std::ios_base::out)
{
}
