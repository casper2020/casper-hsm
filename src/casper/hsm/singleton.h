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
#pragma once
#ifndef CASPER_HSM_SINGLETON_H_
#define CASPER_HSM_SINGLETON_H_

#include "cc/singleton.h"

#include "casper/hsm/api.h"

namespace casper
{
    
    namespace hsm
    {
    
    // ---- //
    class Singleton;
    class Initializer final : public ::cc::Initializer<Singleton>
    {
        
    public: // Constructor(s) / Destructor
        
        Initializer (Singleton& a_instance);
        virtual ~Initializer ();
        
    }; // end of class 'Initializer'
    
    // ---- //
    class Singleton final : public cc::Singleton<Singleton, Initializer>
    {
        
        friend class Initializer;
        
        typedef struct {
            std::function<::casper::hsm::API*()>                          new_;
            std::function<::casper::hsm::API*(const ::casper::hsm::API*)> clone_;
        } Factory;
            
        private: // Data
            
            std::string share_dir_;
            API*        api_;
            Factory     factory_;
            
        public: // Method(s) / Function(s) - Oneshot call only!!!
            
            void Startup  (const std::string& a_share_dir, Factory a_factory);
            void Recycle  ();
            void Shutdown ();
            void Sign     (const std::string& a_key, const std::string& a_hash, std::string& o_signature);
            
        }; // end of class 'Singleton'
        
    } // end of namespace 'hsm'
    
} // end of namespace 'casper'

#endif // CASPER_HSM_SINGLETON_H_
