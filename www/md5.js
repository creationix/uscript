// based on Alexander Peslyak's public domain MD5 C code
// http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
(function () {
  window.md5 = md5;

  // The MD5 transformation for all four rounds.
  function common(a, b, m, k, s, f) {
    a += f + m + k;
    return ((a << s) | (a >>> (32 - s))) + b;
  }

  // The basic MD5 functions.
  // F and G are optimized from to their RFC 1321 definitions to use XOR.
  function F(a, b, c, d, m, k, s) { return common(a, b, m, k, s, d ^ (b & (c ^ d))); }
  function G(a, b, c, d, m, k, s) { return common(a, b, m, k, s, c ^ (d & (b ^ c))); }
  function H(a, b, c, d, m, k, s) { return common(a, b, m, k, s, b ^ c ^ d); }
  function I(a, b, c, d, m, k, s) { return common(a, b, m, k, s, c ^ (b | ~d)); }

  function cycle(state, block) {
    var a = state[0],
        b = state[1],
        c = state[2],
        d = state[3];

      a = F(a, b, c, d, block[0],  0xd76aa478, 7);
      d = F(d, a, b, c, block[1],  0xe8c7b756, 12);
      c = F(c, d, a, b, block[2],  0x242070db, 17);
      b = F(b, c, d, a, block[3],  0xc1bdceee, 22);
      a = F(a, b, c, d, block[4],  0xf57c0faf, 7);
      d = F(d, a, b, c, block[5],  0x4787c62a, 12);
      c = F(c, d, a, b, block[6],  0xa8304613, 17);
      b = F(b, c, d, a, block[7],  0xfd469501, 22);
      a = F(a, b, c, d, block[8],  0x698098d8, 7);
      d = F(d, a, b, c, block[9],  0x8b44f7af, 12);
      c = F(c, d, a, b, block[10], 0xffff5bb1, 17);
      b = F(b, c, d, a, block[11], 0x895cd7be, 22);
      a = F(a, b, c, d, block[12], 0x6b901122, 7);
      d = F(d, a, b, c, block[13], 0xfd987193, 12);
      c = F(c, d, a, b, block[14], 0xa679438e, 17);
      b = F(b, c, d, a, block[15], 0x49b40821, 22);

      a = G(a, b, c, d, block[1],  0xf61e2562, 5);
      d = G(d, a, b, c, block[6],  0xc040b340, 9);
      c = G(c, d, a, b, block[11], 0x265e5a51, 14);
      b = G(b, c, d, a, block[0],  0xe9b6c7aa, 20);
      a = G(a, b, c, d, block[5],  0xd62f105d, 5);
      d = G(d, a, b, c, block[10], 0x02441453, 9);
      c = G(c, d, a, b, block[15], 0xd8a1e681, 14);
      b = G(b, c, d, a, block[4],  0xe7d3fbc8, 20);
      a = G(a, b, c, d, block[9],  0x21e1cde6, 5);
      d = G(d, a, b, c, block[14], 0xc33707d6, 9);
      c = G(c, d, a, b, block[3],  0xf4d50d87, 14);
      b = G(b, c, d, a, block[8],  0x455a14ed, 20);
      a = G(a, b, c, d, block[13], 0xa9e3e905, 5);
      d = G(d, a, b, c, block[2],  0xfcefa3f8, 9);
      c = G(c, d, a, b, block[7],  0x676f02d9, 14);
      b = G(b, c, d, a, block[12], 0x8d2a4c8a, 20);

      a = H(a, b, c, d, block[5],  0xfffa3942, 4);
      d = H(d, a, b, c, block[8],  0x8771f681, 11);
      c = H(c, d, a, b, block[11], 0x6d9d6122, 16);
      b = H(b, c, d, a, block[14], 0xfde5380c, 23);
      a = H(a, b, c, d, block[1],  0xa4beea44, 4);
      d = H(d, a, b, c, block[4],  0x4bdecfa9, 11);
      c = H(c, d, a, b, block[7],  0xf6bb4b60, 16);
      b = H(b, c, d, a, block[10], 0xbebfbc70, 23);
      a = H(a, b, c, d, block[13], 0x289b7ec6, 4);
      d = H(d, a, b, c, block[0],  0xeaa127fa, 11);
      c = H(c, d, a, b, block[3],  0xd4ef3085, 16);
      b = H(b, c, d, a, block[6],  0x04881d05, 23);
      a = H(a, b, c, d, block[9],  0xd9d4d039, 4);
      d = H(d, a, b, c, block[12], 0xe6db99e5, 11);
      c = H(c, d, a, b, block[15], 0x1fa27cf8, 16);
      b = H(b, c, d, a, block[2],  0xc4ac5665, 23);

      a = I(a, b, c, d, block[0],  0xf4292244, 6);
      d = I(d, a, b, c, block[7],  0x432aff97, 10);
      c = I(c, d, a, b, block[14], 0xab9423a7, 15);
      b = I(b, c, d, a, block[5],  0xfc93a039, 21);
      a = I(a, b, c, d, block[12], 0x655b59c3, 6);
      d = I(d, a, b, c, block[3],  0x8f0ccc92, 10);
      c = I(c, d, a, b, block[10], 0xffeff47d, 15);
      b = I(b, c, d, a, block[1],  0x85845dd1, 21);
      a = I(a, b, c, d, block[8],  0x6fa87e4f, 6);
      d = I(d, a, b, c, block[15], 0xfe2ce6e0, 10);
      c = I(c, d, a, b, block[6],  0xa3014314, 15);
      b = I(b, c, d, a, block[13], 0x4e0811a1, 21);
      a = I(a, b, c, d, block[4],  0xf7537e82, 6);
      d = I(d, a, b, c, block[11], 0xbd3af235, 10);
      c = I(c, d, a, b, block[2],  0x2ad7d2bb, 15);
      b = I(b, c, d, a, block[9],  0xeb86d391, 21);

      state[0] += a;
      state[1] += b;
      state[2] += c;
      state[3] += d;
  }

  var block = new Uint32Array(16);
  var bblock = new Uint8Array(block.buffer);
  var state = new Uint32Array(4);
  var bstate = new Uint8Array(state.buffer);

  // Process a one or more 512-bit chunks of data.
  // Expects an array-like value as input.
  function md5(data) {
    var dataLength = data.length;

    // Pad the input string.
    var length = dataLength + 9;
    length += 64 - (length % 64);

    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;

    for (var offset = 0; offset < length; offset += 64) {
      for (var i = 0; i < 64; i++) {
        var o = offset + i;
        if (o < dataLength) {
          bblock[i] = data[i];
          continue;
        }
        if (o === dataLength) {
          bblock[i] = 0x80;
          continue;
        }
        var x = o - length + 8;
        if (x >= 0 && x < 4) {
          bblock[i] = (dataLength << 3 >>> (x * 8)) & 0xff;
          continue;
        }
        bblock[i] = 0;
      }
      cycle(state, block);
    }

    return Array.prototype.map.call(bstate, function (num) {
      return "0123456789abcdef"[num >> 4] + "0123456789abcdef"[num & 15];
    }).join("");

  }


  // Convert a JS string into a UTF-8 encoded byte array.
  function stringToBuffer(string) {
    // Convert the unicode string to be the ASCII representation of
    // the UTF-8 bytes.
    string = unescape(encodeURIComponent(string));
    var length = string.length;
    var buf = new Uint8Array(length);
    for (var i = 0; i < length; i++) {
      buf[i] = string.charCodeAt(i);
    }
    return buf;
  }
})();
