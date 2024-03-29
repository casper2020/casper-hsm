/**
 * @file errors.cc
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

#include "ngx/casper/broker/hsm/errors.h"

/**
 * @brief Default constructor.
 *
 * @param a_locale
 */
ngx::casper::broker::hsm::Errors::Errors (const std::string& a_locale)
    : cc::errors::jsonapi::Tracker(a_locale, "application/vnd.api+json",
                                   /* a_generic_error_message               */ "BROKER_AN_ERROR_OCCURRED_MESSAGE",
                                   /* a_generic_error_message_with_code_key */ "BROKER_AN_ERROR_OCCURRED_MESSAGE_WITH_CODE",
                                   /* a_enable_nice_piece_of_code_a_k_a_ordered_json */ false)
{
    /* empty */
}

/**
 * @brief Destructor.
 */
ngx::casper::broker::hsm::Errors::~Errors ()
{
    /* empty */
}
