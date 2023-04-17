/**
 * @file version.h
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
 * along with hsm. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef CASPER_HSM_VERSION_H_
#define CASPER_HSM_VERSION_H_

#ifndef CASPER_HSM_LIB_NAME
#define CASPER_HSM_LIB_NAME "hsm"
#endif

#ifndef CASPER_HSM_LIB_VERSION
#define CASPER_HSM_LIB_VERSION "@VERSION@"
#endif

#ifndef CASPER_HSM_LIB_INFO
#define CASPER_HSM_LIB_INFO CASPER_HSM_LIB_NAME " v" CASPER_HSM_LIB_VERSION
#endif

#endif // CASPER_HSM_VERSION_H_
