/*
 * Copyright (C) 2017 eSi-RISC.
 *
 * Compiler specific definitions.
 *
 */

/** @file 
 * Compiler specific definitions.
 */

#ifndef _ESI_COMPILER_H
#define _ESI_COMPILER_H

#ifdef __GNUC__

/** GCC attribute to force N byte alignment for the associated declaration.  */
#define ESI_ALIGN(N)            __attribute__ ((aligned (N)))                           

/** Force 2 byte alignment for the associated declaration.  */
#define ESI_ALIGN_2             ESI_ALIGN(2) 
 
/** Force 4 byte alignment for the associated declaration.  */
#define ESI_ALIGN_4             ESI_ALIGN(4)  

/** GCC attribute to prevent inlining of the the associated function.  */
#define ESI_NOINLINE            __attribute__((noinline))

/** GCC attribute to make the the associated symbol weak. */
#define ESI_WEAK                __attribute__ ((weak))

/** GCC attribute to use section name N.
 * 
 * For example, to assign the variable foo to the section .mysection
 * 
 * ESI_SECTION(".mysection") int foo;
 *
 * This can be used for data and functions.
 */
#define ESI_SECTION(name)       __attribute__ ((section (name)))

/** GCC Attribute to indicate a function is an exception handler.
 *  
 * To declare a C function as an exception handler, use this macro as follows:
 *  
 *     ESI_EXCEPTION_HANDLER void handler(void);
 * 
 * This changes the compiled prologue and epilogue for the function such that 
 * it saves caller save registers as well as callee save registers. The function
 * will also use the eret instruction, rather than the standard ret instruction. Therefore,
 * functions declared with this attribute should not be called from other functions.          
 */
#define ESI_EXCEPTION_HANDLER   __attribute__((interrupt))

/** GCC Attribute to indicate a function is an exception handler that should save vector registers.
 *  
 * To declare a C function as an exception handler, use this macro as follows:
 *  
 *     ESI_VECTOR_EXCEPTION_HANDLER void handler(void);
 * 
 * This changes the compiled prologue and epilogue for the function such that 
 * it saves all vector registers, as well as caller save registers and callee save registers. 
 * The function  will also use the eret instruction, rather than the standard ret instruction. 
 * Therefore, functions declared with this attribute should not be called from other functions.          
 */
#define ESI_VECTOR_EXCEPTION_HANDLER   __attribute__((interrupt("vector")))

/** GCC Attribute to indicate a function is an s/w nested interrupt handler.
 *  
 * To declare a C function as a nested interrupt handler, use this macro as follows:
 *  
 *     ESI_NESTED_INTERRUPT_HANDLER void handler(void);
 * 
 * This changes the compiled prologue and epilogue for the function such that 
 * it saves the exception CSRs, caller save registers as well as callee save registers, 
 * before reenabling interrupts.
 * The function will also use the eret instruction, rather than the standard ret instruction.
 * Therefore, functions declared with this attribute should not be called from other functions.          
 * 
 * This macro is only supported if INTERRUPT_PRIORITIES > 0. 
 * If NESTED_INTERRUPTS_ENABLED is TRUE, this macro does not enable s/w nesting as well.
 */
#ifdef __nested_interrupts_enabled__
#define ESI_NESTED_INTERRUPT_HANDLER   __attribute__((interrupt))
#elif __interrupt_priorities__ > 0
#define ESI_NESTED_INTERRUPT_HANDLER   __attribute__((interrupt("nest")))
#endif

/** GCC Attribute to indicate a function is an nested vector interrupt handler.
 *  
 * To declare a C function as a nested vector interrupt handler, use this macro as follows:
 *  
 *     ESI_NESTED_VECTOR_INTERRUPT_HANDLER void handler(void);
 * 
 * This changes the compiled prologue and epilogue for the function such that 
 * it saves the exception CSRs, vector registers, caller save registers as well 
 * as callee save registers, before reenabling interrupts.
 * The function will also use the eret instruction, rather than the standard ret instruction.
 * Therefore, functions declared with this attribute should not be called from other functions.
 * 
 * This macro is only supported if INTERRUPT_PRIORITIES > 0.
 * If NESTED_INTERRUPTS_ENABLED is TRUE, this macro does not enable s/w nesting as well.
 */
#ifdef __nested_interrupts_enabled__
#define ESI_NESTED_VECTOR_INTERRUPT_HANDLER   __attribute__((interrupt("vector")))
#elif __interrupt_priorities__ > 0
#define ESI_NESTED_VECTOR_INTERRUPT_HANDLER   __attribute__((interrupt("vector", "nest")))
#endif

#else

#error "Unsupported compiler."

#endif

#endif /* _ESI_COMPILER_H */
