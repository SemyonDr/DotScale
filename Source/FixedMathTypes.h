#pragma once
#include <cstdint>

//--------------------------------
// FIXED BINARY PRECISION
// DATA TYPES
//--------------------------------

// Each type represents fractional decimal number.
// 
// Numbers can be signed and unsigned
// -- First number in the type name is the width of underlying type.
// -- Second number in the type name is number of bottom bits dedicated to fractional part.
// -- Upper bits represent integer part and possible a sign.


//--------------------------------
// SIGNED 32 BIT
//--------------------------------



//--------------------------------
// UNSIGNED 32 BIT
//--------------------------------

/// <summary>
/// Unsigned.
/// 16 bit integer part.
/// 16 bit fractional part.
/// </summary>
using ufxd32_16_t = uint32_t;

/// <summary>
/// Unsigned.
/// NO integer part.
/// 32 bit fractional part.
/// </summary>
using ufxd32_32_t = uint32_t;



//--------------------------------
// SIGNED 64 BIT
//--------------------------------

/// <summary>
/// Signed.
/// 35 bit integer part.
/// 28 bit fractional part.
/// </summary>
using fxd64_28_t = int64_t;

/// <summary>
/// Signed.
/// 3 bit integer part. Max value is 7.
/// 60 bit fractional part.
/// </summary>
using fxd64_60_t = int64_t;



//--------------------------------
// UNSIGNED 64 BIT
//--------------------------------

/// <summary>
/// Unsigned.
/// 36 bit integer part.
/// 28 bit fractional part.
/// </summary>
using ufxd64_28_t = uint64_t;

/// <summary>
/// Unsigned.
/// 32 bit integer part.
/// 32 bit fractional part.
/// </summary>
using ufxd64_32_t = uint64_t;

/// <summary>
/// Unsigned.
/// 4 bit integer part. Max value is 15.
/// 60 bit fractional part.
/// </summary>
using ufxd64_60_t = uint64_t;

/// <summary>
/// Unsigned.
/// 1 bit integer part. Max value is 1.
/// 63 bit fractional part.
/// </summary>
using ufxd64_63_t = uint64_t;

/// <summary>
/// Unsigned.
/// NO integer part.
/// 64 bit fractional part.
/// </summary>
using ufxd64_64_t = uint64_t;





