/*
  ==============================================================================

  Copyright 2007-2013, 2017 William Andrew Burnson
  Copyright 2013-2016 Robert Taub

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  ==============================================================================
*/

//
//
//                               MICA Standard API
//
//

/*
Limitations:
  JavaScript numbers are 64-bit floating-point doubles. However due to the 53-bit
  mantissa a large range of integers are expressible. Due to this caveat, the
  following implementation differs from the API specification:
  
  * UUID integer(Integer64 x) will only accept signed or unsigned 32-bit numbers
    and it will return Undefined if given a number that is too large.
  
  * numerator() and denominator() will return zero if for the value x:
    x <= -9007199254740992 or x >= 9007199254740992
    (This number is the first number that is not able to differentiated from the
    next integer by magnitude)
*/

//Create the global mica namespace object
mica = {}

//
// Type Information
//

mica.identifier = function (concept) {
  concept = mica._sanitizeUuid(concept)
  return concept !== mica.Undefined &&
    mica._concept_length[concept] !== undefined
}
mica.number = function (concept) {
  concept = mica._sanitizeUuid(concept)
  return mica.denominator(concept) >= 1
}
mica.integer = function (conceptOrNumber) {
  if(typeof conceptOrNumber === "string") {
    return mica.denominator(mica._sanitizeUuid(conceptOrNumber)) == 1
  }
  else
    return mica._native(mica._toUuidInt(conceptOrNumber))
}
mica.undefined = function (concept) {
  concept = mica._sanitizeUuid(concept)
  return concept === mica.Undefined
}

//
// Numbers
//

mica.numerator = function (concept) {
  concept = mica._sanitizeUuid(concept)
  if(mica.denominator(concept) <= 0)
    return 0
  var u = mica._toUuid(concept)
  var x = mica._toInt([u[0], u[1]])
  if(x <= mica._minInt() || x >= mica._maxInt()) return 0
  return x
}

mica.denominator = function (concept) {
  concept = mica._sanitizeUuid(concept)
  var u = mica._toUuid(concept)
  if(u[2] >= 2147483648)
    return 0
  var x = mica._toInt([u[2], u[3]])
  if(x <= mica._minInt() || x >= mica._maxInt()) return 0
  return x
}

//
// Name
//

mica.language = "en"

mica.name = function (languageOrConcept, concept) {
  var language = languageOrConcept
  if(concept === undefined) {
    concept = languageOrConcept
    language = mica.language
  }
  concept = mica._sanitizeUuid(concept)
  
  if(mica.integer(concept)) {
    return mica.numerator(concept) //Returns with type number
  } else if(mica.number(concept)) {
    return ''+mica.numerator(concept)+'/'+mica.denominator(concept)
  }
  
  var x = mica._conceptLanguage_key[concept+'|'+language]
  if(x === undefined)
    x = mica._conceptLanguage_key[mica.Undefined+'|'+mica.language]
  return x.substr(x.indexOf(":")+1)
}

mica.named = function (name) {
  var x = mica._key_concept[''+name]
  if(x === undefined)
    x = mica._key_concept[mica.language+':'+name]
  if(x === undefined)
    x = mica.Undefined
  return mica._native(x)
}

//
// Sequences
//

mica.sequence = function (concept) {
  concept = mica._sanitizeUuid(concept)
  return mica._concept_length[concept] !== 0
}

mica.length = function (concept) {
  concept = mica._sanitizeUuid(concept)
  return Math.abs(mica._concept_length[concept])
}

mica.cyclic = function (concept) {
  concept = mica._sanitizeUuid(concept)
  return mica._concept_length[concept] < 0
}

//
// Indexing
//

mica.index = function (sequence, elementOrOrigin, element) {
  sequence = mica._sanitizeUuid(sequence)
  elementOrOrigin = mica._sanitizeUuid(elementOrOrigin)
  if(element === undefined) {
    var x = mica._sequenceConcept_index[sequence + '|' + elementOrOrigin]
    if(x === undefined)
      x = mica.Undefined
    return mica._native(x)
  }
  element = mica._sanitizeUuid(element)
  
  var i = mica.index(sequence, elementOrOrigin)
  if(mica.undefined(i))
    return mica._native(mica.Undefined)
  var j = mica._sequenceConcept_index[sequence + '|' + element]
  if(j === undefined)
    return mica._native(mica.Undefined)
  return j - i
}

