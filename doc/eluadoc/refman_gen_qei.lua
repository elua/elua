-- eLua reference manual - QEI

data_en = 
{

  -- Title
  title = "eLua reference manual - QEI",

  -- Menu name
  menu_name = "qei",

  -- Overview
  overview = [[This module contains functions that access the Quadrature Encoder Interface (QEI).<\p>
  <p>Using the QEI is a three or four step process: First the peripheral must be initiailised (Using $qei.init$). The peripheral can be optionally initialised to measure velocity in a second step (Using $qei.velInit$). The peripheral must then be enabled ($qei.enable$) before lastly, results can be read (Using $qei.getPosition$, and if initialised, $qei.getVelPulses$, $qei.getRPM$). The QEI can also be disabled via $qei.disabled$<\p>
  <p>Encoders can be set alone or in combination in the initialisation functions, however must have data read in separate calls for each encoder.<\p>
  <p>This module can be utilized if the device in use has a supported QEI peripheral (see @status.html@status@ for details) and if QEI functionality is enabled at build time (see @building.html@building@).</p>
<p><span class="warning">IMPORTANT</span>: Platform support varies for this module (see @status.html#plat_notes@status notes@ for details) .
  ]],
  
    -- Structures
    structures =
    {
        { 
            text = [[// eLua qei error codes
            enum
            {
                ELUA_QEI_ERR_OK = 0,                // exported as $qei.ERR_OK$
                ELUA_QEI_ERR_VEL_NOT_ENABLED,  // exported as $qei.ERR_VELOCITY_NOT_ENABLED$
                ELUA_QEI_ERR_ENC_NOT_ENABLED,   // exported as $qei.ERR_ENCODER_NOT_ENABLED$
            };]],
            name = "Error codes",
            desc = "These are the error codes defined by the eLua_qei layer and are returned by a number of functions in this module.",
        }
    },
  
  -- Functions
  funcs = 
  {
    { sig = "#qei.init#( encoder_id, phase, swap, index, max_count )",
      desc = "Initiates the encoder channels for use.",
      args = 
      {
        "$encoder_id$ - QEI channel ID. Specified as qei.CH0, qei.CH1, qei.CHO1. Channels 0 or 1 can be initialised together or separately by calling this function once on each encoder.",
        "$phase$ - Each encoder has two phases: A and B. Either qei.PHA or qei.PHAB can be specified, effecting resolution. NOTE: phase B cannot be set on its own.",
        "$swap$ - Specified as either qei.SWAP or qei.NO_SWAP, this defines whether the phases are switched before processing. This changes the direction of the encoders via software.",
        "$index$ - Specified as either qei.INDEX or qei.NO_INDEX, this defines whether an index pulse is used or not.",
        "$max_count$ - Sets the maximum count of the encoder before it resets to zero.",
      }
    },
    { sig = "#qei.velInit#( encoder_id, vel_period, ppr, edges )",
      desc = "Optionally initialises the encoder channels for use in velocity measurement.",
      args = 
      {
        "$encoder_id$ - QEI channel ID. Specified as qei.CH0, qei.CH1, qei.CHO1. Channels 0 or 1 can be initialised together or separately by calling this function once on each encoder.",
        "$vel_period$ - Sets the period in micro seconds over which time the velocity is calculated.",
        "$ppr$ - Pulses per revolution of the encoder.",
        "$edges$ - The number of edges the encoder is triggering on.",
      }
    },
    { sig = "#qei.enable#( encoder_id )",
      desc = "Enables the encoder, post initialisation.",
      args = 
      {
        "$encoder_id$ - QEI channel ID. Specified as qei.CH0, qei.CH1, qei.CHO1. Channels 0 or 1 can be enabled separately by calling this function once on each encoder.",
      }
    },
    { sig = "#qei.disable#( encoder_id )",
      desc = "Disables the encoder.",
      args = 
      {
        "$encoder_id$ - QEI channel ID. Specified as qei.CH0, qei.CH1, qei.CHO1. Channels 0 or 1 can be disabled separately by calling this function once on each encoder.",
      }
    },
    { sig = "#qei.getPosition#( encoder_id )",
      desc = "Returns the current count of the encoder, ranging between 0 and maximum value set during initialisation.",
      args = 
      {
        "$encoder_id$ - QEI channel ID. Ethier qei.CH0 or qei.CH1 can be called. NOTE: qei.CH01 cannot be called. Must call each encoder separately.",
      }
      ret = 
      {
        "$count$ - the current encoder count between 0 and max_count set during initialisation.",
        "$err$ - Error code as defined above."
      }
    },
    { sig = "#qei.getVelPulses#( encoder_id )",
      desc = "Returns the number of pulses counted during the vel_period set during initialisation of velocity measurement. This value can be used raw, but is also used to calculate the RPM. ",
      args = 
      {
        "$encoder_id$ - QEI channel ID. Ethier qei.CH0 or qei.CH1 can be called. NOTE: qei.CH01 cannot be called. Must call each encoder separately.",
      }
      ret = 
      {
        "$pulses$ - The number of pulses recorded during the period defined during initialisation.",
        "$err$ - Error code as defined above."
      }
    },
    { sig = "#qei.getRPM#( encoder_id )",
      desc = "Returns the current number of revolutions per minute (RPM) of the selected encoder.",
      args = 
      {
        "$encoder_id$ - QEI channel ID. Ethier qei.CH0 or qei.CH1 can be called. NOTE: qei.CH01 cannot be called. Must call each encoder separately.",
      }
      ret = 
      {
        "$rpm$ - The current rpm.",
        "$err$ - Error code as defined above."
      }
    }
  }
}

data_pt = data_en

