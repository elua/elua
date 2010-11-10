// General purpose function/macros

#ifndef __UTILS_H__
#define __UTILS_H__

#define ABSDIFF( x, y )   ( ( x ) >= ( y ) ? ( x ) - ( y ) : ( y ) - ( x ) )
#define UMIN( x, y )      ( ( x ) <= ( y ) ? ( x ) : ( y ) )
#define UMAX( x, y )      ( ( x ) >= ( y ) ? ( x ) : ( y ) )
#define UABS( x )         ( ( x ) >= 0 ? ( x ) : -( x ) )

// Implement a very simple try-catch ike mechanism using setmp/longjmp,
// mostly to avoid goto's :)
#define EXC_DECLARE static jmp_buf exception_buf
#define EXC_TRY if( setjmp( exception_buf ) == 0 )
#define EXC_CATCH else
#define EXC_THROW() longjmp( exception_buf, 1 )

// Macro version of Duff's device found in 
// "A Reusable Duff Device" by Ralf Holly
// Dr Dobb's Journal, August 1, 2005

#define DUFF_DEVICE_8(count, action)  \
  do {                                \
    int _count = ( count );           \
    int _times = ( _count + 7 ) >> 3; \
    switch ( _count & 7 ){            \
        case 0: do { action;          \
        case 7:      action;          \
        case 6:      action;          \
        case 5:      action;          \
        case 4:      action;          \
        case 3:      action;          \
        case 2:      action;          \
        case 1:      action;          \
           } while (--_times > 0);    \
        }                             \
  } while (0)

#define STD_CTRLZ_CODE    26    

#endif
