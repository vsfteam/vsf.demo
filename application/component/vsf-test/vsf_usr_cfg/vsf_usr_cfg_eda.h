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

#ifndef __VSF_USR_CFG_EDA_H__
#define __VSF_USR_CFG_EDA_H__

#define VSF_USE_KERNEL                          ENABLED
#define VSF_KERNEL_CFG_SUPPORT_THREAD           DISABLED
// Main runs in the idle task so it sits below every SWI-backed priority; the
// arch_preempt workers (posted at prio_0/prio_1) then preempt it instead of
// starving each other cooperatively. VSF_OS_CFG_PRIORITY_NUM is derived
// automatically from the available SWIs (PendSV + RP2040 device SWI,
// VSF_DEV_SWI_NUM=1) → 2. Hand-setting it (or VSF_USR_SWI_NUM) is unnecessary
// and trips a kernel_cfg.h #warning.
#define VSF_OS_CFG_MAIN_MODE                    VSF_OS_CFG_MAIN_MODE_IDLE

// arch_preempt exercises EDA priority preemption and requires the kernel, so it
// is only meaningful under this profile. Enable it here (it defaults DISABLED in
// vsf_test_arch.h for the bare-metal profile, which has no scheduler).
#define VSF_TEST_ARCH_PREEMPT_ENABLE            ENABLED

#endif
