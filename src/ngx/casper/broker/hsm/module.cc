/**
 * @file module.cc
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

#include "ngx/casper/broker/hsm/module.h"

#include "ngx/casper/broker/hsm/errors.h"

#include "cc/exception.h"

#include "cc/easy/json.h"

#ifdef __APPLE__
  #include "casper/hsm/fake/api.h"
#else
    #include "casper/hsm/safenet/api.h"
#endif

#include "casper/hsm/singleton.h"

#include "ngx/version.h"

#if defined(__APPLE__) && defined(CC_DEBUG_ON)
    #include "cc/global/initializer.h"
#endif


/**
 * @brief Default constructor.
 *
 * @param a_config
 * @param a_params
 * @param a_ngx_loc_conf
 * @param a_ngx_hsm_loc_conf
 */
ngx::casper::broker::hsm::Module::Module (const ngx::casper::broker::Module::Config& a_config, const ngx::casper::broker::Module::Params& a_params,
                                          ngx_http_casper_broker_module_loc_conf_t& a_ngx_loc_conf, ngx_http_casper_broker_hsm_module_loc_conf_t& a_ngx_hsm_loc_conf)
    : ngx::casper::broker::Module("hsm", a_config, a_params),
      use_singleton_(1 == a_ngx_hsm_loc_conf.singleton)
{
    // ...
    body_read_supported_methods_ = {
        NGX_HTTP_POST
    };
    body_read_allow_empty_methods_ = {
      NGX_HTTP_POST
    };
    // ...
#if defined(__APPLE__) && defined(CC_DEBUG_ON)
    ctx_.log_body_ = (
                       ( true == ::cc::global::Initializer::GetInstance().IsBeingDebugged() )
                        ||
                       ( 1 == a_ngx_loc_conf.cc_log.set && 1 == a_ngx_loc_conf.cc_log.write_body )
                     );
#else
    ctx_.log_body_ = ( 1 == a_ngx_loc_conf.cc_log.set && 1 == a_ngx_loc_conf.cc_log.write_body );
#endif
}

/**
 * @brief Destructor.
 */
ngx::casper::broker::hsm::Module::~Module ()
{
    /* empty */
}

/**
 * @brief Process the previously defined HTTP request.
 *
 * @return
 */
ngx_int_t ngx::casper::broker::hsm::Module::Run ()
{
    //
    // Request:
    //
    //   Content-Type: application/json
    //   Method      : POST
    //    Body       : { "key": <string>, "hash": <string> }
    //      or
    //    Body       : { "key": <string>, "hash": [<string>] }
    //
    // Response:
    //
    //      400: Bad Request - when missing or invalid body
    //      404: Not Found   - when there is no certificate available
    //      200: Ok
    //              - HSM APIs   : HSM => { hsm: { "provider": "HSM", "signing": <certificate>, "intermediate": <certificate>, "root": <certificate>, "pin": <PIN> , "otp": <OTP>}}
    //
    
    // ... starts as a bad request ...
    ctx_.response_.status_code_ = NGX_HTTP_BAD_REQUEST;
    ctx_.response_.return_code_ = NGX_OK;

    ::casper::hsm::API* api = nullptr;
    
    try {

        std::string key;
        Json::Value hash;
        try {
            
            const ::cc::easy::JSON<::cc::Exception> json;
            Json::Value                             request;
            
            json.Parse(ctx_.request_.body_, request);
            
            key  = json.Get(request, "key" , Json::ValueType::stringValue, /* a_default */ nullptr).asString();

            const Json::Value& hash_ref = json.Get(request, "hash", { Json::ValueType::stringValue, Json::ValueType::arrayValue }, &Json::Value::null);
            if ( true == hash_ref.isString() ) {
                hash = Json::Value(Json::ValueType::arrayValue);
                hash.append(hash_ref);
            } else {
                hash = hash_ref;
            }
                        
        } catch (const ::cc::Exception& a_cc_exception) {
            NGX_BROKER_MODULE_SET_BAD_REQUEST_EXCEPTION(ctx_, a_cc_exception);
        }
        
        // ... continue?
        if ( NGX_OK == ctx_.response_.return_code_ ) {
            std::string signature;
            // ... use HSM to sign hash ...
            if ( false == use_singleton_ ) {
                ::casper::hsm::Singleton::GetInstance().Recycle();
            }
            // ... prepare response ...
            Json::Value response = Json::Value(Json::ValueType::objectValue);
            response["signatures"] = Json::Value(Json::ValueType::arrayValue);
            // ... sign ...
            for ( Json::ArrayIndex idx = 0 ; idx < hash.size() ; ++idx ) {
                ::casper::hsm::Singleton::GetInstance().Sign(key, hash[idx].asString(), signature);
                response["signatures"].append(Json::Value(signature));
            }
            // ... serialize response ...
            Json::FastWriter fw; fw.omitEndingLineFeed();
            // ... done ...
            NGX_BROKER_MODULE_SET_RESPONSE(ctx_, NGX_HTTP_OK, ctx_.response_.content_type_, fw.write(response));
        }
    } catch (const ::cc::Exception& a_cc_exception) {
        // ... cleanup ...
        if ( nullptr != api ) {
            delete api;
        }
        // ... report ...
        NGX_BROKER_MODULE_SET_INTERNAL_SERVER_ERROR(ctx_, a_cc_exception.what());
    } catch (...) {
        // ... cleanup ...
        if ( nullptr != api ) {
            delete api;
        }
        try {
            ::cc::Exception::Rethrow(/* a_unhandled */ false, __FILE__, __LINE__, __FUNCTION__);
        } catch (const ::cc::Exception& a_cc_exception) {
            // ... report ...
            NGX_BROKER_MODULE_SET_INTERNAL_SERVER_ERROR(ctx_, a_cc_exception.what());
        }
    }

    // ... no error, just should be scheduled ...
    return ctx_.response_.return_code_;
}

