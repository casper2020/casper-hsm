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

#ifndef CASPER_HSM_ABBR
#define CASPER_HSM_ABBR "chsm"
#endif

#ifndef CASPER_HSM_NAME
#define CASPER_HSM_NAME "casper-hsm"
#endif

#ifndef CASPER_HSM_VERSION
#define CASPER_HSM_VERSION "x.x.x"
#endif

#ifndef CASPER_HSM_REL_NAME
#define CASPER_HSM_REL_NAME "n.n.n"
#endif

#ifndef CASPER_HSM_REL_DATE
#define CASPER_HSM_REL_DATE "r.r.d"
#endif

#ifndef CASPER_HSM_REL_BRANCH
#define CASPER_HSM_REL_BRANCH "r.r.b"
#endif

#ifndef CASPER_HSM_REL_HASH
#define CASPER_HSM_REL_HASH "r.r.h"
#endif

#ifndef CASPER_HSM_REL_TARGET
#define CASPER_HSM_REL_TARGET "r.r.t"
#endif

#ifndef CASPER_HSM_INFO
#define CASPER_HSM_INFO CASPER_HSM_NAME " v" CASPER_HSM_VERSION
#endif

#endif // CASPER_HSM_VERSION_H_
