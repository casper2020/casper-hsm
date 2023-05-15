/**
 * @file ngx_http_casper_broker_hsm_module.h
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
#ifndef NRS_NGX_HTTP_CASPER_BROKER_HSM_MODULE_H_
#define NRS_NGX_HTTP_CASPER_BROKER_HSM_MODULE_H_

extern "C" {
    #include <ngx_config.h>
    #include <ngx_core.h>
    #include <ngx_http.h>
    #include <ngx_errno.h>
}

#ifdef __APPLE__
#pragma mark - module loc_conf_t
#endif

/**
 * @brief Module configuration structure, applicable to a location scope
 */
typedef struct {
    ngx_str_t config;
} nginx_hsm_service_fake_conf_t;

typedef struct {
    ngx_flag_t                    enabled;
    ngx_uint_t                    slot_id;
    ngx_str_t                     pin;
    nginx_hsm_service_fake_conf_t fake;
} nginx_hsm_service_conf_t;

typedef struct {
    ngx_flag_t  enable;    //!< flag that enables the module
    ngx_str_t   log_token; //!<
    ngx_flag_t  singleton; //!<
} ngx_http_casper_broker_hsm_module_loc_conf_t;

extern ngx_module_t ngx_http_casper_broker_hsm_module;

#endif // NRS_NGX_HTTP_CASPER_BROKER_HSM_MODULE_H_
