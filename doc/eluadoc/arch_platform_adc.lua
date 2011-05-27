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
  ~#define NUM_ADC   1      $// The platform has 1 ADC interface$~<p> ]],
      args = "$id$ - ADC interface ID",
      ret = "1 if the ADC interface exists, 0 otherwise"
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

data_pt = 
{
  -- Menu name
  menu_name = "ADC",

  -- Title
  title = "eLua API - ADC",

  -- Overview
  overview = "Interfaceamento com as funcões do ADC da MCU.",

  -- Functions
  funcs = 
  {
    { sig = "int #platform_adc_exists#( unsigned id );",
      desc = [[Checa se a plataforma possui hardware para o Conversor AD especificado no argumento. Implementado em %src/common.c%, utiliza a  macro $NUM_ADC$ que deve estar definida no arquivo $platform_conf.h$ da plataforma em questão. (veja @arch_overview.html#platforms@here@ para detalhes). Por exemplo:</p>
  ~#define NUM_ADC   1      $// A Plataforma possui uma interface para ADC$~<p> ]],
      args = "$id$ - ADC ID",
      ret = "1 se a interface do ADC existir, 0 em caso contrário"
    },
    
    { sig = "u32 #platform_adc_op#( unsigned id, int op, u32 data );",
      desc = "Executes an operation on an ADC channel",
      args = 
      {
        "$id$ - ID do canal ADC",
        [[$op$ - A operação que deve ser executada. Valores possíveis em  @#adc_operations@this enum@, conforme:
  <ul>
    <li>$PLATFORM_ADC_GET_MAXVAL$: Pega o maior valor de conversão que o canal pode fornecer, dependendo de sua resolução.</li>
    <li>$PLATFORM_ADC_SET_SMOOTHING$: Seta o tamanho do filtro atenuador de média móvel em $data$ </li>  
    <li>$PLATFORM_ADC_SET_BLOCKING$: Seta se a amostragem deve aguardar novos valores (blocking)</li>
    <li>$PLATFORM_ADC_IS_DONE$: Checa se a amostragem foi completada</li>
    <li>$PLATFORM_ADC_OP_SET_TIMER$: Seleciona um Timer para o controle da amostragem</li>
    <li>$PLATFORM_ADC_OP_SET_CLOCK$: Seta a frequência de amostragem</li>
  </ul>]],
        "$data$ - quando usada com $op$ == $PLATFORM_ADC_SET_SMOOTHING$, especifica o tamanho do filtro de média móvel (deve ser potência de 2). O valor 1 desabilita o filtro.",
        "$data$ - quando usada com $op$ == $PLATFORM_ADC_SET_BLOCKING$, especifica se a amostragem é blocking ou não.  Se 1, os pedidos só retornam quando as amostras tiverem sido obtidas ou a amostragem encerrada. Se 0, os pedidos retornam imediatamente, com o número de amostras já obtidas.",
        "$data$ - quando usada com $op$ == $PLATFORM_ADC_OP_SET_TIMER$, especifica o Timer a ser usado para o controle da frequência de amostragem.",
        "$data$ - quando usada com $op$ == $PLATFORM_ADC_OP_SET_CLOCK$, especifica a frequência de amostragem em Hz (número de amostras por segundo). Se 0, nenhum Timer é usado e a amostragem é feita o mais rapidamente possível."
      },
      ret = 
      {
        "O maior valor possível de ser obtido do conversor, quando $op$ == $PLATFORM_ADC_GET_MAXVAL$",
        "Flag informando se a conversão foi completada (1: sim, 0: não), quando $op$ == $PLATFORM_ADC_IS_DONE$. Também retorna 1 se nenhuma conversão foi solicitada.",
        "A frequência real de conversão que sera usada, quando $op$ == $PLATFORM_ADC_OP_SET_CLOCK$",
        "Indefinido para outras operações."
      }  
    },
    
    { sig = "int #platform_adc_check_timer_id#( unsigned id, unsigned timer_id );",
      desc = "Checa see um Timer pode ser usado com um ADC específico",
      args = 
      {
        "$id$ - ADC ID",
        "$timer_id$ - Timer ID",
      },
      ret = "1 se o Timer pode ser usado para trigar um ADC. 0 em caso contrário",
    }
  }
}

