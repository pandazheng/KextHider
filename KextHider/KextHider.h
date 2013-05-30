//
//  KextHider.h
//   by @_rc0r
//
#ifndef KextHider_KextHider_h
#define KextHider_KextHider_h

#include <mach-o/loader.h>
#include <mach/mach_types.h>
#include <sys/fcntl.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/vnode.h>
//#include <libkern/c++/OSArray.h>
// make sure to add ~/xnu-2050.22.13/libkern/ to include path,
// otherwise we'll get complaints about missing .h files...
//#include <libkern/c++/OSKext.h>

#ifdef DEBUG
#define DLOG(args...) printf(args)
#elif
#define DLOG(args...) /* */
#endif

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

struct nlist_64 {
    union {
        uint32_t  n_strx;   /* index into the string table */
    } n_un;
    uint8_t n_type;         /* type flag, see below */
    uint8_t n_sect;         /* section number or NO_SECT */
    uint16_t n_desc;        /* see <mach-o/stab.h> */
    uint64_t n_value;       /* value of this symbol (or stab offset) */
};

uint64_t KERNEL_MH_START_ADDR;

//mach_vm_address_t *sLoadedKexts;

kern_return_t KextHider_start(kmod_info_t * ki, void *d);
kern_return_t KextHider_stop(kmod_info_t *ki, void *d);
struct segment_command_64 *find_segment_64(struct mach_header_64 *mh, const char *segname);
struct section_64 *find_section_64(struct segment_command_64 *seg, const char *name);
struct load_command *find_load_command(struct mach_header_64 *mh, uint32_t cmd);
void *find_symbol(struct mach_header_64 *mh, const char *name);
uint64_t find_kernel_baseaddr( void );

#endif
