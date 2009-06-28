-- eLua reference manual - ADC

data_en = 
{

  -- Title
  title = "eLua reference manual - ADC",

  -- Menu name
  menu_name = "ADC",

  -- Overview
  overview = [[This module contains functions that access analog to digital converter (ADC) peripherals. ]],

  -- Functions
  funcs = 
  {
    { sig = "#adc.sample#( id, count )",
      desc = "Request sample conversions.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$count$ - number of samples to acquire."
      }
    },
    { sig = "sample = #adc.getsample#( id )",
      desc = "Get converted samples.",
      args = 
      {
        "$id$ - ADC channel ID."
      },
      ret = "$sample$ - numeric value of conversion"
    },
    { sig = "samples = #adc.getsamples#( id, count )",
      desc = "Get multiple converted samples.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$count$ - optional parameter to indicate number of samples to return. If not included, all available samples are returned."
      },
      ret = "$samples$ - table containing integer conversion values"
    },
    { sig = "#adc.insertsamples#( id, table, idx, count )",
      desc = "Write multiple samples to a table.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$table$ - table to write samples to. Values at $idx$ to $idx$ + $count$ will be overwritten with samples.",
        "$idx$ - first index to use for writing samples",
        "$count$ - number of samples to return. If not enough samples are available (after blocking, if enabled) nil is written to indexes that would have received samples."
      }
    },
    { sig = "maxval = #adc.maxval#( id )",
      desc = "Get maximum conversion value.",
      args = 
      {
        "$id$ - ADC channel ID."
      },
      ret = "$maxval$ - maximum conversion value (based on channel resolution)"
    },
    { sig = "clock = #adc.setclock#( id, clock, timer_id )",
      desc = "Set frequency of sample acquisition.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$clock$ - frequency to acquire samples at in Hz (number of samples per second), 0 to acquire as fast as possible.",
        "$timer_id$ - Timer channel ID to use to control ADC conversion."
      },
      ret = "$clock$ - actual acquisition frequency to be used"
    },
    { sig = "status = #adc.isdone#( id )",
      desc = "Check if sampling is done.",
      args = 
      {
        "$id$ - ADC channel ID."
      },
      ret = "$status$ - 1 if no samples are being acquired, 0 if samples are pending acquisition."
    },
    { sig = "#adc.setblocking#( id, mode )",
      desc = "Set whether or not to block waiting for requested samples.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$mode$ - 1 if requests to get samples should block until requested samples are available or sampling has completed, 0 to return immediately with available samples"
      },
    },
    { sig = "#adc.setsmoothing#( id, length )",
      desc = "Set length of moving average filter.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$length$ - number of samples to include in moving average filter (must be a power of 2). If 1, filter is disabled."
      }
    }
  }
}

