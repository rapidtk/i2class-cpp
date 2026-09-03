       // ============================================================
       // RPGVERIFY3 -- empirical probes for %DECH, %INTH, and %UNSH.
       //
       // Compile: CRTBNDRPG PGM(RPGVERIFY3)
      //          SRCSTMF('/home/ACLARK/git/i2class-cpp/legacy/')
      //          RPGVERIFY3.rpgle
       // Run:     CALL RPGVERIFY3, then WRKSPLF to inspect QSYSPRT.
       // ============================================================

       dcl-f qsysprt printer(100);

       dcl-s dec50       zoned(7:0);
       dcl-s dec51       zoned(7:1);
       dcl-s intResult   int(10);
       dcl-s unsResult   uns(10);
       dcl-s prtTxt      char(100);

       prtTxt = 'SECTION A: %DECH to zero decimal places';
       except PRTLINE;

       dec50 = %dech(1.49:7:0);
       prtTxt = 'A1 %DECH(1.49:7:0) = ' + %char(dec50);
       except PRTLINE;
       dec50 = %dech(1.50:7:0);
       prtTxt = 'A2 %DECH(1.50:7:0) = ' + %char(dec50);
       except PRTLINE;
       dec50 = %dech(1.51:7:0);
       prtTxt = 'A3 %DECH(1.51:7:0) = ' + %char(dec50);
       except PRTLINE;
       dec50 = %dech(-1.49:7:0);
       prtTxt = 'A4 %DECH(-1.49:7:0) = ' + %char(dec50);
       except PRTLINE;
       dec50 = %dech(-1.50:7:0);
       prtTxt = 'A5 %DECH(-1.50:7:0) = ' + %char(dec50);
       except PRTLINE;
       dec50 = %dech(-1.51:7:0);
       prtTxt = 'A6 %DECH(-1.51:7:0) = ' + %char(dec50);
       except PRTLINE;

       prtTxt = 'SECTION B: %DECH reducing two fractional digits to one';
       except PRTLINE;
       dec51 = %dech(12.34:7:1);
       prtTxt = 'B1 %DECH(12.34:7:1) = ' + %char(dec51);
       except PRTLINE;
       dec51 = %dech(12.35:7:1);
       prtTxt = 'B2 %DECH(12.35:7:1) = ' + %char(dec51);
       except PRTLINE;
       dec51 = %dech(-12.34:7:1);
       prtTxt = 'B3 %DECH(-12.34:7:1) = ' + %char(dec51);
       except PRTLINE;
       dec51 = %dech(-12.35:7:1);
       prtTxt = 'B4 %DECH(-12.35:7:1) = ' + %char(dec51);
       except PRTLINE;

       prtTxt = 'SECTION C: %INTH signed half-adjust';
       except PRTLINE;
       intResult = %inth(1.49);
       prtTxt = 'C1 %INTH(1.49) = ' + %char(intResult);
       except PRTLINE;
       intResult = %inth(1.50);
       prtTxt = 'C2 %INTH(1.50) = ' + %char(intResult);
       except PRTLINE;
       intResult = %inth(-1.49);
       prtTxt = 'C3 %INTH(-1.49) = ' + %char(intResult);
       except PRTLINE;
       intResult = %inth(-1.50);
       prtTxt = 'C4 %INTH(-1.50) = ' + %char(intResult);
       except PRTLINE;

       prtTxt = 'SECTION D: %UNSH unsigned half-adjust';
       except PRTLINE;
       unsResult = %unsh(1.49);
       prtTxt = 'D1 %UNSH(1.49) = ' + %char(unsResult);
       except PRTLINE;
       unsResult = %unsh(1.50);
       prtTxt = 'D2 %UNSH(1.50) = ' + %char(unsResult);
       except PRTLINE;
       monitor;
         unsResult = %unsh(-1.50);
         prtTxt = 'D3 %UNSH(-1.50) accepted = ' + %char(unsResult);
         except PRTLINE;
       on-error;
         prtTxt = 'D3 %UNSH(-1.50) raised runtime error';
         except PRTLINE;
       endmon;

       prtTxt = '=== DONE ===';
       except PRTLINE;
       *inlr = *on;

     OQSYSPRT   E            PRTLINE        1
     O                       prtTxt             100
