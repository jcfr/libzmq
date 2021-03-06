/*
    Copyright (c) 2007-2016 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <sstream>

#include "macros.hpp"
#include "udp_address.hpp"
#include "platform.hpp"
#include "stdint.hpp"
#include "err.hpp"
#include "ip.hpp"

#ifdef ZMQ_HAVE_WINDOWS
#include "windows.hpp"
#else
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#endif

zmq::udp_address_t::udp_address_t ()
{
    memset (&bind_address, 0, sizeof bind_address);
}

zmq::udp_address_t::~udp_address_t ()
{
}

int zmq::udp_address_t::resolve (const char *name_)
{
    //  Find the ':' at end that separates address from the port number.
    const char *delimiter = strrchr (name_, ':');
    if (!delimiter) {
        errno = EINVAL;
        return -1;
    }

    //  Separate the address/port.
    std::string addr_str (name_, delimiter - name_);
    std::string port_str (delimiter + 1);

    //  Remove square brackets around the address, if any, as used in IPv6
    if (addr_str.size () >= 2 && addr_str [0] == '[' &&
          addr_str [addr_str.size () - 1] == ']')
        addr_str = addr_str.substr (1, addr_str.size () - 2);

    //  Parse the port number (0 is not a valid port).
    uint16_t port = (uint16_t) atoi (port_str.c_str ());
    if (port == 0) {
        errno = EINVAL;
        return -1;
    }

    dest_address.sin_family = AF_INET;
    dest_address.sin_port = htons (port);
    dest_address.sin_addr.s_addr = inet_addr (addr_str.c_str ());

    if (dest_address.sin_addr.s_addr == INADDR_NONE) {
        errno = EINVAL;
        return -1;
    }

    // we will check only first byte of IP
    // and if it from 224 to 239, then it can
    // represent multicast IP.
    int i = dest_address.sin_addr.s_addr & 0xFF;
    if(i >=  224 && i <= 239) {
        multicast = dest_address.sin_addr;
        is_mutlicast = true;
    }
    else
        is_mutlicast = false;

    interface.s_addr = htons (INADDR_ANY);
    if (interface.s_addr == INADDR_NONE) {
        errno = EINVAL;
        return -1;
    }

    bind_address.sin_family = AF_INET;
    bind_address.sin_port = htons (port);
    bind_address.sin_addr.s_addr = htons (INADDR_ANY);

    address = name_;

    return 0;
}

int zmq::udp_address_t::to_string (std::string &addr_)
{
    addr_ = address;
    return 0;
}

bool zmq::udp_address_t::is_mcast () const
{
    return is_mutlicast;
}

const sockaddr* zmq::udp_address_t::bind_addr () const
{
    return (sockaddr *) &bind_address;
}

socklen_t zmq::udp_address_t::bind_addrlen () const
{
    return sizeof (sockaddr_in);
}

const sockaddr* zmq::udp_address_t::dest_addr () const
{
    return (sockaddr *) &dest_address;
}

socklen_t zmq::udp_address_t::dest_addrlen () const
{
    return sizeof (sockaddr_in);
}

const in_addr zmq::udp_address_t::multicast_ip () const
{
    return multicast;
}

const in_addr zmq::udp_address_t::interface_ip () const
{
    return interface;
}

#if defined ZMQ_HAVE_WINDOWS
unsigned short zmq::udp_address_t::family () const
#else
sa_family_t zmq::udp_address_t::family () const
#endif
{
    return AF_INET;
}
