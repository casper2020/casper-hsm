/**
 * @file api.cc
 *
 * Copyright (c) 2011-2023 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper-hsm.
 *
 * hsm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hsm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper. If not, see <http://www.gnu.org/licenses/>.
 */

#include "casper/hsm/api.h"

#include <fstream>  // std::ifstream
        
#include "cc/fs/file.h"

/**
 * @brief Default constructor.
 *
 * @param a_application   Application name.
 */
casper::hsm::API::API (const std::string& a_application)
    : application_(a_application)
{
    /* empty */
}

/**
 * @brief Copy constructor.
 */
casper::hsm::API::API (const casper::hsm::API& a_api)
 : application_(a_api.application_)
{
    /* empty */
}

/**
 * @brief Destructor.
 */
casper::hsm::API::~API ()
{
    /* empty */
}

/**
 * @brief Load shared resources.
 *
 * @param a_directory Shared resources directory URI.
 */
void casper::hsm::API::LoadSharedResources (const std::string& a_directory)
{
    std::string name;
    // ... cleanup ...
    certificates_.clear();
    // ... load ...
    ::cc::fs::File::Find(a_directory, /* a_pattern */ "*.crt",
                         [this, &name] (const std::string& a_uri) -> bool {
                            // ... get name ...
                            ::cc::fs::File::Name(a_uri, name);
                            name = name.substr(0, name.length() - 4);
                            // ... load ...
                            std::ifstream in_stream(a_uri, std::ifstream::in);
                            certificates_[name] = std::string((std::istreambuf_iterator<char>(in_stream)), std::istreambuf_iterator<char>());
                            // ... continue ...
                            return true;
                         }
    );
}

// MARK: -

/**
 * @brief Try-catch function call.
 *
 * @param a_run     Function to call.
 * @param a_cleanup Function to call for cleanup.
 */
void casper::hsm::API::TryCall (const std::function<void()>& a_run, const std::function<void()>& a_cleanup) const
{
    try {
        // ... perform ...
        a_run();
        // ... cleanup?
        if ( nullptr != a_cleanup ) {
            a_cleanup();
        }
    } catch (const ::cc::Exception& a_cc_exception) {
        // ... cleanup?
        if ( nullptr != a_cleanup ) {
            a_cleanup();
        }
        // ... notify ...
        throw a_cc_exception;
    } catch (...) {
        // ... cleanup?
        if ( nullptr != a_cleanup ) {
            a_cleanup();
        }
        // ... notify ...
        try {
            ::cc::Exception::Rethrow(/* a_unhandled */ false, __FILE__, __LINE__, __FUNCTION__);
        } catch (const ::cc::Exception& a_cc_exception) {
            throw ::casper::hsm::Exception(a_cc_exception);
        }
    }
}
