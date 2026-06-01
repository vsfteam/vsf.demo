/*****************************************************************************
 *   Copyright(C)2009-2026 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

#ifndef __VSF_USR_CFG_BAREMETAL_H__
#define __VSF_USR_CFG_BAREMETAL_H__

#ifndef VSF_USE_KERNEL
#   define VSF_USE_KERNEL                          DISABLED
#endif

// When the kernel is disabled, vsf_os.h (which normally defines these macros)
// is skipped entirely. queue_stream.c and other service code still reference
// vsf_protect_scheduler / vsf_unprotect_scheduler, so provide no-op stubs.
#ifndef vsf_protect_scheduler
#   define vsf_protect_scheduler()                 0
#endif
#ifndef vsf_unprotect_scheduler
#   define vsf_unprotect_scheduler(__state)        ((void)(__state))
#endif

// vsf_kernel_bsp.c defines main() and calls VSF_USER_ENTRY().
// When the kernel is disabled, define VSF_USER_ENTRY as main so the
// application entry in main.c becomes the C runtime entry point.
#ifndef VSF_USER_ENTRY
#   define VSF_USER_ENTRY                       main
#endif

#endif
