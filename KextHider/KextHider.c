//
//  KextHider.c
//  KextHider
//
//  Created by Folker Schwesinger on 22.05.13.
//  Copyright (c) 2013 rc0r. All rights reserved.
//

#include <mach/mach_types.h>

kern_return_t KextHider_start(kmod_info_t * ki, void *d);
kern_return_t KextHider_stop(kmod_info_t *ki, void *d);

kern_return_t KextHider_start(kmod_info_t * ki, void *d)
{
    return KERN_SUCCESS;
}

kern_return_t KextHider_stop(kmod_info_t *ki, void *d)
{
    return KERN_SUCCESS;
}
