//  Quadrature Encoding Interface (QEI)

#include "elua_qei.h"
#include "platform.h"

#ifdef BUILD_QEI
static u32 vel_modifier = 0;

void elua_qei_init( u8 enc_id, u8 phase, u8 swap, u8 index, u32 max_count )
{
    platform_qei_init( enc_id, phase, swap, index, max_count );
}

void elua_qei_vel_init( u8 enc_id, u32 vel_period, int ppr, int edges )
{
    platform_qei_vel_init( enc_id, vel_period );
    qei_flag |= ( enc_id << VEL_FLAG_OFFSET );  //Sets encoder velocity flag
    u32 clk_freq = platform_qei_get_sys_clk();
    vel_modifier = (clk_freq * 60) / (vel_ticks * ppr * edges);
}
void elua_qei_enable( u8 enc_id )
{
    platform_qei_enable( enc_id );
    qei_flag |= enc_id;
}

void elua_qei_disable( u8 enc_id )
{
    platform_qei_disable( enc_id );
    qei_flag &= ~enc_id;
}

u32 elua_qei_getPulses( u8 enc_id )
{
    u32 pulses = platform_qei_getPulses( enc_id );
    return pulses;
}

s32 elua_qei_getRPM( u8 enc_id )
{
    u32 pulses = elua_qei_getPulses( enc_id );
    s32 rpm = pulses * vel_modifier * platform_qei_getDirection( enc_id );
    return rpm;
}

u32 elua_qei_getPosition( u8 enc_id )
{
    return platform_qei_getPosition( enc_id );
}

#endif
