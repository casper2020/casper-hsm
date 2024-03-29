/**
 * @file module.h
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
#ifndef NRS_NGX_CASPER_BROKER_HSM_MODULE_H_
#define NRS_NGX_CASPER_BROKER_HSM_MODULE_H_

#include "ngx/casper/broker/module.h"

#include "ngx/casper/broker/module/ngx_http_casper_broker_module.h"
#include "ngx/casper/broker/hsm/module/ngx_http_casper_broker_hsm_module.h"

namespace ngx
{
    
    namespace casper
    {
        
        namespace broker
        {
            
            namespace hsm
            {

                class Module final : public ::ngx::casper::broker::Module
                {
                private: // Const Data
                    
                    const bool use_singleton_;

                protected: // Constructor(s)
                    
                    Module (const broker::Module::Config& a_config, const broker::Module::Params& a_params,
                            ngx_http_casper_broker_module_loc_conf_t& a_ngx_broker_loc_conf, ngx_http_casper_broker_hsm_module_loc_conf_t& a_ngx_hsm_loc_conf);
                    
                public: // Constructor(s) / Destructor
                    
                    virtual ~Module();
                    
                public: // Inherited Method(s) / Function(s)
                    
                    virtual ngx_int_t Run ();
                    
                public: // Static Method(s) / Function(s)
                    
                    static ngx_int_t Factory (ngx_http_request_t* a_r, bool a_at_rewrite_handler);

                private: // Static Method(s) / Function(s)
                    
                    static void ReadBodyHandler (ngx_http_request_t* a_request);
                    static void CleanupHandler  (void*);

                }; // end of class 'Module'

            } // end of namespace 'hsm'
            
        } // end of namespace 'broker'
        
    } // end of namespace 'casper'
    
} // end of namespace 'ngx'

#endif // NRS_NGX_CASPER_BROKER_HSM_MODULE_H_
