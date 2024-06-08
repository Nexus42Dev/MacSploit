#include <iostream>
#include <fstream>
#include <unordered_map>
#include <filesystem>

#include "exploit.h"

namespace bitop {
    int bit_bdiv(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)//checks if they provided a number
        {
            (*rbx_error)("Expected number in bit.bdiv");//errors because there was no number specified if u dont include this roblox would crash
            return 0;
        }

        if (rbx_gettype(rL, -1) != LUA_TNUMBER)//checks if they provided a number
        {
            (*rbx_error)("Expected number in bit.bdiv");//errors because there was no number specified if u dont include this roblox would crash
            return 0;
        }

        int val = rbx_tointeger(rL, -2, 0);//gets the numbers the user specified
        int by = rbx_tointeger(rL, -1, 0);//gets the numbers the user specified
        int ret = val / by;//does the action(devides)

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_badd(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.badd");
            return 0;
        }

        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.badd");
            return 0;
        }

        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = val + by; //adds the values so if it was 10,5 it would be 15 because 10 + 5 = 15

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_bsub(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bsub");
            return 0;
        }

        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bsub");
            return 0;
        }

        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = val - by;//substracts

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_bmul(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bmul");
            return 0;
        }

        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bmul");
            return 0;
        }

        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = val * by;//multiplies

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_band(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.band");
            return 0;
        }

        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.band");
            return 0;
        }

        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = val & by;

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_bor(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bor");
            return 0;
        }

        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bor");
            return 0;
        }

        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = val | by;

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_bxor(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bxor");
            return 0;
        }

        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bxor");
            return 0;
        }

        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = val ^ by;//does bitwise xor

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }


    int bit_bnot(uint64_t rL)
    {
        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.bnot");
            return 0;
        }

        int val = rbx_tointeger(rL, -1, 0);
        int ret = ~val;//does bitwise not

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_bswap(uint64_t rL)
    {
        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.rol");
            return 0;
        }
        int by = rbx_tointeger(rL, -1, 0);
        int ret = by >> 24 | by >> 8 & 0xff00 | (by & 0xff00) << 8 | by << 24;//swaps value
        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int leftRotate(unsigned n, unsigned d) {
        return (n << d) | (n >> (32 - d));
    }
    int rightRotate(unsigned n, unsigned d) {
        return (n >> d) | (n << (32 - d));
    }
    int bit_rol(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.rol");
            return 0;
        }
        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.rol");
            return 0;
        }
        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = leftRotate(val, by); //rotates left
        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_ror(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.ror");
            return 0;
        }
        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.ror");
            return 0;
        }
        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = rightRotate(val, by); //rotates right
        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_lshift(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.lshift");
            return 0;
        }

        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.lshift");
            return 0;
        }

        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        int ret = val << by; //totates left

        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bit_rshift(uint64_t rL)
    {
        if (rbx_gettype(rL, -2) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.rshift");
            return 0;
        }
        if (rbx_gettype(rL, -1) != LUA_TNUMBER)
        {
            (*rbx_error)("Expected number in bit.rshift");
            return 0;
        }
        int val = rbx_tointeger(rL, -2, 0);
        int by = rbx_tointeger(rL, -1, 0);
        std::cout << "Value: 0x" << val << "\n";
        std::cout << "Byte: 0x" << by << "\n";
        int ret = val >> by; //rotates right
        (*rbx_pushnumber)(rL, ret);
        return 1;
    }

    int bittohex(uint64_t rL) //credits to http://bitop.luajit.org/ turns out it wasnt synapse lol
    {
        int b = rbx_tointeger(rL, 1, 0);
        int n = rbx_gettop(rL) == 1 ? 8 : rbx_tointeger(rL, 2, 0);
        const char* hexdigits = "0123456789abcdef";
        char buf[8];
        if (n < 0)
        {
            n = -n;
            hexdigits = "0123456789ABCDEF";
        }
        if (n > 8) n = 8;
        for (int i = (int)n; --i >= 0;)
        {
            buf[i] = hexdigits[b & 15];
            b >>= 4;
        }
        rbx_pushstring(rL, std::string(buf, n));
        return 1;
    }

    typedef int32_t SBits;
    typedef uint32_t UBits;
    #define BRET(b)  (*rbx_pushnumber)(rL, b); return 1;
    typedef union {
        double n;
    #ifdef LUA_NUMBER_DOUBLE
        uint64_t b;
    #else
        UBits b;
    #endif
    } BitNum;

    static UBits barg(uint64_t rL, int idx)
    {
        double x = rbx_tonumber(rL, idx);
        x += 6755399441055744.0;  /* 2^52+2^51 */
        return x;
    }
    static int bit_tobit(uint64_t rL) { BRET(barg(rL, 1)) }
}