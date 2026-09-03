      // ============================================================
      // RPGVERIFY4 -- empirical probes for edit codes and edit words.
      //
      // Every value is printed between a '>' and a '<' marker so leading and
      // trailing blanks in the edited result are visible and can be compared
      // character for character against the C++ implementation's committed
      // report, i2class-cpp/tests/edit-output.txt.
      //
      // The edit codes and edit words are coded directly in the O-specs
      // (edit code in position 44, edit word in positions 53-80). The two
      // asterisk-fill / floating-currency sections use %EDITC instead, since
      // that spelling of the same editing is unambiguous in free form.
      //
      // Compile: CRTBNDRPG PGM(RPGVERIFY4)
      //          SRCSTMF('/home/ACLARK/git/i2class-cpp/legacy/RPGVERIFY4.rpgle')
      // Run:     CALL RPGVERIFY4, then WRKSPLF to inspect QSYSPRT.
      // ============================================================

       dcl-f qsysprt printer(132);

       dcl-s prtTxt char(120);

      // Section A -- digit grouping, one field per integer width 1..9
       dcl-s g1 zoned(1:0) inz(5);
       dcl-s g2 zoned(2:0) inz(45);
       dcl-s g3 zoned(3:0) inz(345);
       dcl-s g4 zoned(4:0) inz(2345);
       dcl-s g5 zoned(5:0) inz(12345);
       dcl-s g6 zoned(6:0) inz(123456);
       dcl-s g7 zoned(7:0) inz(1234567);
       dcl-s g8 zoned(8:0) inz(12345678);
       dcl-s g9 zoned(9:0) inz(123456789);

      // Sections B/C -- the six sample values the EDTCDE table is built on
       dcl-s v72  zoned(7:2) inz(12345.67);
       dcl-s v70  zoned(7:0) inz(1234567);
       dcl-s v73n zoned(7:3) inz(-0.125);
       dcl-s v30n zoned(3:0) inz(-125);
       dcl-s z62  zoned(6:2) inz(0);
       dcl-s z60  zoned(6:0) inz(0);

      // Section E -- the DDS examples worked through in the EDTCDE reference
       dcl-s price  zoned(5:2) inz(-123.45);
       dcl-s sales  zoned(7:2) inz(-12345.67);
       dcl-s salary zoned(8:2) inz(123456.78);

      // Section F -- edit word probes
       dcl-s e72  zoned(7:2) inz(12345.67);
       dcl-s e72n zoned(7:2) inz(-12345.67);
       dcl-s e72s zoned(7:2) inz(0.05);
       dcl-s e72t zoned(7:2) inz(0.07);
       dcl-s e72z zoned(7:2) inz(0);
       dcl-s e72a zoned(7:2) inz(1357.92);
       dcl-s e70  zoned(7:0) inz(1234567);
       dcl-s e70s zoned(7:0) inz(35);
       dcl-s e70z zoned(7:0) inz(0);
       dcl-s e70n zoned(7:0) inz(-35);
       dcl-s e70d zoned(7:0) inz(1230576);

       except HEAD;

       prtTxt = 'SECTION A: digit grouping, edit code 1, widths 1-9';
       except TEXT;
       except A1;
       except A2;
       except A3;
       except A4;
       except A5;
       except A6;
       except A7;
       except A8;
       except A9;

       prtTxt = 'SECTION B: zero balance, 6,0 and 6,2 fields holding zero';
       except TEXT;
       except B1;
       except B2;
       except B3;
       except B4;
       except B5;
       except B6;
       except B7;
       except B8;
       except B9;
       except B10;
       except B11;
       except B12;

       prtTxt = 'SECTION C: sign placement on -.125 (7,3) and -125 (3,0)';
       except TEXT;
       except C1;
       except C2;
       except C3;
       except C4;
       except C5;
       except C6;
       except C7;
       except C8;
       except C9;
       except C10;
       except C11;
       except C12;

       prtTxt = 'SECTION D: asterisk fill and floating currency, via %EDITC';
       except TEXT;
       prtTxt = 'D1  1 astfill  12345.67  >' + %editc(v72:'1':*ASTFILL) + '<';
       except TEXT;
       prtTxt = 'D2  1 astfill  0 (6,0)   >' + %editc(z60:'1':*ASTFILL) + '<';
       except TEXT;
       prtTxt = 'D3  1 astfill  0 (6,2)   >' + %editc(z62:'1':*ASTFILL) + '<';
       except TEXT;
       prtTxt = 'D4  J astfill  -.125     >' + %editc(v73n:'J':*ASTFILL) + '<';
       except TEXT;
       prtTxt = 'D5  N astfill  -.125     >' + %editc(v73n:'N':*ASTFILL) + '<';
       except TEXT;
       prtTxt = 'D6  1 cursym   12345.67  >' + %editc(v72:'1':*CURSYM) + '<';
       except TEXT;
       prtTxt = 'D7  1 cursym   0 (6,0)   >' + %editc(z60:'1':*CURSYM) + '<';
       except TEXT;
       prtTxt = 'D8  J cursym   -.125     >' + %editc(v73n:'J':*CURSYM) + '<';
       except TEXT;
       prtTxt = 'D9  N cursym   -.125     >' + %editc(v73n:'N':*CURSYM) + '<';
       except TEXT;
       prtTxt = 'D10 K cursym   -12345.67 >' + %editc(sales:'K':*CURSYM) + '<';
       except TEXT;
       prtTxt = 'D11 1 astfill  123456.78 >' + %editc(salary:'1':*ASTFILL) + '<';
       except TEXT;

       prtTxt = 'SECTION E: DDS examples from the EDTCDE reference';
       except TEXT;
       except E1;
       prtTxt = 'E2 SALES  7,2 EDTCDE(K $) >' + %editc(sales:'K':*CURSYM) + '<';
       except TEXT;
       prtTxt = 'E3 SALARY 8,2 EDTCDE(1 *) >' + %editc(salary:'1':*ASTFILL) + '<';
       except TEXT;

       prtTxt = 'SECTION F: edit words';
       except TEXT;
       except F1;
       except F2;
       except F3;
       except F4;
       except F5;
       except F6;
       except F7;
       except F8;
       except F9;
       except F10;
       except F11;
       except F12;
       except F13;
       except F14;
       except F15;
       except F16;
       except F17;
       except F18;
       except F19;
       except F20;
       except F21;
       except F22;
       except F23;
       except F24;

       prtTxt = '=== DONE ===';
       except TEXT;
       *inlr = *on;

      //*.. 1 ...+... 2 ...+... 3 ...+... 4 ...+... 5 ...+... 6 ...+... 7
      //OFilename++DF..N01N02N03Excnam++++B++A++Sb+Sa+
      //O..............N01N02N03Field+++++++++YB.End++PConstant/editword+++

     OQSYSPRT   E            HEAD           1
     O                                           46 'RPGVERIFY4 EDIT CODE / EDIT WORD PROBE'
     OQSYSPRT   E            TEXT           1
     O                       prtTxt             125

      * ---- Section A: edit code 1 across integer widths 1 through 9 ----
     OQSYSPRT   E            A1             1
     O                                           30 'A1 width 1  code 1'
     O                                           32 '>'
     O                       g1            1     60
     O                                           61 '<'
     OQSYSPRT   E            A2             1
     O                                           30 'A2 width 2  code 1'
     O                                           32 '>'
     O                       g2            1     60
     O                                           61 '<'
     OQSYSPRT   E            A3             1
     O                                           30 'A3 width 3  code 1'
     O                                           32 '>'
     O                       g3            1     60
     O                                           61 '<'
     OQSYSPRT   E            A4             1
     O                                           30 'A4 width 4  code 1'
     O                                           32 '>'
     O                       g4            1     60
     O                                           61 '<'
     OQSYSPRT   E            A5             1
     O                                           30 'A5 width 5  code 1'
     O                                           32 '>'
     O                       g5            1     60
     O                                           61 '<'
     OQSYSPRT   E            A6             1
     O                                           30 'A6 width 6  code 1'
     O                                           32 '>'
     O                       g6            1     60
     O                                           61 '<'
     OQSYSPRT   E            A7             1
     O                                           30 'A7 width 7  code 1'
     O                                           32 '>'
     O                       g7            1     60
     O                                           61 '<'
     OQSYSPRT   E            A8             1
     O                                           30 'A8 width 8  code 1'
     O                                           32 '>'
     O                       g8            1     60
     O                                           61 '<'
     OQSYSPRT   E            A9             1
     O                                           30 'A9 width 9  code 1'
     O                                           32 '>'
     O                       g9            1     60
     O                                           61 '<'

      * ---- Section B: zero balance ----
     OQSYSPRT   E            B1             1
     O                                           30 'B1 zero 6,0 code 1'
     O                                           32 '>'
     O                       z60           1     60
     O                                           61 '<'
     OQSYSPRT   E            B2             1
     O                                           30 'B2 zero 6,0 code 2'
     O                                           32 '>'
     O                       z60           2     60
     O                                           61 '<'
     OQSYSPRT   E            B3             1
     O                                           30 'B3 zero 6,0 code 3'
     O                                           32 '>'
     O                       z60           3     60
     O                                           61 '<'
     OQSYSPRT   E            B4             1
     O                                           30 'B4 zero 6,0 code 4'
     O                                           32 '>'
     O                       z60           4     60
     O                                           61 '<'
     OQSYSPRT   E            B5             1
     O                                           30 'B5 zero 6,0 code A'
     O                                           32 '>'
     O                       z60           A     60
     O                                           61 '<'
     OQSYSPRT   E            B6             1
     O                                           30 'B6 zero 6,0 code B'
     O                                           32 '>'
     O                       z60           B     60
     O                                           61 '<'
     OQSYSPRT   E            B7             1
     O                                           30 'B7 zero 6,0 code J'
     O                                           32 '>'
     O                       z60           J     60
     O                                           61 '<'
     OQSYSPRT   E            B8             1
     O                                           30 'B8 zero 6,0 code N'
     O                                           32 '>'
     O                       z60           N     60
     O                                           61 '<'
     OQSYSPRT   E            B9             1
     O                                           30 'B9 zero 6,2 code 1'
     O                                           32 '>'
     O                       z62           1     60
     O                                           61 '<'
     OQSYSPRT   E            B10            1
     O                                           30 'B10 zero 6,2 code 2'
     O                                           32 '>'
     O                       z62           2     60
     O                                           61 '<'
     OQSYSPRT   E            B11            1
     O                                           30 'B11 zero 6,2 code 3'
     O                                           32 '>'
     O                       z62           3     60
     O                                           61 '<'
     OQSYSPRT   E            B12            1
     O                                           30 'B12 zero 6,2 code J'
     O                                           32 '>'
     O                       z62           J     60
     O                                           61 '<'

      * ---- Section C: sign placement ----
     OQSYSPRT   E            C1             1
     O                                           30 'C1 -.125 7,3 code A'
     O                                           32 '>'
     O                       v73n          A     60
     O                                           61 '<'
     OQSYSPRT   E            C2             1
     O                                           30 'C2 -.125 7,3 code C'
     O                                           32 '>'
     O                       v73n          C     60
     O                                           61 '<'
     OQSYSPRT   E            C3             1
     O                                           30 'C3 -.125 7,3 code J'
     O                                           32 '>'
     O                       v73n          J     60
     O                                           61 '<'
     OQSYSPRT   E            C4             1
     O                                           30 'C4 -.125 7,3 code L'
     O                                           32 '>'
     O                       v73n          L     60
     O                                           61 '<'
     OQSYSPRT   E            C5             1
     O                                           30 'C5 -.125 7,3 code N'
     O                                           32 '>'
     O                       v73n          N     60
     O                                           61 '<'
     OQSYSPRT   E            C6             1
     O                                           30 'C6 -.125 7,3 code P'
     O                                           32 '>'
     O                       v73n          P     60
     O                                           61 '<'
     OQSYSPRT   E            C7             1
     O                                           30 'C7 -125 3,0 code A'
     O                                           32 '>'
     O                       v30n          A     60
     O                                           61 '<'
     OQSYSPRT   E            C8             1
     O                                           30 'C8 -125 3,0 code C'
     O                                           32 '>'
     O                       v30n          C     60
     O                                           61 '<'
     OQSYSPRT   E            C9             1
     O                                           30 'C9 -125 3,0 code J'
     O                                           32 '>'
     O                       v30n          J     60
     O                                           61 '<'
     OQSYSPRT   E            C10            1
     O                                           30 'C10 -125 3,0 code L'
     O                                           32 '>'
     O                       v30n          L     60
     O                                           61 '<'
     OQSYSPRT   E            C11            1
     O                                           30 'C11 -125 3,0 code N'
     O                                           32 '>'
     O                       v30n          N     60
     O                                           61 '<'
     OQSYSPRT   E            C12            1
     O                                           30 'C12 -125 3,0 code P'
     O                                           32 '>'
     O                       v30n          P     60
     O                                           61 '<'

      * ---- Section E: DDS example PRICE ----
     OQSYSPRT   E            E1             1
     O                                           30 'E1 PRICE 5,2 EDTCDE(J)'
     O                                           32 '>'
     O                       price         J     60
     O                                           61 '<'

      * ---- Section F: edit words ----
     OQSYSPRT   E            F1             1
     O                                           30 'F1  7,0 1234567 no stop'
     O                                           32 '>'
     O                       e70                 60 '       '
     O                                           61 '<'
     OQSYSPRT   E            F2             1
     O                                           30 'F2  7,0 35 no stop'
     O                                           32 '>'
     O                       e70s                60 '       '
     O                                           61 '<'
     OQSYSPRT   E            F3             1
     O                                           30 'F3  7,0 35 stop at units'
     O                                           32 '>'
     O                       e70s                60 '      0'
     O                                           61 '<'
     OQSYSPRT   E            F4             1
     O                                           30 'F4  7,0 0 stop at units'
     O                                           32 '>'
     O                       e70z                60 '      0'
     O                                           61 '<'
     OQSYSPRT   E            F5             1
     O                                           30 'F5  7,0 0 stop left of units'
     O                                           32 '>'
     O                       e70z                60 '     0 '
     O                                           61 '<'
     OQSYSPRT   E            F6             1
     O                                           30 'F6  7,2 12345.67 commas'
     O                                           32 '>'
     O                       e72                 60 '  ,  0.  '
     O                                           61 '<'
     OQSYSPRT   E            F7             1
     O                                           30 'F7  7,2 0.07 commas'
     O                                           32 '>'
     O                       e72t                60 '  ,  0.  '
     O                                           61 '<'
     OQSYSPRT   E            F8             1
     O                                           30 'F8  7,2 0.00 commas'
     O                                           32 '>'
     O                       e72z                60 '  ,  0.  '
     O                                           61 '<'
     OQSYSPRT   E            F9             1
     O                                           30 'F9  7,2 -12345.67 CR'
     O                                           32 '>'
     O                       e72n                60 '  ,  0.  CR'
     O                                           61 '<'
     OQSYSPRT   E            F10            1
     O                                           30 'F10 7,2 12345.67 CR'
     O                                           32 '>'
     O                       e72                 60 '  ,  0.  CR'
     O                                           61 '<'
     OQSYSPRT   E            F11            1
     O                                           30 'F11 7,2 -12345.67 minus'
     O                                           32 '>'
     O                       e72n                60 '  ,  0.  -'
     O                                           61 '<'
     OQSYSPRT   E            F12            1
     O                                           30 'F12 7,2 12345.67 minus'
     O                                           32 '>'
     O                       e72                 60 '  ,  0.  -'
     O                                           61 '<'
     OQSYSPRT   E            F13            1
     O                                           30 'F13 7,0 -35 minus no stop'
     O                                           32 '>'
     O                       e70n                60 '       -'
     O                                           61 '<'
     OQSYSPRT   E            F14            1
     O                                           30 'F14 7,2 1357.92 astfill'
     O                                           32 '>'
     O                       e72a                60 '  ,  *.  '
     O                                           61 '<'
     OQSYSPRT   E            F15            1
     O                                           30 'F15 7,2 0.00 astfill'
     O                                           32 '>'
     O                       e72z                60 '  ,  *.  '
     O                                           61 '<'
     OQSYSPRT   E            F16            1
     O                                           30 'F16 7,0 35 astfill'
     O                                           32 '>'
     O                       e70s                60 '      *'
     O                                           61 '<'
     OQSYSPRT   E            F17            1
     O                                           30 'F17 7,2 12345.67 float $'
     O                                           32 '>'
     O                       e72                 60 '    $0.  '
     O                                           61 '<'
     OQSYSPRT   E            F18            1
     O                                           30 'F18 7,2 0.05 float $'
     O                                           32 '>'
     O                       e72s                60 '    $0.  '
     O                                           61 '<'
     OQSYSPRT   E            F19            1
     O                                           30 'F19 7,2 0.00 float $'
     O                                           32 '>'
     O                       e72z                60 '    $0.  '
     O                                           61 '<'
     OQSYSPRT   E            F20            1
     O                                           30 'F20 7,2 12345.67 fixed $'
     O                                           32 '>'
     O                       e72                 60 '$    0.  '
     O                                           61 '<'
     OQSYSPRT   E            F21            1
     O                                           30 'F21 7,2 0.05 fixed $'
     O                                           32 '>'
     O                       e72s                60 '$    0.  '
     O                                           61 '<'
     OQSYSPRT   E            F22            1
     O                                           30 'F22 7,2 12345.67 $ with commas'
     O                                           32 '>'
     O                       e72                 60 '  ,  $0.  '
     O                                           61 '<'
     OQSYSPRT   E            F23            1
     O                                           30 'F23 7,0 1234567 ampersands'
     O                                           32 '>'
     O                       e70                 60 '   &  &  '
     O                                           61 '<'
     OQSYSPRT   E            F24            1
     O                                           30 'F24 7,0 1230576 constants'
     O                                           32 '>'
     O                       e70d                60 '  0/  /  '
     O                                           61 '<'
