/*
 ============================================================================
 Name        : sMPI.h
 Author      : huangbin
 Version     :
 Copyright   : Your copyright notice
 Description : sMPI Headfile
 ============================================================================
 */
#ifndef NUMERICAL_TOOL_H
#define NUMERICAL_TOOL_H

#include <stdio.h>
#include <stdlib.h>

struct float_info{
    unsigned int sign;
    unsigned int exponent;
    unsigned int mantissa;
};

// 120.5    -> 0_10000101_11100010000000000000000 -> 1111000.1=1.1110001*2^6 
// 10.125   -> 0_10000010_01000100000000000000000 -> 1010.001=1.010001*2^3 
// 8.25     -> 0_10000010_00001000000000000000000 -> 1.00001*2^3 
// 130.625  -> 0_10000110_00000101010000000000000
//                      1000001010100000000000000 => 右移一位
// 1.875    -> 0_01111111_11100000000000000000000
//                        00111100000000000000000 => 左移三位
// -1.875   -> 1_01111111_11100000000000000000000
//                        00111100000000000000000 => 左移三位
// 83.53125 -> 0_10000101_01001110001000000000000 
                          101001110001000000000000
struct float_info float_extract(float f){
    int *p = (int*)&f;
    static struct float_info fi;
    
    unsigned int i = *p;
    fi.sign = (i>>31) & 0x00000001;       // 符号位
    fi.exponent = (i >> 23) & 0x000000FF; // 指数位
    fi.mantissa = i & 0x007FFFFF;         // 尾数位
    // printf("sign: %d, exponent: %d, mantissa: %d\n", sign, exponent, mantissa);
    return fi;
}

float float_combine(struct float_info *fi){
    static float f;
    int *p = (int*)&f;
    *p = (fi->sign << 31) | (fi->exponent << 23) | fi->mantissa;
    return f;
}

float float_add(float a, float b){
    if (a == 0.0) return b;
    if (b == 0.0) return a;

    struct float_info ai = float_extract(a);
    struct float_info bi = float_extract(b);
    struct float_info fi;

    // step1：对齐阶码
    int delta_E = ai.exponent - bi.exponent;
    ai.mantissa = ai.mantissa | 0x00800000;
    bi.mantissa = bi.mantissa | 0x00800000;
    if (delta_E > 0){
        bi.mantissa = (bi.mantissa >> delta_E);
        fi.exponent = ai.exponent;
    }else if (delta_E < 0){
        ai.mantissa = (ai.mantissa >> -delta_E);
        fi.exponent = bi.exponent;
    }else{
        fi.exponent = ai.exponent;
    }

    // step2：尾数相加
    if (ai.sign == bi.sign){
        fi.mantissa = ai.mantissa + bi.mantissa;
        fi.sign = ai.sign;
    }else{
        if (ai.mantissa > bi.mantissa){
            fi.mantissa = ai.mantissa - bi.mantissa;
            fi.sign = ai.sign;
        }else{
            fi.mantissa = bi.mantissa - ai.mantissa;
            fi.sign = bi.sign;
        }
    }
    
    // step3：对阶规格化
    if (fi.mantissa < 0x00800000){
        while (fi.mantissa < 0x00800000){
            fi.mantissa = fi.mantissa << 1;
            fi.exponent = fi.exponent - 1;
        }
    } else {
        while (fi.mantissa >= 0x00800000){
            fi.mantissa = fi.mantissa >> 1;
            fi.exponent = fi.exponent + 1;
        }
        fi.mantissa = fi.mantissa << 1;
        fi.exponent = fi.exponent - 1;
    }
    fi.mantissa = fi.mantissa & 0x007FFFFF;

    // step4：结果规格化
    return float_combine(&fi);
}

// 0.0125   -> 0_01111000_10011001100110011001101 
// 0.3      -> 0_01111101_00110011001100110011010 
// 0.00375  -> 0_01110110_11101011100001010001111
                         111101011100001010010000
float float_mul(float a, float b){
    if (a == 0.0 | b == 0.0)  return 0.0;

    struct float_info ai = float_extract(a);
    struct float_info bi = float_extract(b);
    struct float_info fi;

    // step1: 尾数相乘
    unsigned long long temp = (ai.mantissa | 0x00800000);
	temp = (temp * (bi.mantissa | 0x00800000)) >> 23;
    fi.mantissa = (unsigned int)temp;

    // step2: 阶码相加
    fi.exponent = ai.exponent + bi.exponent - 128;

    // step3: 规格化
    fi.exponent = fi.exponent + (fi.mantissa >> 23);
    fi.mantissa = fi.mantissa & 0x007FFFFF;

    // step4: 结果规格化
    return float_combine(&fi);
}

float float_div(float a, float b){
    struct float_info ai = float_extract(a);
    struct float_info bi = float_extract(b);
    struct float_info fi;

    return float_combine(&fi);
}


#endif












