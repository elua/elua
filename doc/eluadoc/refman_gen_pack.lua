-- eLua reference manual - pack

data_en = 
{

  -- Title
  title = "eLua reference manual - pack",

  -- Menu name
  menu_name = "pack",

  -- Overview
  overview = [[This module allows for arbitrary packing of data into Lua strings and unpacking data from Lua strings. In this way, a string can be used to store data in a platform-indepdendent 
manner. It is based on the ^http://www.tecgraf.puc-rio.br/~~lhf/ftp/lua/#lpack^lpack^ module from Luiz Henrique de Figueiredo (with some minor tweaks). </p>
<p>Both methods of this module (@#pack@pack@ and @#unpack@unpack@) use a $format string$ to describe how to pack/unpack the data. The format string contains one or more $data specifiers$, each
data specifier is applied to a single variable that must be packed/unpacked. The data specifier has the following general format:</p>
~[endianness]<<format specifier>>[count]~
<p>where:</p>
<ul>
  <li>$endianness$ is an optional endian flags that specifies how the numbers that are to be packed/unpacked are stored in memory. It can be either:
  <ol>
    <li>$'<<'$ for little endian.</li>
    <li>$'>>'$ for big endian.</li>
    <li>$'='$ for native endian (the platform's endian order, default).</li>
  </ol></li>
  <li>$format specifier$ describes what kind of variable will be packed/unpacked. $The format specifier is case-sensitive$. The possible values of this parameter are summarized in the table below:
  <p/>
  <table class="table_center" style="margin-top: 4px; margin-bottom: 4px;">
  <tbody>
  <tr>
    <th>Format specifier</th>
    <th>Corresponding variable type</th>
  </tr>
  <tr>
    <td>'z'</td>
    <td>zero-terminated string</td>
  </tr>  
  <tr>
    <td>'p'   </td>
    <td>string preceded by length byte</td>
  </tr>  
  <tr>
    <td>'P'   </td>
    <td>string preceded by length word</td>
  </tr>  
  <tr>
    <td>'a'   </td>
    <td>string preceded by length size_t</td>
  </tr>  
  <tr>
    <td>'A'   </td>
    <td>string</td>
  </tr>  
  <tr>
    <td>'f'   </td>
    <td>float</td>
  </tr>  
  <tr>
    <td>'d'   </td>
    <td>double</td>
  </tr>  
  <tr>
    <td>'n'   </td>
    <td>Lua number</td>
  </tr>  
  <tr>
    <td>'c'   </td>
    <td>char</td>
  </tr>  
  <tr>
    <td>'b'   </td>
    <td>byte = unsigned char</td>
  </tr>  
  <tr>
    <td>'h'   </td>
    <td>short</td>
  </tr>  
  <tr>
    <td>'H'   </td>
    <td>unsigned short</td>
  </tr>  
  <tr>
    <td>'i'   </td>
    <td>int</td>
  </tr>  
  <tr>
    <td>'I'   </td>
    <td>unsigned int</td>
  </tr>  
  <tr>
    <td>'l'   </td>
    <td>long</td>
  </tr>  
  <tr>
    <td>'L'   </td>
    <td>unsigned long</td>
  </tr>  
  </tbody>
  </table></li>
  <li>$count$ is an optional counter for the $format specifier$. For example, $i5$ instructs the code to pack/unpack 5 integer variables, as opposed to $i$ that specifies a
  single integer variable.</li>
</ul><p>]],

  -- Functions
  funcs = 
  {
    { sig = "packed = #pack.pack#( format, val1, val2, ..., valn )",
      desc = "Packs variables in a string.",
      args = 
      {
        "$format$ - format specifier (as described @#overview@here@).",
        "$val1$ - first variable to pack.",
        "$val2$ - second variable to pack.",
        "$valn$ - nth variable to pack.",
      },
      ret = "$packed$ - a string containing the packed representation of all variables according to the format."
    },

    { sig = "nextpos, val1, val2, ..., valn = #pack.unpack#( string, format, [ init ] )",
      desc = "Unpacks a string",
      args = 
      {
        "$string$ - the string to unpack.",
        "$format$ - format specifier (as described @#overview@here@).",
        "$init$ - $(optional)$ marks where in $string$ the unpacking should start (1 if not specified)."
      },
      ret = 
      {
        "$nextpos$ - the position in the string after unpacking.",
        "$val1$ - the first unpacked value.",
        "$val2$ - the second unpacked value.",
        "$valn$ - the nth unpacked value."
      }
    }
  },
}

data_pt = data_en
