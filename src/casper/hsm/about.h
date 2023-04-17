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
#ifndef CASPER_HSM_ABOUT_H_
#define CASPER_HSM_ABOUT_H_

namespace casper
{
    namespace hsm
    {

        const char* const ABBR () __attribute__((used));
        const char* const NAME () __attribute__((used));
        const char* const VERSION () __attribute__((used));
        const char* const REL_NAME () __attribute__((used));
        const char* const REL_DATE () __attribute__((used));
        const char* const REL_BRANCH () __attribute__((used));
        const char* const REL_HASH () __attribute__((used));
        const char* const INFO () __attribute__((used));

    } // end of namespace 'hsm'
} // end of namespace 'casper'

#endif // CASPER_HSM_ABOUT_H_
