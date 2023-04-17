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
#ifndef CASPER_HSM_API_H_
#define CASPER_HSM_API_H_

#include "cc/non-copyable.h"
#include "cc/non-movable.h"

#include "cc/exception.h"

#include <string>
#include <functional>
#include <map>

namespace casper
{

    namespace hsm
    {
        
        typedef unsigned long int SlotID; //!< Keep it compatible with 1st HSM API
        
        typedef ::cc::Exception Exception;
        
        class API : public ::cc::NonCopyable, public ::cc::NonMovable
        {
            
        private: // Const Data
            
            const std::string application_;
            
        private: // Data
            
            std::map<std::string, std::string> certificates_;
            
        public: // Constructor(s) / Destructor
            
            API () = delete;
            API (const API& a_api);
            API (const std::string& a_application);
            virtual ~API();
            
        public: // Method(s) // Function(s)
            
            virtual void Load   () = 0;
            virtual void Sign   (const std::string& a_key, const std::string& a_hash, std::string& o_signature) = 0;
            virtual void Unload () noexcept = 0;

        public: // Method(s) // Function(s)

            virtual void LoadSharedResources (const std::string& a_directory);

        protected: // Method(s) // Function(s)
            
            void TryCall (const std::function<void()>& a_run, const std::function<void()>& a_cleanup) const;
        
        protected: // Inline Method(s) // Function(s)
            
            /**
             * @return R/O access to loaded certificates map.
             */
            inline const std::map<std::string, std::string> certificates () const
            {
                return certificates_;
            }
            
        }; // end of class 'API'
        
    } // end of namespace 'hsm'

} // end of namespace 'casper'

        
#endif // CASPER_HSM_API_H_
