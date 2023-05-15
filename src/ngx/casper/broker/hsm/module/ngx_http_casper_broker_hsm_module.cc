/**
 * @file ngx_http_casper_broker_hsm_module.cc
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

#include "ngx/casper/broker/hsm/module/ngx_http_casper_broker_hsm_module.h"

#include "ngx/casper/broker/hsm/module.h"

#include <sys/stat.h>

#ifndef __APPLE__ // backtrace
   #include <stdio.h>
   #include <execinfo.h>
   #include <signal.h>
   #include <stdlib.h>
   #include <unistd.h>
#else
    #include <execinfo.h>
    #include <stdio.h>
#endif

#ifdef __APPLE__
#pragma mark -
#pragma mark - Module - Forward declarations
#pragma mark -
#endif

static void* ngx_http_casper_broker_hsm_module_create_main_conf    (ngx_conf_t* a_cf);
static char* ngx_http_casper_broker_hsm_module_init_main_conf      (ngx_conf_t* a_cf, void* a_conf);

static void*     ngx_http_casper_broker_hsm_module_create_loc_conf (ngx_conf_t* a_cf);
static char*     ngx_http_casper_broker_hsm_module_merge_loc_conf  (ngx_conf_t* a_cf, void* a_parent, void* a_child);

static ngx_int_t ngx_http_casper_broker_hsm_module_filter_init     (ngx_conf_t* a_cf);
static ngx_int_t ngx_http_casper_broker_hsm_module_content_handler (ngx_http_request_t* a_r);
static ngx_int_t ngx_http_casper_broker_hsm_module_rewrite_handler (ngx_http_request_t* a_r);

#ifdef __APPLE__
#pragma mark -
#pragma mark - Data && Data Types
#pragma mark -
#endif

NGX_BROKER_MODULE_DECLARE_MODULE_ENABLER;

/**
 * @brief This struct defines the configuration command handlers
 */
static ngx_command_t ngx_http_casper_broker_hsm_module_commands[] = {
    /* service */
    {
        ngx_string("nginx_casper_broker_hsm_enabled"),
        NGX_HTTP_MAIN_CONF | NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(nginx_hsm_service_conf_t, enabled),
        NULL
    },
    {
        ngx_string("nginx_casper_broker_hsm_slot_id"),
        NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(nginx_hsm_service_conf_t, slot_id),
        NULL
    },
    {
        ngx_string("nginx_casper_broker_hsm_pin"),
        NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(nginx_hsm_service_conf_t, pin),
        NULL
    },
    {
         ngx_string("nginx_casper_broker_hsm_fake_config"),
         NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE1,
         ngx_conf_set_str_slot,
         NGX_HTTP_MAIN_CONF_OFFSET,
         offsetof(nginx_hsm_service_conf_t, fake.config),
         NULL
    },
    /* location */
    {
        ngx_string("nginx_casper_broker_hsm"),                          /* directive name */
        NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,                              /* legal on location context and takes a boolean ("on" or "off") */
        ngx_conf_set_flag_slot,                                         /* translates "on" or "off" to 1 or 0 */
        NGX_HTTP_LOC_CONF_OFFSET,                                       /* value saved on the location struct configuration ... */
        offsetof(ngx_http_casper_broker_hsm_module_loc_conf_t, enable), /* ... on the 'enable' element */
        NULL
    },
    {
        ngx_string("nginx_casper_broker_hsm_log_token"),
        NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_casper_broker_hsm_module_loc_conf_t, log_token),
        NULL
    },
    {
         ngx_string("nginx_casper_broker_hsm_singleton"),
         NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE1,
         ngx_conf_set_flag_slot,
         NGX_HTTP_LOC_CONF_OFFSET,
         offsetof(ngx_http_casper_broker_hsm_module_loc_conf_t, singleton),
         NULL
    },
    /* */
    ngx_null_command
};

/**
 * @brief The nginx-hsm 'api' module context setup data.
 */
static ngx_http_module_t ngx_http_casper_broker_hsm_module_ctx = {
    NULL,                                               /* preconfiguration              */
    ngx_http_casper_broker_hsm_module_filter_init,      /* postconfiguration             */
    ngx_http_casper_broker_hsm_module_create_main_conf, /* create main configuration     */
    ngx_http_casper_broker_hsm_module_init_main_conf,   /* init main configuration       */
    NULL,                                               /* create server configuration   */
    NULL,                                               /* merge server configuration    */
    ngx_http_casper_broker_hsm_module_create_loc_conf,  /* create location configuration */
    ngx_http_casper_broker_hsm_module_merge_loc_conf    /* merge location configuration  */
};

/**
 * @brief The nginx-hsm 'api' module setup data.
 */
ngx_module_t ngx_http_casper_broker_hsm_module = {
    NGX_MODULE_V1,
    &ngx_http_casper_broker_hsm_module_ctx,     /* module context    */
    ngx_http_casper_broker_hsm_module_commands, /* module directives */
    NGX_HTTP_MODULE,                            /* module type       */
    NULL,                                       /* init master       */
    NULL,                                       /* init module       */
    NULL,                                       /* init process      */
    NULL,                                       /* init thread       */
    NULL,                                       /* exit thread       */
    NULL,                                       /* exit process      */
    NULL,                                       /* exit master       */
    NGX_MODULE_V1_PADDING
};

// MARK: - Module - Implementation

/**
 * @brief Allocate module 'main' config.
 *
 * param a_cf
 */
