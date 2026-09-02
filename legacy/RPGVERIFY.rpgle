       // ============================================================
       // RPGVERIFY -- empirical probes of ambiguous/undocumented RPG
       // MOVE/MOVEL numeric<->character formatting, plus a couple of
       // other opcode edge cases.
       //
       // NOTE: MOVEA cannot mix numeric and character operands at all
       // (IBM docs: "Both factor 2 and the result field must be the
       // same type") -- that is a *compile-time* error. The tests that
       // exercise it are in RPGVERIFY2.rpgle instead, so a compile
       // failure there does not block this program.
       //
       // Compile: CRTBNDRPG PGM(RPGVERIFY) SRCSTMF(...RPGVERIFY.rpgle)
       // Run:     CALL RPGVERIFY, then WRKSPLF to view/copy the output.
       // ============================================================

       dcl-f qsysprt printer(100);

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
       dcl-s int1Arr      int(3) dim(3);
       dcl-s charArr      char(4) dim(3);
       dcl-s tenChar      char(10);
       dcl-s bigChar      char(20);
       dcl-s srcChar      char(20);
       dcl-s resInt       int(10);
       dcl-s probeVal     zoned(10:0);
       dcl-s prtTxt       char(100);

<<<<<<< Updated upstream
       prtTxt = 'SECTION A: MOVE numeric -> char, positive 32, righ' +
       't/left justified';
=======
       prtTxt = 'SECTION A: MOVE numeric -> char, positive 32, ' +
        'right/left justified';
>>>>>>> Stashed changes
       except PRTLINE;

       int8 = 32;
       bigChar = '####################';
     C                   MOVE      int8          bigChar
<<<<<<< Updated upstream
       prtTxt = 'A1 MOVE int8, INT(3)/1-byte, value 32 into char(20' +
       ') filler: ' + bigChar;
=======
       prtTxt = 'A1 MOVE int8, INT(3)/1-byte, value 32 into char(20) ' +
        'filler: ' + bigChar;
>>>>>>> Stashed changes
       except PRTLINE;

       int16 = 32;
       bigChar = '####################';
     C                   MOVE      int16         bigChar
<<<<<<< Updated upstream
       prtTxt = 'A2 MOVE int16, INT(5)/2-byte, value 32 into char(2' +
       '0) filler: ' + bigChar;
=======
       prtTxt = 'A2 MOVE int16, INT(5)/2-byte, value 32 into char(20) ' +
        'filler: ' + bigChar;
>>>>>>> Stashed changes
       except PRTLINE;

       int32 = 32;
       bigChar = '####################';
     C                   MOVE      int32         bigChar
<<<<<<< Updated upstream
       prtTxt = 'A3 MOVE int32, INT(10)/4-byte, value 32 into char(' +
       '20) filler: ' + bigChar;
=======
       prtTxt = 'A3 MOVE int32, INT(10)/4-byte, value 32 into char(20) ' +
        'filler: ' + bigChar;
>>>>>>> Stashed changes
       except PRTLINE;

       int64 = 32;
       bigChar = '####################';
     C                   MOVE      int64         bigChar
<<<<<<< Updated upstream
       prtTxt = 'A4 MOVE int64, INT(20)/8-byte, value 32 into char(' +
       '20) filler: ' + bigChar;
=======
       prtTxt = 'A4 MOVE int64, INT(20)/8-byte, value 32 into char(20) ' +
        'filler: ' + bigChar;
>>>>>>> Stashed changes
       except PRTLINE;

       uns8 = 32;
       bigChar = '####################';
     C                   MOVE      uns8          bigChar
<<<<<<< Updated upstream
       prtTxt = 'A5 MOVE uns8, UNS(3)/1-byte, value 32 into char(20' +
       ') filler: ' + bigChar;
=======
       prtTxt = 'A5 MOVE uns8, UNS(3)/1-byte, value 32 into char(20) ' +
        'filler: ' + bigChar;
>>>>>>> Stashed changes
       except PRTLINE;

       uns16 = 32;
       bigChar = '####################';
     C                   MOVE      uns16         bigChar
<<<<<<< Updated upstream
       prtTxt = 'A6 MOVE uns16, UNS(5)/2-byte, value 32 into char(2' +
       '0) filler: ' + bigChar;
=======
       prtTxt = 'A6 MOVE uns16, UNS(5)/2-byte, value 32 into char(20) ' +
        'filler: ' + bigChar;
>>>>>>> Stashed changes
       except PRTLINE;

       int32 = 32;
       bigChar = '####################';
     C                   MOVEL     int32         bigChar
