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

#include "casper/hsm/fake/api.h"

#include "cc/b64.h"

#include "cc/crypto/rsa.h"

#include "cc/macros.h"

#include "ed.h"

#include <fstream>  // std::ifstream

/**
 * @brief Default constructor.
 *
 * @param a_application Application name.
 * @param a_config      JSON config.
 */
casper::hsm::fake::API::API (const std::string& a_application, const std::string& a_config)
    : casper::hsm::API(a_application), config_(a_config)
{
    /* empty */
}

/**
 * @brief Copy constructor.
 */
casper::hsm::fake::API::API (const casper::hsm::fake::API& a_api)
    : casper::hsm::API(a_api), config_(a_api.config_)
{
    /* empty */
}

/**
 * @brief Destructor.
 */
casper::hsm::fake::API::~API ()
{
    /* empty */
}

/**
 * @brief Load shared library and functions, also initialize usage.
 */
void casper::hsm::fake::API::Load ()
{
    const ::cc::easy::JSON<::casper::hsm::Exception> json;
    //
    // { "<a_key>": { "key": <uri>, "pwd":<base64 encrypted password>" }}
    //
    // ... cleanup ...
    cfg_.clear();
    // ... parse ...
    json.Parse(config_, cfg_);
    for ( auto member : cfg_.getMemberNames() ) {
        auto& obj = cfg_[member];
        auto  tmp = _edd(obj["pwd"].asString());
        obj["pwd"] = ::cc::base64_rfc4648::encode(tmp.c_str(), tmp.length());
    }
}

/**
 * @brief Sign an hash.
 *
 * @param a_key         HSM private key token label.
 * @param a_hash        Base64-encoded hash value to be signed.
 * @param o_signature   Base64-encoded signature value.
 */
void casper::hsm::fake::API::Sign (const std::string& a_key, const std::string& a_hash, std::string& o_signature)
{
    unsigned char* ua = nullptr;    
    // ... check if required certificate exist ...
    {
        const auto& c = certificates();
        const auto  i = c.find(a_key);
        if ( c.end() == i ) {
            throw ::casper::hsm::Exception("Configuration error: certificate for %s not found!", a_key.c_str());
        }
    }
    // ... perform request ...
    TryCall(/* a_run */
            [this, &ua, &a_key, &a_hash, &o_signature] () {
                const ::cc::easy::JSON<::casper::hsm::Exception> json;
                // ...
                const auto& cfg = json.Get(cfg_, a_key.c_str(), Json::ValueType::objectValue, nullptr);
                const auto& key = json.Get(cfg,  "key"        , Json::ValueType::stringValue, nullptr);
                const auto& pwd = json.Get(cfg,  "pwd"        , Json::ValueType::stringValue, nullptr);
                // ... calculate maximum base64 decode size and ensure a buffer for decoder ...
                const size_t mds = ::cc::base64_rfc4648::decoded_max_size(a_hash.length());
                ua = new unsigned char[mds];
                // ... decode 'has' from base64 ...
                const size_t ds = ::cc::base64_rfc4648::decode(ua, mds, a_hash.c_str(), a_hash.length());
                // ... sign ...
                o_signature = ::cc::crypto::RSA::SignSHA256(ua, ds, key.asString(), ::cc::base64_rfc4648::decode<std::string>(pwd.asString()), ::cc::crypto::RSA::SignOutputFormat::BASE64_RFC4648);
            },
            /* a_cleanup */
            [&ua] () {
                if ( nullptr != ua ) {
                    delete [] ua;
                    ua = nullptr;
                }
            }
    );
    // ... sanity check ...
    CC_ASSERT(nullptr == ua);
}

/**
 * @brief Unload previously loaded shared library and functions, also close any open session.
 */
void casper::hsm::fake::API::Unload () noexcept
{
    cfg_.clear();
}