static void* ngx_http_casper_broker_hsm_module_create_main_conf(ngx_conf_t* a_cf)
{
    nginx_hsm_service_conf_t* conf = (nginx_hsm_service_conf_t*) ngx_pcalloc(a_cf->pool, sizeof(nginx_hsm_service_conf_t));
    if ( NULL == conf ) {
        return NGX_CONF_ERROR;
    }

    conf->enabled     = NGX_CONF_UNSET;
    conf->slot_id     = NGX_CONF_UNSET_UINT;
    conf->pin         = ngx_null_string;
    conf->fake.config = ngx_null_string;

    // ... done ...
    return conf;
}

/**
 * @brief Initialize module 'main' config.
 *
 * param a_cf
 * param a_conf
 */
#define nrs_conf_init_str_value(conf, default) \
    if ( NULL == conf.data ) { \
        conf.len  = strlen(default); \
        conf.data = (u_char*)ngx_palloc(a_cf->pool, conf.len); \
        ngx_memcpy(conf.data, default, conf.len); \
    }
static char* ngx_http_casper_broker_hsm_module_init_main_conf  (ngx_conf_t* a_cf, void* a_conf)
{
    nginx_hsm_service_conf_t* conf = (nginx_hsm_service_conf_t*)a_conf;
    
    ngx_conf_init_value     (conf->enabled    ,  0);  /* 0 - disabled */
    ngx_conf_init_uint_value(conf->slot_id    ,  3);
    nrs_conf_init_str_value (conf->pin        , "");
    nrs_conf_init_str_value (conf->fake.config, "");
    
    // ... done ...
    return NGX_CONF_OK;
}

/**
 * @brief Alocate the module configuration structure.
 *
 * @param a_cf
 */
static void* ngx_http_casper_broker_hsm_module_create_loc_conf (ngx_conf_t* a_cf)
{
    ngx_http_casper_broker_hsm_module_loc_conf_t* conf =
        (ngx_http_casper_broker_hsm_module_loc_conf_t*) ngx_pcalloc(a_cf->pool, sizeof(ngx_http_casper_broker_hsm_module_loc_conf_t));
    if ( NULL == conf ) {
        return NGX_CONF_ERROR;
    }

    conf->enable      = NGX_CONF_UNSET;
    conf->log_token   = ngx_null_string;
    conf->singleton   = NGX_CONF_UNSET;

    return conf;
}

/**
 * @brief The merge conf callback.
 *
 * @param a_cf
 * @param a_parent
 * @param a_child
 */
static char* ngx_http_casper_broker_hsm_module_merge_loc_conf (ngx_conf_t* /* a_cf */, void* a_parent, void* a_child)
{
    ngx_http_casper_broker_hsm_module_loc_conf_t* prev = (ngx_http_casper_broker_hsm_module_loc_conf_t*) a_parent;
    ngx_http_casper_broker_hsm_module_loc_conf_t* conf = (ngx_http_casper_broker_hsm_module_loc_conf_t*) a_child;

    ngx_conf_merge_value     (conf->enable     , prev->enable     ,           0 ); /* 0 - disabled */
    ngx_conf_merge_str_value (conf->log_token  , prev->log_token  , "hsm_module");
    ngx_conf_merge_value     (conf->singleton  , prev->singleton  ,           0 ); /* 0 - not set */

    NGX_BROKER_MODULE_LOC_CONF_MERGED();

    return (char*) NGX_CONF_OK;
}

/**
 * @brief Filter module boiler plate installation
 *
 * @param a_cf
 */
static ngx_int_t ngx_http_casper_broker_hsm_module_filter_init (ngx_conf_t* a_cf)
{
    /*
     * Install the rewrite handler
     */
    const ngx_int_t rv = NGX_BROKER_MODULE_INSTALL_REWRITE_HANDLER(ngx_http_casper_broker_hsm_module_rewrite_handler);
    if ( NGX_OK != rv ) {
        return rv;
    }
    
    /**
     * Install content handler.
     */
    return NGX_BROKER_MODULE_INSTALL_CONTENT_HANDLER(ngx_http_casper_broker_hsm_module_content_handler);
}

/**
 * @brief Content phase handler, sends the stashed response or if does not exist passes to next handler
 *
 * @param  a_r The http request
 *
 * @return @li NGX_DECLINED if the content is not produced here, pass to next
 *         @li the return of the content sender function
 */
static ngx_int_t ngx_http_casper_broker_hsm_module_content_handler (ngx_http_request_t* a_r)
{
    /*
     * Check if module is enabled and the request can be handled here.
     */
    NGX_BROKER_MODULE_CONTENT_HANDLER_BARRIER(a_r, ngx_http_casper_broker_hsm_module, ngx_http_casper_broker_hsm_module_loc_conf_t,
                                              "hsm_module");
    /*
     * This module is enabled, handle request.
     */
    return ngx::casper::broker::hsm::Module::ContentPhaseTackleResponse(a_r, ngx_http_casper_broker_hsm_module, "hsm_module");
}

/**
 * @brief
 *
 * @param a_r
 */
static ngx_int_t ngx_http_casper_broker_hsm_module_rewrite_handler (ngx_http_request_t* a_r)
{
    /*
     * Check if module is enabled and the request can be handled here.
     */
    NGX_BROKER_MODULE_REWRITE_HANDLER_BARRIER(a_r, ngx_http_casper_broker_hsm_module, ngx_http_casper_broker_hsm_module_loc_conf_t,
                                              "hsm_module");
   
    /*
    * This module is enabled, handle request.
    */
   return ngx::casper::broker::hsm::Module::RewritePhaseTackleResponse(a_r, ngx_http_casper_broker_hsm_module,
                                                                          "hsm_module",
                                                                          [a_r] () -> ngx_int_t {
                                                                            return ngx::casper::broker::hsm::Module::Factory(a_r, /* a_at_rewrite_handler */ true);
                                                                          }
   );
}
