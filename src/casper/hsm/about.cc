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
#include "casper/hsm/about.h"
#include "casper/hsm/version.h"

const char* const casper::hsm::ABBR ()
{
    return CASPER_HSM_ABBR;
}

const char* const casper::hsm::NAME ()
{
    return CASPER_HSM_NAME;
}

const char* const casper::hsm::VERSION ()
{
    return CASPER_HSM_VERSION;
}

const char* const casper::hsm::REL_NAME ()
{
    return CASPER_HSM_REL_NAME;
}

const char* const casper::hsm::REL_DATE ()
{
    return CASPER_HSM_REL_DATE;
}

const char* const casper::hsm::REL_BRANCH ()
{
    return CASPER_HSM_REL_BRANCH;
}

const char* const casper::hsm::REL_HASH ()
{
    return CASPER_HSM_REL_HASH;
}

const char* const casper::hsm::INFO ()
{
    return CASPER_HSM_INFO;
}
