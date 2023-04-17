/**
 * @file api.h
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
#ifndef CASPER_API_HSM_H_
#define CASPER_API_HSM_H_

#include "casper/hsm/api.h"

#include "cc/easy/json.h"

namespace casper
{

    namespace hsm
    {
        
        namespace fake
        {
        
            class API final : public ::casper::hsm::API
            {
                
            private: // Const Data
                
                const std::string config_;
                
            private: // Data
                
                Json::Value       cfg_;
                
            public: // Constructor(s) / Destructor
                
                API () = delete;
                API (const std::string& a_application, const std::string& a_config);
                API (const API& a_api);
                virtual ~API();
                
            public: // Method(s) // Function(s) - ::casper::hsm::API
                
                virtual void Load   ();
                virtual void Sign   (const std::string& a_key, const std::string& a_hash, std::string& o_signature);
                virtual void Unload () noexcept;
                
            }; // end of class 'API'
            
        } // end of namespace 'fake'
                
    } // end of namespace 'hsm'

} // end of namespace 'casper'

        
#endif // CASPER_HSM_API_H_
