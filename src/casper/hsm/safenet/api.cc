/**
 * @file api.cc
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

#include "casper/hsm/safenet/api.h"

#include "cc/macros.h"
#include "cc/types.h"

#include "cc/exception.h"
#include "cc/b64.h"
#include "cc/hash/sha256.h"
#include "cc/fs/file.h"

#include <dlfcn.h> // dl*
#include <string.h> // memcpy, memset

#include <fstream>  // std::ifstream

#include "ed.h"

#define CASPER_HSM_API_MAX_FIND_HANDLES  1 // arbitrary

/**
 * @brief Default constructor.
 *
 * @param a_application   Application name.
 * @param a_slot_id       HSM user slot ID.
 * @param a_pin           USER PIN.
 * @param a_reuse_session When true, session will be reused.
 */
casper::hsm::safenet::API::API (const std::string& a_application, const CK_SLOT_ID a_slot_id, const std::string& a_pin,
                  const bool a_reuse_session)
: casper::hsm::API(a_application),
  slot_id_(a_slot_id), reuse_session_(a_reuse_session)
{
    dl_handle_      = nullptr;
    p11_functions_  = nullptr;
#if defined(CASPER_HSM_API_ENABLE_SFNT_FUNCTIONS)
    sfnt_functions_ = nullptr;
#endif
    session_        = CK_INVALID_HANDLE;
    memset(dpin_, 0, CASPER_HSM_API_MAX_PIN_SIZE);
    const auto tmp = _edd(a_pin);
    if ( tmp.length() > 0 && tmp.length() <= CASPER_HSM_API_MAX_PIN_SIZE ) {
        memcpy(dpin_, (const CK_BYTE*)tmp.c_str(), tmp.length());
        lpin_ = (CK_ULONG)strlen((char*)dpin_);
        vpin_ = true;
    } else {
        lpin_ = 0;
        vpin_ = false;
    }
}

/**
 * @brief Copy constructor.
 */
casper::hsm::safenet::API::API (const casper::hsm::safenet::API& a_api)

 : casper::hsm::API(a_api),
   slot_id_(a_api.slot_id_), reuse_session_(a_api.reuse_session_)
{
    dl_handle_      = nullptr;
    p11_functions_  = nullptr;
#if defined(CASPER_HSM_API_ENABLE_SFNT_FUNCTIONS)
    sfnt_functions_ = nullptr;
#endif
    session_        = CK_INVALID_HANDLE;
    memcpy(dpin_, a_api.dpin_, CASPER_HSM_API_MAX_PIN_SIZE);
    vpin_           = a_api.vpin_;
}

/**
 * @brief Destructor.
 */
casper::hsm::safenet::API::~API ()
{
    Unload();
}

/**
 * @brief Load shared library and functions, also initialize usage.
 */
void casper::hsm::safenet::API::Load ()
{
    // ... already loaded?
    if ( nullptr != dl_handle_ ) {
        // ... done ...
        return;
    }
#ifdef __APPLE__
    const char* const lib = "/usr/local/safenet/lunaclient/lib/libCryptoki2_64.so";
#else
    const char* const lib = "/usr/safenet/lunaclient/lib/libCryptoki2_64.so";
#endif
    // ... load the dynamic shared object ( shared library ) ...
    dl_handle_ = dlopen(lib, RTLD_NOW);
    if ( nullptr == dl_handle_) {
        throw ::casper::hsm::Exception("Unable to load shared library '%s': %s !", lib, dlerror());
    }
    // ... grab pointer to 'C_GetFunctionList' function from shared library ...
    CK_C_GetFunctionList C_GetFunctionList = (CK_C_GetFunctionList)dlsym(dl_handle_, "C_GetFunctionList");
    if ( nullptr == C_GetFunctionList ) {
        throw ::casper::hsm::Exception("An error occurred while %s: %s!", "obtain functions list handle", "nullptr");
    }
    CK_RV rv = CKR_TOKEN_NOT_PRESENT;
    // ... load P11 functions ...
    if ( CKR_OK != ( rv = C_GetFunctionList(&p11_functions_) ) ) {
        throw ::casper::hsm::Exception("An error occurred while %s: 0x%08lx!", "load functions list", rv);
    }
    // ... initialize library ...
    if ( CKR_OK != ( rv = p11_functions_->C_Initialize(NULL_PTR) ) ) {
        throw ::casper::hsm::Exception("An error occurred while %s: 0x%08lx!", "initialize functions", rv);
    }
#if defined(CASPER_HSM_API_ENABLE_SFNT_FUNCTIONS)
    // ... grab pointer to 'CA_GetFunctionList' function from shared library ...
    CK_CA_GetFunctionList CA_GetFunctionList = (CK_CA_GetFunctionList)dlsym(dl_handle_, "CA_GetFunctionList");
    if ( nullptr == CA_GetFunctionList ) {
        throw ::casper::hsm::Exception("An error occurred while %s: %s!", "obtain CA functions list handle", "nullptr");
    }
    // ... load SFNT functions ...
    if ( CKR_OK != ( rv = CA_GetFunctionList(&sfnt_functions_) ) ) {
        throw ::casper::hsm::Exception("An error occurred while %s: 0x%08lx!", "load CA functions", rv);
    }
#endif
    // ... load certificates ...
    // ... TODO ?
//    int app_id_n = 1;
//    const CK_APPLICATION_ID app_id = { application_ };
//    CA_OpenApplicationIDV2(ckSlot, &app_id); */
}

