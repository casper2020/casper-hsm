/**
 * @file errors.h
 *
 * Copyright (c) 2017-2023 Cloudware S.A. All rights reserved.
 *
 * This file is part of nginx-hsm.
 *
 * nginx-hsm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nginx-hsm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with nginx-hsm. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef NRS_NGX_CASPER_BROKER_HSM_ERRORS_H_
#define NRS_NGX_CASPER_BROKER_HSM_ERRORS_H_

#include "cc/errors/jsonapi/tracker.h"

namespace ngx
{
    
    namespace casper
    {
        
        namespace broker
        {
            
            namespace hsm
            {
                
                class Errors final : public cc::errors::jsonapi::Tracker
                {
                    
                public: // Constructor (s) / Destructor
                    
                    Errors (const std::string& a_locale);
                    virtual ~Errors ();
                    
                }; // end of class 'Errors'
                
            } // end of namespace 'hsm'
            
        } // end of namespace 'broker'
        
    } // end of namespace 'casper'
    
} // end of namespace 'ngx'

#endif // NRS_NGX_CASPER_BROKER_HSM_ERRORS_H_
