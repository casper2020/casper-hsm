/**
 * @file singleton.h
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

#include "casper/hsm/singleton.h"

// MARK: -

/**
 * @brief Default constructor.
 */
casper::hsm::Initializer::Initializer (casper::hsm::Singleton& a_instance)
    : cc::Initializer<Singleton>(a_instance)
{
    instance_.api_     = nullptr;
    instance_.factory_ = { nullptr, nullptr };
}

/**
 * @brief Destructor.
 */
casper::hsm::Initializer::~Initializer ()
{
    if ( nullptr != instance_.api_ ) {
        instance_.api_->Unload();
        delete instance_.api_;
    }
}

// MARK: -

/**
 * @brief This method must ( and can only ) be called once to initialize HSM engine.
 *
 * @param a_share_dir Shared directory URI.
 * @param a_factory   Function to call to create new instances when needed.
 */
void casper::hsm::Singleton::Startup (const std::string& a_share_dir, Factory a_factory)
{
    // ... if already initialized ...
    if ( nullptr != api_ ) {
        // ... can't be initialized twice ...
        throw std::runtime_error("HSM API singleton already initialized!");
    }
    // ... keep track of factory ...
    share_dir_ = a_share_dir;
    factory_   = a_factory;
    // ... setup HSM session ...
    api_ = factory_.new_();
    api_->LoadSharedResources(share_dir_);
    api_->Load();
}

/**
 * @brief Recycle an API object.
 */
void casper::hsm::Singleton::Recycle ()
{
    // ... if NOT initialized ...
    if ( nullptr == api_ ) {
        // ... nothing to recycle ...
        return;
    }
    auto n = factory_.clone_(api_);
    delete api_;
    api_ = n;
    api_->LoadSharedResources(share_dir_);
    api_->Load();
}

/**
 * @brief Call this no longer required to be alive.
 */
void casper::hsm::Singleton::Shutdown ()
{
    // ... if NOT initialized ...
    if ( nullptr == api_ ) {
        // ... nothing to shutdown ...
        return;
    }
    // ... can be reused ...
    api_->Unload();
    delete api_;
    api_ = nullptr;
}

// MARK: -

/**
 * @brief Sign an hash.
 *
 * @param a_key         HSM private key token label.
 * @param a_hash        Base64-encoded hash value to be signed.
 * @param o_signature   Base64-encoded signature value.
 */
void casper::hsm::Singleton::Sign (const std::string& a_key, const std::string& a_hash, std::string& o_signature)
{
    // ... if NOT initialized ...
    if ( nullptr == api_ ) {
        throw std::runtime_error("HSM API singleton NOT initialized!");
    }
    api_->Sign(a_key, a_hash, o_signature);
}
   