/**
 * @brief Unload previously loaded shared library and functions, also close any open session.
 */
void casper::hsm::safenet::API::Unload () noexcept
{
    // ... first close session ( if any ) ...
    CloseSession();
    // ... finalize library call and release library handle ...
    if ( nullptr != dl_handle_ ) {
        // TODO: FIX segmentation fault when calling:
        // p11_functions_->C_Finalize(NULL_PTR);
        dlclose(dl_handle_);
        dl_handle_ = nullptr;
    }
    // ... forget pointers to library functions ...
    p11_functions_  = nullptr;
#if defined(CASPER_HSM_API_ENABLE_SFNT_FUNCTIONS)
    sfnt_functions_ = nullptr;
#endif
}

// MARK: -

/**
 * @brief Reset reusable data.
 */
void casper::hsm::safenet::API::Reset () noexcept
{
    //    /* 19 byte ASN1 header + sha256 ( 32 byte ) size */
    //    signing_data_ = {
    //        /* 19 byte ASN1 structure from the IETF rfc3447 for SHA256 */
    //        0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20,
    //        /* the hash bytes, zero them out */
    //        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    //    };
    memcpy(signing_data_, ::cc::hash::SHA256::sk_signature_prefix_, ::cc::hash::SHA256::sk_signature_prefix_size_);
    memset(signing_data_ + 19, 0, 32);
}

/**
 * @brief Sign an hash.
 *
 * @param a_key       HSM private key token label.
 * @param a_hash      Base64-encoded hash value to be signed.
 * @param o_signature Base64-encoded signature value.
 */