<<<<<<< Updated upstream
       prtTxt = 'A7 MOVEL (left-justified) int32=32 into char(20) f' +
       'iller: ' + bigChar;
       except PRTLINE;

       prtTxt = 'SECTION B: boundary and negative values (sign over' +
       'punch)';
=======
       prtTxt = 'A7 MOVEL (left-justified) int32=32 into char(20) ' +
        'filler: ' + bigChar;
       except PRTLINE;

       prtTxt = 'SECTION B: boundary and negative values (sign over punch)';
>>>>>>> Stashed changes
       except PRTLINE;

       int8 = 127;
       bigChar = '####################';
     C                   MOVE      int8          bigChar
       prtTxt = 'B1 MOVE int8=127 (max positive) into char(20) fill' +
       'er: ' + bigChar;
       except PRTLINE;

       int8 = -32;
       bigChar = '####################';
     C                   MOVE      int8          bigChar
       prtTxt = 'B2 MOVE int8=-32 into char(20) filler: ' + bigChar;
       except PRTLINE;

       int8 = -128;
       bigChar = '####################';
     C                   MOVE      int8          bigChar
       prtTxt = 'B3 MOVE int8=-128 (min negative) into char(20) fil' +
       'ler: ' + bigChar;
       except PRTLINE;

       uns8 = 255;
       bigChar = '####################';
     C                   MOVE      uns8          bigChar
       prtTxt = 'B4 MOVE uns8=255 (max) into char(20) filler: ' + bigChar;
       except PRTLINE;

<<<<<<< Updated upstream
       prtTxt = 'SECTION C: baseline calibration, zoned/packed -> c' + 'har';
=======
       prtTxt = 'SECTION C: baseline calibration, zoned/packed -> char';
>>>>>>> Stashed changes
       except PRTLINE;

       zon8 = 32;
       bigChar = '####################';
     C                   MOVE      zon8          bigChar
       prtTxt = 'C1 MOVE zoned(3,0)=32 into char(20) filler: ' + bigChar;
       except PRTLINE;

       pak8 = 32;
       bigChar = '####################';
     C                   MOVE      pak8          bigChar
       prtTxt = 'C2 MOVE packed(3,0)=32 into char(20) filler: ' + bigChar;
       except PRTLINE;

<<<<<<< Updated upstream
       prtTxt = 'SECTION D: reverse direction, char digit-text -> n' +
       'umeric';
=======
       prtTxt = 'SECTION D: reverse direction, char digit-text -> numeric';
>>>>>>> Stashed changes
       except PRTLINE;

       srcChar = '                 032';
       int8 = 0;
     C                   MOVE      srcChar       int8
<<<<<<< Updated upstream
       prtTxt = 'D1 MOVE char(20), rightmost 3 bytes ''032'', into in' +
       't8(3): ' + %char(int8);
=======
       prtTxt = 'D1 MOVE char(20), rightmost 3 bytes ''032'', into int8(3): ' + 
        %char(int8);
>>>>>>> Stashed changes
       except PRTLINE;

       srcChar = '          0000000032';
       resInt = 0;
     C                   MOVE      srcChar       resInt
<<<<<<< Updated upstream
       prtTxt = 'D2 MOVE char(20), rightmost 10 bytes ''0000000032'',' +
       ' into ' + 'int32(10): ' + %char(resInt);
       except PRTLINE;

       prtTxt = 'SECTION E: is the moved byte pattern valid decimal' +
       ' digit text?';
=======
       prtTxt = 'D2 MOVE char(20), rightmost 10 bytes ''0000000032'', ' +
        'into int32(10): ' + %char(resInt);
       except PRTLINE;

       prtTxt = 'SECTION E: is moved byte pattern valid decimal digit text?';
>>>>>>> Stashed changes
       except PRTLINE;

       int8 = 32;
       bigChar = '####################';
     C                   MOVE      int8          bigChar
       monitor;
         probeVal = %dec(%subst(bigChar:18:3):3:0);
         prtTxt = 'E1 %dec succeeded on bigChar bytes 18-20, value: ' +
           %char(probeVal);
         except PRTLINE;
       on-error;
         prtTxt = 'E1 %dec FAILED -- bytes 18-20 are not valid digit text';
         except PRTLINE;
       endmon;

       prtTxt = 'SECTION F: valid numeric MOVEA, same byte length';
       except PRTLINE;

       int8Arr(1) = 1;
       int8Arr(2) = 2;
       int8Arr(3) = 3;
       uns8Arr(1) = 0;
       uns8Arr(2) = 0;
       uns8Arr(3) = 0;
       monitor;
     C                   MOVEA     int8Arr       uns8Arr
         prtTxt = 'F1 MOVEA int(3)[1,2,3] -> uns(3), elems now: ' +
           %char(uns8Arr(1)) + ' ' + %char(uns8Arr(2)) + ' ' +
           %char(uns8Arr(3));
         except PRTLINE;
       on-error;
         prtTxt = 'F1 MOVEA raised a runtime error';
         except PRTLINE;
       endmon;

       int8Arr(1) = -1;
       uns8 = 0;
       monitor;
     C                   MOVEA     int8Arr(1)    uns8
         prtTxt = 'F2 MOVEA int8Arr(1)=-1 (all bits set) -> uns8, result: ' +
           %char(uns8);
         except PRTLINE;
       on-error;
