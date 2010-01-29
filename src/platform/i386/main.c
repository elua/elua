// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
//           Made for JamesM's tutorials <www.jamesmolloy.co.uk>

#include "monitor.h"
#include "descriptor_tables.h"
#include "timer.h"

extern void keyboard_install();
void monitor_write_hex(u32int n);

int main(struct multiboot_info *mboot_ptr)
{
    unsigned long lastmem = 0, temp;
    
    // Initialise all the ISRs and segmentation
    init_descriptor_tables();
    
    // Initialise the screen (by clearing it)
    monitor_clear();    
    
    // Look in the GRUB memory map for the space starting at 1M and locate
    // the stack at the end of this space
    memory_map_t* mmap = ( memory_map_t* )mboot_ptr->mmap_addr;
    while( (unsigned long)mmap < mboot_ptr->mmap_addr + mboot_ptr->mmap_length) {
      if(mmap->base_addr_low == 0x00100000)
      {
        if(mmap->length_high)
          lastmem = 0x00100000 + 0xFFFFFFF0;
        else
          lastmem = 0x00100000 + ( ( mmap->length_low >> 4 ) << 4 );
        break;
      }
      mmap = (memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
    }    
    if( lastmem == 0 )
    {
      monitor_write( "Unable to set stack, system halted!\n" );
      while( 1 );
    }
    
    // Set the stack now
    // NOTE that after this we won't be able to return to the code that called us (boot.s),
    // so don't ever return from main!
    asm volatile("mov %0, %%esp" : : "r" (lastmem));
    
    // Write out a sample string
    monitor_write("Hello, world!\n");
    
    // Print our stack pointer
    asm volatile("mov %%esp, %0" : "=r" (temp));
    monitor_write( "Stack pointer is at 0x" );
    monitor_write_hex( temp );
    monitor_put( '\n' );
    
    asm volatile("int $0x3");
    asm volatile("int $0x4");

    asm volatile("sti");
    //init_timer(50);
    keyboard_install();
   
    while( 1 );
}