void casper::hsm::safenet::API::Sign (const std::string& a_key, const std::string& a_hash, std::string& o_signature)
{
    CK_BYTE* signature_bytes = NULL_PTR;
    // ... reset reusable data ...
    Reset();
    // ... sanity check - we can't afford to pass invalid PIN values due to invalid size ...
    if ( false == vpin_ || 0 == lpin_ ) {
        throw ::casper::hsm::Exception("Configuration error: %s!", "invalid PIN");
    }    
    // ... perform request ...
    TryCall(/* a_run */
            [this, &a_key, &a_hash, &o_signature, &signature_bytes] () {
                
                CK_RV rv = CKR_TOKEN_NOT_PRESENT;
                
                casper::hsm::safenet::API::NoExceptionCallResult osr = OpenSession();
                if ( CKR_OK != osr.rv_ ) {
                    throw ::casper::hsm::Exception("An error occurred while calling '%s' function: 0x%08lx!", osr.where_, osr.rv_);
                }
                
                CK_MECHANISM_INFO info;
                if ( CKR_OK != ( rv = p11_functions_->C_GetMechanismInfo(slot_id_, CKM_SHA256_RSA_PKCS, &info) ) ) {
                    throw ::casper::hsm::Exception("An error occurred while calling '%s' function: 0x%08lx!", "C_GetMechanismInfo", rv);
                }
                
                CK_OBJECT_HANDLE key = CK_INVALID_HANDLE;
                const NoExceptionCallResult find_rv = FindPrivateKey(session_, a_key.c_str(), key);
                if ( CKR_OK != find_rv.rv_ ) {
                    throw ::casper::hsm::Exception("An error occurred while calling '%s' function: 0x%08lx!", "FindPrivateKey", rv);
                }

                //
                // 2.1.14 PKCS #1 v1.5 RSA signature with MD2, MD5, SHA-1, SHA-256, SHA-384, SHA-512, RIPE-MD 128 or RIPE-MD 160
                //
                // Likewise, the PKCS #1 v1.5 RSA signature with SHA-256, SHA-384, and SHA-512 mechanisms, denoted CKM_SHA256_RSA_PKCS, CKM_SHA384_RSA_PKCS, and CKM_SHA512_RSA_PKCS respectively,
                // perform the same operations using the SHA-256, SHA-384 and SHA-512 hash functions with the object identifiers sha256WithRSAEncryption, sha384WithRSAEncryption and sha512WithRSAEncryption respectively.
                
                // 2.1.6 PKCS #1 v1.5 RSA
                // The PKCS #1 v1.5 RSA mechanism, denoted CKM_RSA_PKCS, is a multi-purpose mechanism based on the RSA public-key cryptosystem and the block formats initially defined in PKCS #1 v1.5.
                // It supports single-part encryption and decryption; single-part signatures and verification with and without message recovery; key wrapping; and key unwrapping.
                // This mechanism corresponds only to the part of PKCS #1 v1.5 that involves RSA;
                // it does not compute a message digest or a DigestInfo encoding as specified for the md2withRSAEncryption and md5withRSAEncryption algorithms in PKCS #1 v1.5 .
                //
                
                // Using 2.1.6 PKCS #1 v1.5 RSA - CKM_RSA_PKCS.
                
                SetSigningBytes(a_hash, signing_data_);
                
                CK_MECHANISM mechanism = { /* mechanism */ CKM_RSA_PKCS, /* pParameter */ NULL_PTR, /* usParameterLen */ 0 };
                if ( CKR_OK != ( rv = p11_functions_->C_SignInit(session_, &mechanism, key) ) ) {
                    throw ::casper::hsm::Exception("An error occurred while calling '%s' function: 0x%08lx!", "C_SignInit", rv);
                }
                
                CK_ULONG signature_length = 0;
                
                if ( CKR_OK != ( rv = p11_functions_->C_Sign(session_, signing_data_, sizeof(signing_data_), NULL_PTR, &signature_length) ) ) {
                    throw ::casper::hsm::Exception("An error occurred while calling '%s' function: 0x%08lx!", "C_Sign ( to obain signature length )", rv);
                }
                // ... prepare signature buffer  ...
                signature_bytes = (CK_BYTE*)calloc(signature_length, 1);
                // ... sign ...
                if ( CKR_OK != ( rv = p11_functions_->C_Sign(session_, signing_data_, sizeof(signing_data_), signature_bytes, &signature_length) ) ) {
                    throw ::casper::hsm::Exception("An error occurred while calling '%s' function: 0x%08lx!", "C_Sign ( to sign data )", rv);
                }
                o_signature = ::cc::base64_rfc4648::encode(signature_bytes, static_cast<size_t>(signature_length));
            },
            /* a_cleanup */
            [this, &signature_bytes] () {
                // ...
                if ( NULL_PTR != signature_bytes ) {
                    free(signature_bytes);
                    signature_bytes = nullptr;
                }
                // ...
                CloseSession();
            }
    );
    // ... sanity check ...
    CC_ASSERT(nullptr == signature_bytes);
}

/**
 * @brief Set the appropriated signing data payload to be used with CKM_RSA_PKCS mechanism.
 *
 * @param a_hash  Base64-encoded hash value to be signed.
 * @param o_bytes Bytes to be signed, ASN1 header + sha256 ( Base64-decoded a_hash value ).
 */
