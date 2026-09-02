      // ============================================================
      // RPGVERIFY -- empirical probes of ambiguous/undocumented RPG
      // MOVE/MOVEL numeric<->character formatting, plus a couple of
      // other opcode edge cases
      //
      // NOTE: MOVEA cannot mix numeric and character operands at all
      // (IBM docs: "Both factor 2 and the result field must be the
      // same type") -- that is a *compile-time* error. The tests that
      // exercise it are in RPGVERIFY2.rpgle instead, so a compile
      // failure there does not block this program.
      //
      // This is "mixed free" source: free-form lines have columns 1-6
      // blank; MOVEA/MOVE have no free-form syntax, so those lines are
      // traditional fixed-form C-specs with C in column 6.
      // ============================================================

      dcl-s int8         int(3);
      dcl-s int16        int(5);
      dcl-s int32        int(10);
      dcl-s int64        int(20);
      dcl-s uns8         uns(3);
      dcl-s uns16        uns(5);
      dcl-s zon8         zoned(3:0);
      dcl-s pak8         packed(3:0);
      dcl-s int8Arr      int(3) dim(3);
      dcl-s uns8Arr      uns(3) dim(3);
      dcl-s pak1Arr      packed(1:0) dim(3);
      dcl-s charArr      char(4) dim(3);
      dcl-s bigChar      char(20);
      dcl-s srcChar      char(20);
      dcl-s resInt       int(10);
      dcl-s probeVal     zoned(10:0);

      dsply '=== SECTION A: MOVE numeric -> char, positive 32, by size ===';

      int8 = 32;
      bigChar = '####################';
      dsply 'A1 MOVE int8(3,1byte)=32 -> char(20)';
     C                   MOVE      int8          bigChar
      dsply bigChar;

      int16 = 32;
      bigChar = '####################';
      dsply 'A2 MOVE int16(5,2byte)=32 -> char(20)';
     C                   MOVE      int16         bigChar
      dsply bigChar;

      int32 = 32;
      bigChar = '####################';
      dsply 'A3 MOVE int32(10,4byte)=32 -> char(20)';
     C                   MOVE      int32         bigChar
      dsply bigChar;

      int64 = 32;
      bigChar = '####################';
      dsply 'A4 MOVE int64(20,8byte)=32 -> char(20)';
     C                   MOVE      int64         bigChar
      dsply bigChar;

      uns8 = 32;
      bigChar = '####################';
      dsply 'A5 MOVE uns8(3,1byte)=32 -> char(20)';
     C                   MOVE      uns8          bigChar
      dsply bigChar;

      uns16 = 32;
      bigChar = '####################';
      dsply 'A6 MOVE uns16(5,2byte)=32 -> char(20)';
     C                   MOVE      uns16         bigChar
      dsply bigChar;

      int32 = 32;
      bigChar = '####################';
      dsply 'A7 MOVEL int32(10,4byte)=32 -> char(20) (left vs right just.)';
     C                   MOVEL     int32         bigChar
      dsply bigChar;

      dsply '=== SECTION B: boundary and negative values ===';

      int8 = 127;
      bigChar = '####################';
      dsply 'B1 MOVE int8=127 (max) -> char(20)';
     C                   MOVE      int8          bigChar
      dsply bigChar;

      int8 = -32;
      bigChar = '####################';
      dsply 'B2 MOVE int8=-32 -> char(20)';
     C                   MOVE      int8          bigChar
      dsply bigChar;

      int8 = -128;
      bigChar = '####################';
      dsply 'B3 MOVE int8=-128 (min) -> char(20)';
     C                   MOVE      int8          bigChar
      dsply bigChar;

      uns8 = 255;
      bigChar = '####################';
      dsply 'B4 MOVE uns8=255 (max) -> char(20)';
     C                   MOVE      uns8          bigChar
      dsply bigChar;

      dsply '=== SECTION C: baseline calibration, zoned/packed ===';

      zon8 = 32;
      bigChar = '####################';
      dsply 'C1 MOVE zoned(3,0)=32 -> char(20)';
     C                   MOVE      zon8          bigChar
      dsply bigChar;

      pak8 = 32;
      bigChar = '####################';
      dsply 'C2 MOVE packed(3,0)=32 -> char(20)';
     C                   MOVE      pak8          bigChar
      dsply bigChar;

      dsply '=== SECTION D: reverse direction, char digit-text -> numeric ===';

      srcChar = '                 032';
      int8 = 0;
      dsply 'D1 MOVE char(20), last 3 bytes "032" -> int8(3,1byte)';
     C                   MOVE      srcChar       int8
      dsply %char(int8);

      srcChar = '          0000000032';
      resInt = 0;
      dsply 'D2 MOVE char(20), last 10 bytes "0000000032" -> int32(10,4byte)';
     C                   MOVE      srcChar       resInt
      dsply %char(resInt);

      dsply '=== SECTION E: is the moved byte pattern valid digit text? ===';

      int8 = 32;
      bigChar = '####################';
     C                   MOVE      int8          bigChar
      monitor;
        probeVal = %dec(%subst(bigChar:%len(%trimr(bigChar))-2:3):3:0);
        dsply 'E1 %dec succeeded on last 3 non-blank bytes, value:';
        dsply %char(probeVal);
      on-error;
        dsply 'E1 %dec FAILED -- not valid digit text';
      endmon;

      dsply '=== SECTION F: valid numeric MOVEA, same byte length ===';

      int8Arr(1) = 1;
      int8Arr(2) = 2;
      int8Arr(3) = 3;
      uns8Arr(1) = 0;
      uns8Arr(2) = 0;
      uns8Arr(3) = 0;
      dsply 'F1 MOVEA int(3) dim(3)=[1,2,3] -> uns(3) dim(3), same 1-byte length';
     C                   MOVEA     int8Arr       uns8Arr
      dsply %char(uns8Arr(1));
      dsply %char(uns8Arr(2));
      dsply %char(uns8Arr(3));

      int8Arr(1) = -1;
      uns8 = 0;
      dsply 'F2 MOVEA int8Arr(1)=-1 (all bits set) -> uns8, same 1-byte len';
     C                   MOVEA     int8Arr(1)    uns8
      dsply %char(uns8);

      pak1Arr(1) = 5;
      pak1Arr(2) = 6;
      pak1Arr(3) = 7;
      int8Arr(1) = 0;
      int8Arr(2) = 0;
      int8Arr(3) = 0;
      dsply 'F3 MOVEA packed(1,0) dim(3)=[5,6,7] -> int(3) dim(3), 1-byte each';
     C                   MOVEA     pak1Arr       int8Arr
      dsply %char(int8Arr(1));
      dsply %char(int8Arr(2));
      dsply %char(int8Arr(3));

      dsply '=== SECTION G: valid character MOVEA, crossing element bounds ===';

      charArr(1) = 'AAAA';
      charArr(2) = 'BBBB';
      charArr(3) = 'CCCC';
      bigChar = '####################';
      dsply 'G1 MOVEA char(4) dim(3) -> char(20), starting mid-element';
     C     2             MOVEA     charArr       bigChar
      dsply bigChar;

      bigChar = 'XY';
      charArr(1) = '....';
      charArr(2) = '....';
      charArr(3) = '....';
      dsply 'G2 MOVEA char(20)="XY..." -> char(4) dim(3), ends mid-element';
     C                   MOVEA     bigChar       charArr
      dsply charArr(1);
      dsply charArr(2);
      dsply charArr(3);

      dsply '=== SECTION H: a couple of other undocumented edge cases ===';

      dsply 'H1 %scan needle longer than haystack, expect 0';
      dsply %char(%scan('LONGNEEDLE':'shrt'));

      dsply 'H2 integer divide by zero';
      int8 = 5;
      int16 = 0;
      monitor;
        resInt = int8 / int16;
        dsply 'H2 no error, result:';
        dsply %char(resInt);
      on-error;
        dsply 'H2 runtime error raised (as expected)';
      endmon;

      dsply '=== DONE ===';
      *inlr = *on;