//
// Items
//

mica.item = function (sequence, originOrIndex, index) {
  if(index !== undefined) {
    var x = mica.index(sequence, originOrIndex)
    if(x === undefined)
      return mica._native(mica.Undefined)
    return mica.item(sequence, x + mica.numerator(mica._sanitizeUuid(index)))
  }
  
  sequence = mica._sanitizeUuid(sequence)
  originOrIndex = mica.numerator(mica._sanitizeUuid(originOrIndex))
  var i = mica.numerator(originOrIndex)
  if(mica.cyclic(sequence)) {
    var n = mica.length(sequence)
    if(i >= 0)
      i = i % n
    else
      i = (n - ((-i) % n)) % n
  }
  var y = mica._sequenceIndex_concept[sequence+'|'+i]
  if(y === undefined) {
    y = mica.Undefined
  }
  return mica._native(y)
}

//
// Maps
//

mica.map = function () {
  var x = mica._map_concept[mica._getHashLookup(arguments)]
  if(x === undefined)
    x = mica.Undefined
  return mica._native(x)
}

//
//
//                              MICA Extended API
//
//

//
// Intervals
//

mica._ascending = function (x, y) {
  var d = mica.direction(x, y)
  if(mica.undefined(d)) return []
  if(d === mica._native(mica.Above)) return [x, y]
  else return [y, x]
}

mica.direction = function (x, y) {
  x = mica._sanitizeUuid(x)
  y = mica._sanitizeUuid(y)
  var i = mica.index(mica.Pitches, x, y)
  if(mica.undefined(i)) return mica._native(mica.Undefined)
  var n = mica.numerator(i)
  if(n === 0) return mica._native(mica.Unison)
  if(n < 0) return mica._native(mica.Below)
  if(n > 0) return mica._native(mica.Above)
}

mica.octaves = function(x, y) {
  var u = mica._ascending(x, y); x = u[0]; y = u[1]
  
  if(mica.undefined(x) || mica.undefined(y))
    return mica._native(mica.Undefined)
  
  if(mica.map(mica.DiatonicPitch, x) === mica.map(mica.DiatonicPitch, y))
    return 0
  
  var adjust = 0
  if(mica.index(mica.C, mica.map(x, mica.Letter), mica.map(y, mica.Letter)) <= 0)
      adjust = 1
  return mica.numerator(mica.map(y, mica.Octave)) -
    mica.numerator(mica.map(x, mica.Octave)) - adjust
}

mica.distance = function(x, y) {
  var u = mica._ascending(x, y); x = u[0]; y = u[1]
  
  var xl = mica.map(x, mica.Letter), yl = mica.map(y, mica.Letter)
  if(xl === yl && mica.map(x, mica.Octave) !== mica.map(y, mica.Octave))
    return mica._native(mica.Octave)
  return mica._native(mica.item(mica.Distances, mica.index(xl, yl)))
}

mica.quality = function(x, y) {
  var u = mica._ascending(x, y); x = u[0]; y = u[1]
  var d = mica.index(mica.map(x, mica.Letter), mica.map(y, mica.Letter))
  
  return mica.item(
    //Quality sequence...
    mica.item(mica.DistanceQualities, d),
    //Origin...
    mica.item(mica.map(mica.map(x, mica.Letter), mica.QualitiesByLetter), d),
    //Accidental delta...
    mica.index(mica.Accidentals, mica.map(mica.Accidental, x),
      mica.map(mica.Accidental, y))
  )
}

mica.interval = function(x, y) {
  return mica.map(mica.distance(x, y), mica.quality(x, y))
}

