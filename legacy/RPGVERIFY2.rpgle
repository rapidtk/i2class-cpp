      // ============================================================
      // RPGVERIFY2 -- lines that, per current IBM documentation,
      // should NOT compile (TEST 4 is the one confirmed exception --
      // see its comment below).
      // ============================================================

       dcl-s int8         int(3);
       dcl-s int16        int(5);
       dcl-s pak1Arr      packed(3:0) dim(3);
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

      // TEST 4: MOVEA packed(3,0) array -> int(3) array. Both have the
      // same DIGIT length (3), even though their underlying byte storage
      // differs (packed(3,0) is 2 bytes, int(3) is 1 byte). Confirmed via
      // real compile: this is accepted with NO error -- MOVEA's type
      // check apparently matches declared digit length, not byte storage
      // size. (Compilation of the whole program still fails overall
      // because of TESTs 1-3, so this line's runtime behavior is
      // untested; a real move between packed and binary storage would
      // need to be checked in RPGVERIFY.rpgle if that matters.)
     C                   MOVEA     pak1Arr       int8Arr

       *inlr = *on;
