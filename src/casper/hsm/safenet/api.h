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
#ifndef CASPER_HSM_SAFENET_API_H_
#define CASPER_HSM_SAFENET_API_H_

#include "casper/hsm/api.h"

#include "cryptoki_v2.h"

/* 19 byte ASN1 header + sha256 ( 32 byte ) size */
#define CASPER_HSM_API_ASN1_PLUS_SHA256_LEN 19 + 32
#undef  CASPER_HSM_API_ENABLE_SFNT_FUNCTIONS
#define CASPER_HSM_API_MAX_PIN_SIZE          64 // defined by client library

namespace casper
{

    namespace hsm
    {
        
        namespace safenet
        {
            
            class API final : public ::casper::hsm::API
            {
                
            private: // Data Type(s)
                
                typedef struct {
                    const char* const where_;
                    const CK_RV       rv_;
                } NoExceptionCallResult;
                
            private: // Const Data
                
                const SlotID              slot_id_;
                const bool                reuse_session_;
                
            private: // Data
                
                void*                     dl_handle_;
                CK_FUNCTION_LIST*         p11_functions_;
        #if defined(CASPER_HSM_API_ENABLE_SFNT_FUNCTIONS)
                CK_SFNT_CA_FUNCTION_LIST* sfnt_functions_;
        #endif
                CK_SESSION_HANDLE         session_;
                CK_BYTE                   dpin_[CASPER_HSM_API_MAX_PIN_SIZE];
                CK_ULONG                  lpin_;
                bool                      vpin_;

            private: // Data
                
                CK_BYTE signing_data_[CASPER_HSM_API_ASN1_PLUS_SHA256_LEN];
                
            public: // Constructor(s) / Destructor
                
                API () = delete;
                API (const std::string& a_application) = delete;
                API (const std::string& a_application, const SlotID a_slot, const std::string& a_pin, const bool a_reuse_session);
                API (const API& a_api);
                virtual ~API();
                
            public: // Method(s) // Function(s) - ::casper::hsm::API
                
                virtual void Load   ();
                virtual void Sign   (const std::string& a_key, const std::string& a_hash, std::string& o_signature);
                virtual void Unload () noexcept;
            
            private: // Method(s) // Function(s)
                
                void Reset () noexcept;

                void SetSigningBytes (const std::string& a_hash, CK_BYTE o_bytes[CASPER_HSM_API_ASN1_PLUS_SHA256_LEN]) const;

                NoExceptionCallResult OpenSession  () noexcept;
                NoExceptionCallResult CloseSession () noexcept;
                
                NoExceptionCallResult FindPrivateKey (const CK_SESSION_HANDLE& a_session, const std::string& a_name, CK_OBJECT_HANDLE& o_key) const noexcept;
                NoExceptionCallResult GetObjectLabel (const CK_SESSION_HANDLE& a_session, const CK_OBJECT_HANDLE& a_object, std::string& o_value) const noexcept;
                
            }; // end of class 'API'
            
        } // end of namespace 'safenet'
                
    } // end of namespace 'hsm'

} // end of namespace 'casper'

#endif // CASPER_HSM_SAFENET_API_H_
