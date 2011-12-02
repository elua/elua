-- eLua reference manual - ADC

data_en = 
{

  -- Title
  title = "eLua reference manual - ADC",

  -- Menu name
  menu_name = "adc",

  -- Overview
  overview = [[This module contains functions that access analog to digital converter (ADC) peripherals.</p>
  <p>When utilizing this module, acquiring ADC data is a two step process: requesting sample conversions (using $adc.sample$) and extraction of conversion results from a conversion buffer (using $adc.getsample$, $adc.getsamples$ or $adc.insertsamples$). Various configuration parameters are available to set conversion rate, how results are extracted from the buffer and how these results are processed prior to extraction.</p>
  <p>This module can be utilized if the device in use has a supported ADC peripheral (see @status.html@status@ for details) and if ADC functionality is enabled at build time (see @building.html@building@).</p>
<p><span class="warning">IMPORTANT</span>: Platform support varies for this module (see @status.html#plat_notes@status notes@ for details) .
  ]],

  -- Functions
  funcs = 
  {
    { sig = "#adc.sample#( id, count )",
      desc = "Initiate conversion and buffering of samples on an ADC channel.",
      args = 
      {
        "$id$ - ADC channel ID. Optionally, this may be a table containing a list of channel IDs (i.e.: {0, 2, 3}), allowing synchronization of acquisition. NOTE: This acceptance of mixed types is only for the sample function.",
        "$count$ - number of samples to acquire and place in buffer."
      }
    },
    { sig = "sample = #adc.getsample#( id )",
      desc = "Get a single conversion value from the buffer associated with a given channel.",
      args = 
      {
        "$id$ - ADC channel ID."
      },
      ret = "$sample$ - numeric value of conversion, or nil if sample was not available."
    },
    { sig = "samples = #adc.getsamples#( id, count )",
      desc = "Get multiple conversion values from the buffer associated with a given channel.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$count$ - optional parameter to indicate number of samples to return. If not included, all available samples are returned."
      },
      ret = "$samples$ - table containing integer conversion values. If not enough samples are available, remaining indices will be nil."
    },
    { sig = "#adc.insertsamples#( id, table, idx, count )",
      desc = "Get multiple conversion values from a channel's buffer, and write them into a table.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$table$ - table to write samples to. Values at $table$[$idx$] to $table$[$idx$ + $count$ -1] will be overwritten with samples (or nil if not enough samples are available).",
        "$idx$ - first index to use in the table for writing samples.",
        "$count$ - number of samples to return. If not enough samples are available (after blocking, if enabled) remaining values will be nil."
      }
    },
    { sig = "maxval = #adc.maxval#( id )",
      desc = "Get the maximum value (corresponding to the maximum voltage) that can be returned on a given channel.",
      args = 
      {
        "$id$ - ADC channel ID."
      },
      ret = "$maxval$ - maximum integer conversion value (based on channel resolution)"
    },
    { sig = "clock = #adc.setclock#( id, clock, timer_id )",
      desc = "Set the frequency (number of samples per second) at which voltages will be converted into samples.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$clock$ - frequency to acquire samples at in Hz (number of samples per second), 0 to acquire as fast as possible.",
        "$timer_id$ - Timer channel ID to use to control ADC conversion. <strong>Note:</strong> At this time, a timer selection will apply to all channels on a given ADC peripheral."
      },
      ret = "$clock$ - actual acquisition frequency that was set"
    },
    { sig = "status = #adc.isdone#( id )",
      desc = "Check whether samples are still being acquired on a channel.",
      args = 
      {
        "$id$ - ADC channel ID."
      },
      ret = "$status$ - 1 if no samples are being acquired, 0 if samples are pending acquisition."
    },
    { sig = "#adc.setblocking#( id, mode )",
      desc = "Set whether or not functions that request converted samples should wait for requested samples or return immediately with what is available. If this function is not called, each channel starts in blocking mode.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$mode$ - 1 if requests to get samples should block until requested samples are available or sampling has completed, 0 to return immediately with available samples"
      },
    },
    { sig = "#adc.setsmoothing#( id, length )",
      desc = "Set the length of the moving average filter. When $length$ is greater than 1, samples pulled from the conversion buffer will be averaged with the preceding $length$ - 1 buffered values.",
      args = 
      {
        "$id$ - ADC channel ID.",
        "$length$ - number of preceding samples to include in moving average filter (must be a power of 2). If 1, filter is disabled. When enabled, a filter buffer is filled before the main conversion buffer, so that averages are always over the same number of samples."
      }
    }
  }
}

data_pt = data_en