mica.transpose = function(pitch, interval, direction, octaves) {
  //Sanitize arguments
  pitch = mica._sanitizeUuid(pitch)
  interval = mica._sanitizeUuid(interval)
  direction = mica._sanitizeUuid(direction)
  if(octaves === undefined)
    octaves = 0
  debug = [direction, mica.Above, mica.Below]

  //Make sure direction is either above or below.
  if(direction !== mica.Above && direction !== mica.Below)
    return mica._native(mica.Undefined)
  debug = 1
  //Get the components...
  var l = mica.map(pitch, mica.Letter), a = mica.map(pitch, mica.Accidental),
    o = mica.map(pitch, mica.Octave), d = mica.map(interval, mica.Distance),
    q = mica.map(interval, mica.Quality)
  var l_result = mica.Undefined, a_result = mica.Undefined,
    o_result = mica.Undefined
  
  //Get the direction sign.
  var s = (direction === mica.Above) ? 1 : -1
  
  //Get the letter...
  var di = mica.index(mica.Distances, d)
  if(!mica.integer(di)) return mica._native(mica.Undefined)
  var n_di = mica.numerator(di)
  l_result = mica.item(l, s * n_di)
  debug = 2
  //Get the accidental...
  var a_delta = mica.index(mica.item(mica.DistanceQualities, di),
    mica.item(mica.map(((s === 1) ? l : l_result), mica.QualitiesByLetter),
    n_di), q)
  if(mica.undefined(a_delta)) return mica._native(mica.Undefined)
  a_result = mica.item(mica.Accidentals, a, s * mica.numerator(a_delta))
  debug = 3
  //Get the octave...
  if(!mica.integer(''+o) || !mica.integer(''+octaves) ||
    mica.numerator(octaves) < 0)
      return mica._native(mica.Undefined)
  o_result = mica.numerator(o) + s * (mica.numerator(octaves) +
    ((mica.numerator(mica.index(mica.C, l, l_result)) * s) <= 0 ? 1 : 0))
  debug = 4
  return mica.map(l_result, a_result, o_result)
}

//
// Tuning
//

mica.cents = function(justIntervalOrRatio) {
  var n = 0, d = 1
  if(typeof justIntervalOrRatio === "number") {
    n = justIntervalOrRatio
    if(n === 0)
      return 0
    else if(n < 0)
      return mica._native(mica.Undefined)
  }
  else {
    justIntervalOrRatio = mica._sanitizeUuid(justIntervalOrRatio)
    if(mica.number(justIntervalOrRatio)) {
      n = mica.numerator(justIntervalOrRatio)
      d = mica.denominator(justIntervalOrRatio)
      if(n === 0)
        return 0
      else if(n < 0)
        return mica._native(mica.Undefined)
    }
    else {
      var x = mica.map(justIntervalOrRatio, mica.Ratio)
      if(!mica.number(x))
        return mica._native(mica.Undefined)
      n = mica.numerator(x)
      d = mica.denominator(x)
    }
  }
  
  return Math.log(n / d) / Math.log(2) * 1200
}

//
//
//                               Internals below
//
//

// Variable names:
// i or j     = 32-character UUID strings
// u or v     = [a b c d] 128-bit number from 4 32-bit parts
// x or y     = [a b] 64-bit number from 2 32-bit parts
// a b c or d = 32 bit numbers

mica._native = function (x) {
  return mica.name(x)
}

mica._maxInt = function () {
  return 9007199254740992
}

mica._minInt = function () {
  return -9007199254740992
}

mica._maxIntDigits = function () {
  return 17
}

mica._toUuidInt = function (x) {
  if(mica._isInt(x)) {
    if(x >= 0)
      return mica._toString([0, x, 0, 1])
    else
      return mica._toString([0xffffffff, x + 4294967296, 0, 1])
  }
  else {
    return mica.Undefined
  }
}

mica._toInt = function (x) {
  if(x[0] < 2147483648) {
    return (x[0] * 4294967296) + x[1]
  }
  else {
    return x[1] - ((4294967296 - x[0]) * 4294967296)
  }
}

mica._isInt = function (a) {
  if(typeof a !== 'number') return false
  if(a < -2147483648 || a > 4294967295) return false
  if(a % 1 !== 0) return false
  return true
}

