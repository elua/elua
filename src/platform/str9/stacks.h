// Stack size definitions

#ifndef __STACKS_H__
#define __STACKS_H__

#define STACK_SIZE_USR   256
#define STACK_SIZE_SVC   32
#define STACK_SIZE_IRQ   32
#define STACK_SIZE_TOTAL ( STACK_SIZE_USR + STACK_SIZE_SVC + STACK_SIZE_IRQ )

#endif