// MARK: -

/**
 * @brief Content handler factory.
 *
 * @param a_r
 * @param a_at_rewrite_handler
 */
ngx_int_t ngx::casper::broker::hsm::Module::Factory (ngx_http_request_t* a_r, bool a_at_rewrite_handler)
{
    //
    // GRAB 'MAIN' CONFIG
    //
    ngx_http_casper_broker_module_loc_conf_t* broker_conf = (ngx_http_casper_broker_module_loc_conf_t*)ngx_http_get_module_loc_conf(a_r, ngx_http_casper_broker_module);
    if ( NULL == broker_conf ) {
        return NGX_ERROR;
    }

    //
    // GRAB 'MODULE' CONFIG
    //
    ngx_http_casper_broker_hsm_module_loc_conf_t* loc_conf =
    (ngx_http_casper_broker_hsm_module_loc_conf_t*)ngx_http_get_module_loc_conf(a_r, ngx_http_casper_broker_hsm_module);
    if ( NULL == loc_conf ) {
        return NGX_ERROR;
    }

    //
    // 'WARM UP'
    //
    ngx::casper::broker::Module::Params params = {
        /* in_headers_              */ {},
        /* config_                  */ {},
        /* locale_                  */ "",
        /* supported_content_types_ */ {
            "application/json", "application/json; charset=UTF-8",
            "application/vnd.api+json", "application/vnd.api+json;charset=utf-8'",
            "text/plain", "text/plain; charset=UTF-8",
            "application/x-www-form-urlencoded"        
        }
    };
    
    ngx_int_t rv = ngx::casper::broker::Module::WarmUp(ngx_http_casper_broker_hsm_module, a_r, loc_conf->log_token,
                                                       params);
    if ( NGX_OK != rv ) {
        return rv;
    }
    
    //
    // 'MODULE' SPECIFIC CONFIG
    //
    ngx::casper::broker::hsm::Errors configuration_errors(params.locale_);
    
    rv = ngx::casper::broker::Module::EnsureDirectives(ngx_http_casper_broker_hsm_module, a_r, loc_conf->log_token,
                                                       {
                                                       },
                                                       &configuration_errors
    );
    if ( NGX_OK != rv ) {
        return rv;
    }
    
    const ngx::casper::broker::Module::Config config = {
        /* ngx_module_            */ ngx_http_casper_broker_hsm_module,
        /* ngx_ptr_               */ a_r,
        /* ngx_body_read_handler_ */ ngx::casper::broker::hsm::Module::ReadBodyHandler,
        /* ngx_cleanup_handler_   */ ngx::casper::broker::hsm::Module::CleanupHandler,
        /* rx_content_type_       */ "text/plain; charset=utf-8",
        /* tx_content_type_       */ "application/json",
        /* log_token_             */ std::string(reinterpret_cast<const char*>(loc_conf->log_token.data), loc_conf->log_token.len),
        /* errors_factory_        */
        [] (const std::string& a_locale) {
            return new ngx::casper::broker::hsm::Errors(a_locale);
        },
        /* executor_factory_      */ nullptr,
        /* landing_page_url_      */ "",
        /* error_page_url_        */ "",
        /* serialize_errors_      */ true,
        /* at_rewrite_handler_    */ a_at_rewrite_handler
    };
    
    return ::ngx::casper::broker::Module::Initialize(config, params,
                                                     [&config, &params, &broker_conf, &loc_conf] () -> ::ngx::casper::broker::Module* {
                                                         return new ::ngx::casper::broker::hsm::Module(config, params, *broker_conf, *loc_conf);
                                                     }
    );
}

// MARK: -

/**
 * @brief Called by nginx a request body is read to be read.
 *
 * @param a_request.
 */
void ngx::casper::broker::hsm::Module::ReadBodyHandler (ngx_http_request_t* a_r)
{
    ngx::casper::broker::Module::ReadBody(ngx_http_casper_broker_hsm_module, a_r);
}

/**
 * @brief This method will be called when nginx is about to finalize a connection.
 *
 * @param a_data In this module, is the pointer to the request it self.
 */
void ngx::casper::broker::hsm::Module::CleanupHandler (void* a_data)
{
    ngx::casper::broker::Module::Cleanup(ngx_http_casper_broker_hsm_module, a_data);
}
