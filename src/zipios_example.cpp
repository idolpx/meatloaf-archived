/*
  Zipios -- a small C++ library that provides easy access to .zip files.

  Copyright (C) 2000-2007  Thomas Sondergaard
  Copyright (C) 2015-2019  Made to Order Software Corporation

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/** \file
 * \brief Implementation of a very simple Zipios example.
 * \anchor zipios_example_anchor
 *
 * This source code is a small program that demonstrates the central
 * elements of Zipios.
 *
 * \include zipios_example.cpp
 */

#include "zipios/zipfile.hpp"


int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        std::cerr << "error: usage: zipios_example <path to test.zip> <filename to extract and print>" << std::endl;
        std::cerr << "       from the source directory, try as:" << std::endl;
        std::cerr << "              zipios_example tests/test.zip test2.txt" << std::endl;
        return 1;
    }

    try
    {
        std::cout << "Instantiating a ZipFile" << std::endl;
        zipios::ZipFile zf(argv[1]);

        std::cout << "list length: " << zf.size() << std::endl;

        zipios::FileEntry::vector_t entries(zf.entries());
        for(auto it = entries.begin(); it != entries.end(); ++it)
        {
            std::cout << "  " << *(*it) << std::endl;
        }

        zipios::FileEntry::pointer_t ent(zf.getEntry(argv[2], zipios::FileCollection::MatchPath::IGNORE));
        if(ent)
        {
            zipios::ZipFile::stream_pointer_t is(zf.getInputStream(ent->getName()));
            if(is)
            {
                std::cout << "Contents of entry, " << ent->getName() << ":" << std::endl;

                std::cout << is->rdbuf();
            }
            else
            {
                std::cerr << "error: found an entry for " << ent->getName() << " in test.zip, but could not read it with getInputStream()" << std::endl;
            }
        }
        else
        {
            std::cerr << "error: could not read " << ent->getName() << " from test.zip" << std::endl;
        }

        std::cout << "end of main()" << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Exception caught in main() :" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// Local Variables:
// mode: cpp
// indent-tabs-mode: nil
// c-basic-offset: 4
// tab-width: 4
// End:

// vim: ts=4 sw=4 et
