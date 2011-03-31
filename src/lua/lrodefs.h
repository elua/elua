/* Read-only tables helper */

#undef LUA_REG_TYPE
#undef LSTRKEY
#undef LNILKEY
#undef LNUMKEY
#undef LFUNCVAL
#undef LNUMVAL
#undef LROVAL
#undef LNILVAL
#undef LREGISTER
#undef LHEADER
#undef LHEADER_S
#undef LFOOTER
#undef LEXTERN

#if (MIN_OPT_LEVEL > 0) && (LUA_OPTIMIZE_MEMORY >= MIN_OPT_LEVEL)

#define LUA_REG_TYPE                luaR_table
#define LSTRKEY                     LRO_STRKEY
#define LNUMKEY                     LRO_NUMKEY
#define LNILKEY                     LRO_NILKEY
#define LFUNCVAL                    LRO_FUNCVAL
#define LNUMVAL                     LRO_NUMVAL
#define LROVAL                      LRO_ROVAL
#define LNILVAL                     LRO_NILVAL
#define LREGISTER(L, name, table)\
  return 0
#define LHEADER( name )\
  const luaR_table name = {\
    LRO_HEADER,\
    {
#define LHEADER_S( name )\
  static const luaR_table name = {\
    LRO_HEADER,\
    {
#define LFOOTER                     } };
#define LEXTERN( name )             extern const luaR_table name

#else

#define LUA_REG_TYPE                luaL_reg
#define LSTRKEY(x)                  x
#define LNILKEY                     NULL
#define LFUNCVAL(x)                 x
#define LNILVAL                     NULL
#define LREGISTER(L, name, table)\
  luaL_register(L, name, table);\
  return 1
#define LHEADER( name )             const luaL_reg name[] = {
#define LHEADER_S( name )           static const luaL_reg name[] = {A
#define LFOOTER                     };
#define LEXTERN( name )             extern const luaL_reg name[]
#endif