void casper::hsm::safenet::API::SetSigningBytes (const std::string& a_hash, CK_BYTE o_bytes[CASPER_HSM_API_ASN1_PLUS_SHA256_LEN]) const
{
    unsigned char* buffer = nullptr;
    TryCall(/* a_run */
            [&buffer, &a_hash, &o_bytes] () {
                // ... calculate maximum base64 decode size and ensure a buffer for decoder ...
                const size_t mds = ::cc::base64_rfc4648::decoded_max_size(a_hash.length());
                buffer = new unsigned char[mds];
                // ... decode 'has' from base64 ...
                const size_t ds = ::cc::base64_rfc4648::decode(buffer, mds, a_hash.c_str(), a_hash.length());
                // ... calculate SHA256 digest ...
                ::cc::hash::SHA256 sha256;
                sha256.Initialize();
                sha256.Update(buffer, ds);
                const unsigned char* const digest = sha256.Final();
                // ... join SHA256 signature prefix and SHA56 digest ...
                memcpy(o_bytes, ::cc::hash::SHA256::sk_signature_prefix_, ::cc::hash::SHA256::sk_signature_prefix_size_);
                memcpy(o_bytes + ::cc::hash::SHA256::sk_signature_prefix_size_, digest, SHA256_DIGEST_LENGTH);
            },
            /* a_cleanup */
            [&buffer] () {
                if ( nullptr != buffer ) {
                    delete [] buffer;
                    buffer = nullptr;
                }
            }
    );
    // ... sanity check ...
    CC_ASSERT(nullptr == buffer);
}

/**
 * @brief Open a new session - using HSM client library.
 *
 * @return Call result, see \link HSM::NoExceptionCallResult \link.
 */
casper::hsm::safenet::API::NoExceptionCallResult casper::hsm::safenet::API::OpenSession () noexcept
{
    // ... if session can be reused ...
    if ( CK_INVALID_HANDLE != session_ && true == reuse_session_ ) {
        // ... done, reusing session ...
        return NoExceptionCallResult { nullptr, CKR_OK };
    }
    // ... close previous ( if any ) ...
    CloseSession();
    // ... sanity check - we can't afford to pass invalid PIN values due to invalid size ...
    if ( false == vpin_ || 0 == lpin_ ) {
        return NoExceptionCallResult { "OpenSession", CK_INVALID_HANDLE };
    }
    // ... open a new one ...
    // ...
    CK_RV rv = CKR_TOKEN_NOT_PRESENT;
    // ... new session ...
    if ( CKR_OK != ( rv = p11_functions_->C_OpenSession(slot_id_, CKF_RW_SESSION | CKF_SERIAL_SESSION, NULL, NULL, &session_) ) ) {
        // ... done, an error is set ...
        return NoExceptionCallResult { "C_OpenSession", rv };
    }
    // ... login ...
    if ( CKR_OK != ( rv = p11_functions_->C_Login(session_, CKU_CRYPTO_USER, dpin_, lpin_) ) ) {
        // ... forget session ...
        CloseSession();
        // ... done, an error is set ...
        return NoExceptionCallResult { "C_Login", rv };
    }
    // ... done ...
    return NoExceptionCallResult { nullptr, rv };
}

/**
 * @brief Close the current session - using HSM client library.
 *
 * @return Call result, see \link HSM::NoExceptionCallResult \link.
 */
casper::hsm::safenet::API::NoExceptionCallResult casper::hsm::safenet::API::CloseSession () noexcept
{
    CK_RV rv = CKR_OK;
    // ... if a session is open ...
    if ( CK_INVALID_HANDLE != session_ ) {
        // ... close it now ...
        rv = p11_functions_->C_CloseSession(session_);
        // ... reset ...
        session_ = CK_INVALID_HANDLE;
    }
    // ... done ...
    return NoExceptionCallResult { CKR_OK != rv ? "C_CloseSession" : nullptr, rv };
}

/**
 * @brief Find a private key via HSM token label - using HSM client library.
 *
 * @param a_session HSM session to use.
 * @param a_key     HSM private key token label.
 * @param o_key     HSM private key object handle.
 *
 * @return Call result, see \link HSM::NoExceptionCallResult \link.
 */
