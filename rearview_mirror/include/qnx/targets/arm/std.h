/* 
 *  Copyright (c) 2011 Texas Instruments and others.
 *  All rights reserved. This program and the accompanying materials
 *  are made available under the terms of the Eclipse Public License v1.0
 *  which accompanies this distribution, and is available at
 *  http://www.eclipse.org/legal/epl-v10.html
 *
 *  Contributors:
 *      Texas Instruments - initial implementation
 *
 * 
 */

/*
 *  ======== qnx/targets/arm/std.h ========
 *  Standard types for supported QNX Arm compilers
 */

#ifndef qnx_targets_arm_STD_
#define qnx_targets_arm_STD_

/*  Define target-specific "portable" macros
 *
 *  The build command-line define xdc_target_name__ to be the value
 *  of the target's name config parameter.  We use this to include the
 *  target-specific definitions for the required target-independent
 *  xdc_target* macros.
 */
#ifdef xdc_target_name__
#include xdc__local_include(xdc_target_name__)
#endif

/* "inherit" (i.e., include) all gnu.targets standard types */
#include <gnu/targets/std.h>

#endif /* qnx_targets_arm_STD_ */

/*
 *  @(#) qnx.targets.arm; 1, 0, 0,101; 6-24-2013 15:21:54; /db/ztree/library/trees/xdctargets/xdctargets-g31x/src/ xlibrary

 */

