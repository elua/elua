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
  platform's $cpu_xxx.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
  ~#define NUM_ADC   1      $// The platform has 1 ADC interface$~<p> ]],
      args = "$id$ - ADC interface ID",
      ret = "1 if the ADC interface exists, 0 otherwise"
    },
    
    { sig = "u32 #platform_adc_get_maxval#( unsigned id );",
      desc = "Gets the maximum conversion value the channel may supply (based on channel resolution)",
      args = "$id$ - ADC channel ID",
      ret = "the maximum possible conversion value"
    },

    { sig = "u32 #platform_adc_set_smoothing#( unsigned id, u32 length );",
      desc = "Sets the length of the moving average smoothing filter",
      args =
      {
        "$id$ - ADC channel ID",
        "$length$ - the length of the moving average filter (must be a power of 2). If it is 1, the filter is disabled.",
      },
      ret = "$PLATFORM_OK$ if the operation succeeded, $PLATFORM_ERR$ otherwise."
    },

    { sig = "void #platform_adc_set_blocking#( unsigned id, u32 mode );",
      desc = "Sets whether or not sample requests should block, waiting for additional samples",
      args =
      {
        "$id$ - ADC channel ID",
        "$mode$ - specifies whether or not sample requests block.  If 1, requests will block until enough samples are available or sampling has ended. If 0, requests will return immediately with up to the number of samples requested.",
      },
    },

    { sig = "u32 #platform_adc_is_done#( unsigned id );",
      desc = "Checks whether sampling has completed",
      args = "$id$ - ADC channel ID",
      ret = "1 if sampling has completed, 0 if not",
    },

    { sig = "void #platform_adc_set_timer#( unsigned id, u32 timer );",
      desc = "Selects a timer to control the sampling frequency",
      args =
      {
        "$id$ - ADC channel ID",
	"$timer$ - the ID of the timer to use to control the sampling frequency.",
      },
    },

    { sig = "u32 #platform_adc_set_clock#( unsigned id, u32 freq );",
      desc = "Set the frequency of sample acquisition",
      args =
      {
        "$id$ - ADC channel ID",
	"$freq$ - the frequency of sample collection in Hz (number of samples per second). If 0, the timer is not used and samples are acquired as quickly as possible.",
      },
      ret = "the actual sampling frequency that will be used, which might be different from the requested frequency, depending on the hardware"
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

data_pt = data_en