<<<<<<< Updated upstream
         prtTxt = 'F2 MOVEA raised a runtime error (value -1 invalid' +
           ' for UNS)';
=======
         prtTxt = 'F2 MOVEA raised a runtime error (value -1 invalid for UNS)';
>>>>>>> Stashed changes
         except PRTLINE;
       endmon;

       int1Arr(1) = 5;
       int1Arr(2) = 6;
       int1Arr(3) = 7;
       int8Arr(1) = 0;
       int8Arr(2) = 0;
       int8Arr(3) = 0;
       monitor;
     C                   MOVEA     int1Arr       int8Arr
<<<<<<< Updated upstream
         prtTxt = 'F3 MOVEA int(3)[5,6,7] -> int(3), same-type baseline,' +
           ' elems now: ' + %char(int8Arr(1)) + ' ' +
=======
         prtTxt = 'F3 MOVEA int(3)[5,6,7] -> int(3), same-type baseline, ' +
           'elems now: ' + %char(int8Arr(1)) + ' ' +
>>>>>>> Stashed changes
           %char(int8Arr(2)) + ' ' + %char(int8Arr(3));
         except PRTLINE;
       on-error;
         prtTxt = 'F3 MOVEA raised a runtime error';
         except PRTLINE;
       endmon;

<<<<<<< Updated upstream
       prtTxt = 'SECTION G: valid character MOVEA, crossing array-e' +
       'lement bounds';
=======
       prtTxt = 'SECTION G: valid character MOVEA, crossing ' +
        'array-element bounds';
>>>>>>> Stashed changes
       except PRTLINE;

       charArr(1) = 'AAAA';
       charArr(2) = 'BBBB';
       charArr(3) = 'CCCC';
       bigChar = '####################';
     C                   MOVEA     charArr(2)    bigChar
<<<<<<< Updated upstream
       prtTxt = 'G1 MOVEA char(4)[3], starting at element 2, into c' +
       'har(20): ' + bigChar;
=======
       prtTxt = 'G1 MOVEA char(4)[3], starting at element 2, into char(20): ' + 
        bigChar;
>>>>>>> Stashed changes
       except PRTLINE;

       tenChar = 'ABCDEFGHIJ';
       charArr(1) = 'WWWW';
       charArr(2) = 'XXXX';
       charArr(3) = 'YYYY';
     C                   MOVEA     tenChar       charArr
<<<<<<< Updated upstream
         prtTxt = 'G2 MOVEA char(10) into char(4)[3] (stops mid-element' +
           ' 3), elems now: ' + charArr(1) + ' / ' + charArr(2) +
           ' / ' + charArr(3);
         except PRTLINE;

       prtTxt = 'SECTION H: a couple of other undocumented edge cas' + 'es';
=======
         prtTxt = 'G2 MOVEA char(10) into char(4)[3] (stops mid-element 3), ' +
          'elems now: ' + charArr(1) + ' / ' + charArr(2) + ' / ' + charArr(3);
         except PRTLINE;

       prtTxt = 'SECTION H: a couple of other undocumented edge cases';
>>>>>>> Stashed changes
       except PRTLINE;

         prtTxt = 'H1 %scan needle longer than haystack, expect 0, got:' +
           ' ' + %char(%scan('LONGNEEDLE':'shrt'));
         except PRTLINE;

       int8 = 5;
       int16 = 0;
       monitor;
         resInt = int8 / int16;
         prtTxt = 'H2 integer divide by zero: no error, result: ' +
           %char(resInt);
         except PRTLINE;
       on-error;
<<<<<<< Updated upstream
         prtTxt = 'H2 integer divide by zero: runtime error raised' +
           ' (as expected)';
=======
         prtTxt = 'H2 integer divide by zero: runtime error raised ' +
           '(as expected)';
>>>>>>> Stashed changes
         except PRTLINE;
       endmon;

       prtTxt = '=== DONE ===';
       except PRTLINE;
       *inlr = *on;

     OQSYSPRT   E            PRTLINE        1
     O                       prtTxt             100
