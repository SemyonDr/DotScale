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
// -- Second number in the type name is the number of bottom bits dedicated to the fractional part.
// -- Upper bits represent integer part and a sign (for signed types).



//--------------------------------
// SIGNED 16 BIT
//--------------------------------

//--------------------------------
// UNSIGNED 16 BIT
//--------------------------------

/// <summary>
/// Unsigned.
/// NO integer part.
/// 16 bit fractional part.
/// </summary>
using ufxd16_16_t = uint16_t;



//--------------------------------
// SIGNED 32 BIT
//--------------------------------

using fxd32_16_t = int32_t;

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
/// 31 bit integer part.
/// 32 bit fractional part.
/// </summary>
using fxd64_32_t = int64_t;

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
/// 48 bit integer part.
/// 16 bit fractional part.
/// </summary>
using ufxd64_16_t = uint64_t;


/// <summary>
/// Unsigned.
/// 36 bit integer part.
/// 28 bit fractional part.
/// </summary>
using ufxd64_28_t = uint64_t;

/// <summary>
/// Unsigned.
/// 33 bit integer part.
/// 31 bit fractional part.
/// </summary>
using ufxd64_31_t = uint64_t;

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
/// 2 bit integer part. Max value is 3.
/// 62 bit fractional part.
/// </summary>
using ufxd64_62_t = uint64_t;

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