mica._sanitizeUuid = function (i) {
  var s = '' + i
  
  //Convert a definitional name to UUID directly if necessary.
  var x = mica._key_concept[mica.language+':'+s]
  if(x !== undefined) {
    return x
  }
  
  var d = s.indexOf('/')
  if(d != -1) {
    //Parsing ratios into UUIDs is unnecessary at the moment.
    var num = parseInt(s.substr(0, d), 10)
    var den = parseInt(s.substr(d + 1), 10)
    if(den === 0) {
      return mica.Undefined
    }
    if(den < 0) {
      num = -num
      den = -den
    }
    if(!mica._isInt(num) || !mica._isInt(den))
      return mica.Undefined
    if(num >= 0)
      return mica._toString([0, num, 0, den])
    else
      return mica._toString([0xffffffff, num + 4294967296, 0, den])
  }
  else if(s.length <= mica._maxIntDigits() &&
    (s.substr(0, 1) !== '0' || s.length == 1) &&
    mica._isInt(parseInt(s, 10)) && parseInt(s, 10) == s) {
      return mica._toUuidInt(parseInt(s, 10))
  }

  //Check UUID string to make sure it is valid.
  if(s.length != 32)
  return mica.Undefined
  var u = mica._toUuid(''+i)
  if(isNaN(u[0]) || isNaN(u[1]) || isNaN(u[2]) || isNaN(u[3]))
    return mica.Undefined
  return ''+i
}

mica._toUuid = function (x) {
  return [parseInt(x.substr(0, 8), 16), parseInt(x.substr(8, 8), 16),
    parseInt(x.substr(16, 8), 16), parseInt(x.substr(24, 8), 16)]
}

mica._pad32 = function (x) {
  while(x.length < 8) x = '0' + x
  return x
}

mica._toString = function (u) {
  return mica._pad32(u[0].toString(16)) + mica._pad32(u[1].toString(16)) +
         mica._pad32(u[2].toString(16)) + mica._pad32(u[3].toString(16))
}

mica._high16 = function (a) {return (a >>> 16) % 65536}

mica._low16 = function (a) {return a % 65536}

mica._hash32 = function (a, b) {
  var z = (36969 * mica._low16(a)) + mica._high16(a)
  var w = (18000 * mica._low16(b)) + mica._high16(b)
  var x = (((z % 65536) * 65536) + w)
  if(x >= 4294967296)
    x = x - 4294967296
  return x
}

mica._hash = function (i, j) {
  var u = mica._toUuid(i), v = mica._toUuid(j)
  var x = mica._toString([mica._hash32(u[0], v[0]), mica._hash32(u[1], v[1]),
    mica._hash32(u[2], v[2]), mica._hash32(u[3], v[3])])
  return x
}

mica._getHashLookup = function (x) {
  var canonicalized = []
  var i = 0
  while(x[i] !== undefined) {
    canonicalized[i] = mica._sanitizeUuid(x[i])
    i++
  }
  canonicalized.sort()
  if(canonicalized.length < 2)
    return mica.Undefined
  var h = canonicalized[0]
  for(var j = 1; j < canonicalized.length; j++) {
    h = mica._hash(h, canonicalized[j])
  }
  return h
}

//
//
//                            Data format examples
//
//

/*
mica.Dynamic='fff6dac5519d3adfbedcf16e6b807aaf'

mica._key_concept = {
'en:Dynamic':'fff6dac5519d3adfbedcf16e6b807aaf'
};

mica._concept_length = {
'fff6dac5519d3adfbedcf16e6b807aaf':0
}

mica._sequenceConcept_index = {
'fff6dac5519d3adfbedcf16e6b807aaf|fff6dac5519d3adfbedcf16e6b807aaf':0
}

mica._sequenceIndex_concept = {
'fff6dac5519d3adfbedcf16e6b807aaf|0':'fff6dac5519d3adfbedcf16e6b807aaf'
}

mica._conceptLanguage_key = {
'fff6dac5519d3adfbedcf16e6b807aaf|en':'en:Dynamic'
};

mica._map_concept = {
  'edcf16e6b807aaffff6dac5519d3adfb':'fff6dac5519d3adfbedcf16e6b807aaf'
}
*/
