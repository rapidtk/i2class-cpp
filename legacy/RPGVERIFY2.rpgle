      // ============================================================
      // RPGVERIFY2 -- lines that, per current IBM documentation,
      // should NOT compile. 
      // ============================================================

      dcl-s int8         int(3);
      dcl-s int16        int(5);
      dcl-s bigChar      char(20);
      dcl-s int8Arr      int(3) dim(3);
      dcl-s int16Arr     int(5) dim(3);

      // TEST 1: MOVEA numeric -> character. Expect a compile error --
      // "Both factor 2 and the result field must be the same type."
     C                   MOVEA     int8Arr       bigChar

      // TEST 2: MOVEA between numeric arrays of different BYTE length
      // (int(3)=1 byte vs int(5)=2 byte elements). Expect a compile
      // error -- "same numeric length defined."
     C                   MOVEA     int8Arr       int16Arr

      // TEST 3: MOVEA where NEITHER side is an array. Expect a compile
      // error -- "Factor 2 or the result field must contain an array."
     C                   MOVEA     int8          int16

      *inlr = *on;
