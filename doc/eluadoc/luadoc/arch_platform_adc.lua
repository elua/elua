-- eLua platform interface - ADC
-- Make a full description for each language

data_en = 
{
  -- Menu name
  menu_name = "ADC",

  -- Title
  title = "eLua platform interface - ADC",

  -- Overview
  overview = "This part of the platform interface groups functions related to the ADC interface(s) of the MCU.",

  -- Functions
  funcs = 
  {
    { sig = "int #platform_adc_exists#( unsigned id );",
      desc = [[Checks if the platform has the hardware ADC specified as argument. Implemented in %src/common.c%, it uses the $NUM_ADC$ macro that must be defined in the
  platform's $platform_conf.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
  ~#define NUM_ADC   1      $// The platform has 1 ADC interface$~</p> ]],
      args = "$id$ - SPI interface ID",
      ret = "1 if the SPI interface exists, 0 otherwise"
    },
    
    { sig = "u32 #platform_adc_op#( unsigned id, int op, u32 data );",
      desc = "Executes an operation on an ADC channel",
      args = 
      {
        "$id$ - ADC channel ID",
        [[$op$ - the operation that must be executed. It can take any value from @#adc_operations@this enum@, as follows:
  <ul>
    <li>$PLATFORM_ADC_GET_MAXVAL$: get the maximum conversion value the channel may supply (based on channel resolution)</li>
    <li>$PLATFORM_ADC_SET_SMOOTHING$: sets the length of the moving average smoothing filter to $data$ </li>  
    <li>$PLATFORM_ADC_SET_BLOCKING$: sets whether or not sample requests should block, waiting for additional samples</li>
    <li>$PLATFORM_ADC_IS_DONE$: checks whether sampling has completed</li>
    <li>$PLATFORM_ADC_OP_SET_TIMER$: selects a timer to control sampling frequency</li>
    <li>$PLATFORM_ADC_OP_SET_CLOCK$: set the frequency of sample acquisition</li>
  </ul>]],
        "$data$ - when used with $op$ == $PLATFORM_ADC_SET_SMOOTHING$, specifies the length of the moving average filter (must be a power of 2). If it is 1, filter is disabled.",
        "$data$ - when used with $op$ == $PLATFORM_ADC_SET_BLOCKING$, specifies whether or not sample requests block.  If 1, requests will block until enough samples are available or sampling has ended. If 0, requests will return immediately with up to the number of samples requested.",
        "$data$ - when used with $op$ == $PLATFORM_ADC_OP_SET_TIMER$, specifies the timer to use to control sampling frequency.",
        "$data$ - when used with $op$ == $PLATFORM_ADC_OP_SET_CLOCK$, specifies the frequency of sample collection in Hz (number of samples per second). If 0, timer is not used and samples are acquired as quickly as possible."
      },
      ret = 
      {
        "the maximum possible conversion value when $op$ == $PLATFORM_ADC_GET_MAXVAL$",
        "whether or not sampling has completed (1: yes, 0: no) when $op$ == $PLATFORM_ADC_IS_DONE$. This will return 1 (yes), if no samples have been requested.",
        "the actual frequency of acquisition that will be used when $op$ == $PLATFORM_ADC_OP_SET_CLOCK$",
        "irellevant for other operations"
      }  
    },
    
    { sig = "int #platform_adc_check_timer_id#( unsigned id, unsigned timer_id );",
      desc = "Checks whether a timer may be used with a particular ADC channel",
      args = 
      {
        "$id$ - ADC channel ID",
        "$timer_id$ - Timer ID",
      },
      ret = "1 if the timer may be used to trigger the ADC channel, 0 if not",
    }
  }
}

