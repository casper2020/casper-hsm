/**
 * @file version.h
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
#ifndef NRS_NGX_CASPER_BROKER_HSM_VERSION_H_
#define NRS_NGX_CASPER_BROKER_HSM_VERSION_H_

#include "core/nginx.h"

#ifndef NGX_CASPER_BROKER_HSM_MODULE_NAME
#define NGX_CASPER_BROKER_HSM_MODULE_NAME "hsm"
#endif

#ifndef NGX_CASPER_BROKER_HSM_MODULE_VERSION
#define NGX_CASPER_BROKER_HSM_MODULE_VERSION "@MODULE_VERSION@"
#endif

#ifndef NGX_CASPER_BROKER_HSM_MODULE_INFO
#define NGX_CASPER_BROKER_HSM_MODULE_INFO NGX_CASPER_BROKER_HSM_MODULE_NAME " v" NGX_CASPER_BROKER_HSM_MODULE_VERSION "/" NGINX_VERSION
#endif

#endif // NRS_NGX_CASPER_BROKER_HSM_VERSION_H_