casper::hsm::safenet::API::NoExceptionCallResult casper::hsm::safenet::API::FindPrivateKey (const CK_SESSION_HANDLE& a_session, const std::string& a_key, CK_OBJECT_HANDLE& o_key) const noexcept
{
    // ... reset ...
    o_key = CKR_OBJECT_HANDLE_INVALID;
    // ...
    CK_OBJECT_CLASS object_class = CKO_PRIVATE_KEY;
    CK_BBOOL        true_value   = CK_TRUE;
    // ... set attribute values to match ...
    CK_ATTRIBUTE attributes[] = {
        {CKA_CLASS, &object_class, sizeof(object_class)},
        {CKA_TOKEN, &true_value, sizeof(true_value)},
        {CKA_PRIVATE, &true_value, sizeof(true_value)},
        // {CKA_MODIFIABLE, &false_value, sizeof(false_value)}
    };
    const CK_ULONG template_size = sizeof(attributes) / sizeof(CK_ATTRIBUTE);
    // ...
    CK_OBJECT_HANDLE handles[CASPER_HSM_API_MAX_FIND_HANDLES];
    CK_RV           rv;
    // ... initialize find operation ...
    rv = p11_functions_->C_FindObjectsInit(a_session, attributes, template_size);
    if ( CKR_OK != rv ) {
        // ... done, an error is set ...
        return NoExceptionCallResult { "C_FindObjectsInit", rv };
    }
    // ...
    CK_ULONG count;
    // ... get first set of objects ...
    rv = p11_functions_->C_FindObjects(a_session, handles, CASPER_HSM_API_MAX_FIND_HANDLES, &count);
    if ( CKR_OK != rv ) {
        // ... done, an error is set ...
        return NoExceptionCallResult { "C_FindObjects", rv };
    }
    // ... extract attribute, compare and keep searching if needed ...
    while ( count > 0 && CKR_OBJECT_HANDLE_INVALID == o_key ) {
        std::string name;
        // ... get label attribute value ...
        for ( CK_ULONG idx = 0; idx < count ; ++idx ) {
            const NoExceptionCallResult call_rv = GetObjectLabel(a_session, handles[idx], name);
            if ( CKR_OK == ( call_rv.rv_ ) ) {
                if ( 0 == strcmp(name.c_str(), a_key.c_str()) ) {
                    o_key = handles[idx];
                    break;
                }
            }
        }
        // ... stop search?
        if ( CKR_OBJECT_HANDLE_INVALID != o_key ) {
            break;
        }
        // ... continue search ...
        rv = p11_functions_->C_FindObjects(a_session, handles, CASPER_HSM_API_MAX_FIND_HANDLES, &count);
        if ( CKR_OK != rv ) {
            // ... done, an error is set ...
            return NoExceptionCallResult { "C_FindObjects", rv };
        }
    }
    // ... finalize find operation ...
    rv = p11_functions_->C_FindObjectsFinal(a_session);
    if ( CKR_OK != rv ) {
        // ... done, an error is set ...
        return NoExceptionCallResult { "C_FindObjectsFinal", rv };
    }
    // ... no key found?
    if ( CKR_OBJECT_HANDLE_INVALID == o_key ) {
        // ... done, an error is set ...
        return NoExceptionCallResult { "FindPrivateKey", CKR_OBJECT_HANDLE_INVALID };
    }
    // ... done ...
    return NoExceptionCallResult { nullptr, rv };
}

/**
 * @brief Get an object by label value - using HSM client library.
 *
 * @param a_session HSM session to use.
 * @param a_object  Object handle to extract label value from.
 * @param o_value   Extract label value from provided object.
 *
 * @return Call result, see \link HSM::NoExceptionCallResult \link.
 */
casper::hsm::safenet::API::NoExceptionCallResult casper::hsm::safenet::API::GetObjectLabel (const CK_SESSION_HANDLE& a_session, const CK_OBJECT_HANDLE& a_object, std::string& o_value) const noexcept
{
    CK_ATTRIBUTE attribute = { CKA_LABEL, NULL_PTR, 0 };
    CK_RV        rv;
    
    // ... get attribute value length ...
    if ( CKR_OK != ( rv = p11_functions_->C_GetAttributeValue(a_session, a_object, &attribute, 1) ) ) {
        // ... done, an error is set ...
        return NoExceptionCallResult { "C_GetAttributeValue", rv };
    }
    // .. allocate a buffer to set the attribute value ...
    char* buffer = new char[attribute.ulValueLen + 1];
    attribute.pValue = buffer;
    // ... get attribute value ...
    if ( CKR_OK != ( rv = p11_functions_->C_GetAttributeValue(a_session, a_object, &attribute, 1) ) ) {
        // ... cleanup ...
        delete [] buffer;
        // ... done, an error is set ...
        return NoExceptionCallResult { "C_GetAttributeValue", rv };
    }
    buffer[attribute.ulValueLen] = '\0';
    // ... copy value ...
    o_value = buffer;
    // ... cleanup ...
    delete [] buffer;
    // ... done ...
    return NoExceptionCallResult { nullptr, rv };
}
