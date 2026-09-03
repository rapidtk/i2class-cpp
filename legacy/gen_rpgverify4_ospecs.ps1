# Regenerates the O-spec block of RPGVERIFY4.rpgle at exact column positions.
# RPG IV output spec: 7-29 reserved, 30-43 field name, 44 edit code,
# 45 blank-after, 46 reserved, 47-51 end position, 52 format, 53+ constant.

$path = "$PSScriptRoot\RPGVERIFY4.rpgle"

function Rec([string]$name) {
    # 1-5 blank, 6 'O', 7-16 filename, 17 type, 18-29 blank, 30-39 EXCEPT name,
    # 40-42 space before, 43-45 space after (1, matching RPGVERIFY3)
    "     O" + "QSYSPRT   " + "E" + (" " * 12) + $name.PadRight(10) + (" " * 5) + "1"
}
function Fld([string]$name, [string]$code, [int]$end, [string]$word) {
    $line = "     O" + (" " * 23) + $name.PadRight(14) + $(if ($code) { $code } else { " " }) `
          + "  " + ([string]$end).PadLeft(5)
    if ($word) { $line += " '" + $word + "'" }
    $line
}
function Con([int]$end, [string]$text) {
    "     O" + (" " * 40) + ([string]$end).PadLeft(5) + " '" + $text + "'"
}

$L = New-Object System.Collections.Generic.List[string]

function Probe([string]$rec, [string]$label, [string]$fld, [string]$code, [string]$word) {
    $L.Add((Rec $rec))
    $L.Add((Con 30 $label))
    $L.Add((Con 32 '>'))
    $L.Add((Fld $fld $code 60 $word))
    # Abuts the field end position so trailing blanks in the edited value
    # (a blanked CR or minus status position) are unambiguous.
    $L.Add((Con 61 '<'))
}

$L.Add("      //*.. 1 ...+... 2 ...+... 3 ...+... 4 ...+... 5 ...+... 6 ...+... 7")
$L.Add("      //OFilename++DF..N01N02N03Excnam++++B++A++Sb+Sa+")
$L.Add("      //O..............N01N02N03Field+++++++++YB.End++PConstant/editword+++")
$L.Add("")
$L.Add((Rec 'HEAD'))
$L.Add((Con 46 'RPGVERIFY4 EDIT CODE / EDIT WORD PROBE'))
$L.Add((Rec 'TEXT'))
$L.Add((Fld 'prtTxt' '' 125 ''))
$L.Add("")

$L.Add("      * ---- Section A: edit code 1 across integer widths 1 through 9 ----")
1..9 | ForEach-Object {
    Probe "A$_" ("A$_ width $_  code 1") "g$_" '1' ''
}
$L.Add("")

$L.Add("      * ---- Section B: zero balance ----")
$b = @(
    @('z60','1'), @('z60','2'), @('z60','3'), @('z60','4'),
    @('z60','A'), @('z60','B'), @('z60','J'), @('z60','N'),
    @('z62','1'), @('z62','2'), @('z62','3'), @('z62','J')
)
for ($i = 0; $i -lt $b.Count; $i++) {
    $f = $b[$i][0]; $c = $b[$i][1]
    $shape = if ($f -eq 'z60') { '6,0' } else { '6,2' }
    Probe "B$($i+1)" ("B$($i+1) zero $shape code $c") $f $c ''
}
$L.Add("")

$L.Add("      * ---- Section C: sign placement ----")
$c1 = @('A','C','J','L','N','P')
$i = 1
foreach ($c in $c1) { Probe "C$i" ("C$i -.125 7,3 code $c") 'v73n' $c ''; $i++ }
foreach ($c in $c1) { Probe "C$i" ("C$i -125 3,0 code $c") 'v30n' $c ''; $i++ }
$L.Add("")

$L.Add("      * ---- Section E: DDS example PRICE ----")
Probe 'E1' 'E1 PRICE 5,2 EDTCDE(J)' 'price' 'J' ''
$L.Add("")

$L.Add("      * ---- Section F: edit words ----")
$f = @(
    @('F1  7,0 1234567 no stop',        'e70',  '       '),
    @('F2  7,0 35 no stop',             'e70s', '       '),
    @('F3  7,0 35 stop at units',       'e70s', '      0'),
    @('F4  7,0 0 stop at units',        'e70z', '      0'),
    @('F5  7,0 0 stop left of units',   'e70z', '     0 '),
    @('F6  7,2 12345.67 commas',        'e72',  '  ,  0.  '),
    @('F7  7,2 0.07 commas',            'e72t', '  ,  0.  '),
    @('F8  7,2 0.00 commas',            'e72z', '  ,  0.  '),
    @('F9  7,2 -12345.67 CR',           'e72n', '  ,  0.  CR'),
    @('F10 7,2 12345.67 CR',            'e72',  '  ,  0.  CR'),
    @('F11 7,2 -12345.67 minus',        'e72n', '  ,  0.  -'),
    @('F12 7,2 12345.67 minus',         'e72',  '  ,  0.  -'),
    @('F13 7,0 -35 minus no stop',      'e70n', '       -'),
    @('F14 7,2 1357.92 astfill',        'e72a', '  ,  *.  '),
    @('F15 7,2 0.00 astfill',           'e72z', '  ,  *.  '),
    @('F16 7,0 35 astfill',             'e70s', '      *'),
    @('F17 7,2 12345.67 float $',       'e72',  '    $0.  '),
    @('F18 7,2 0.05 float $',           'e72s', '    $0.  '),
    @('F19 7,2 0.00 float $',           'e72z', '    $0.  '),
    @('F20 7,2 12345.67 fixed $',       'e72',  '$    0.  '),
    @('F21 7,2 0.05 fixed $',           'e72s', '$    0.  '),
    @('F22 7,2 12345.67 $ with commas', 'e72',  '  ,  $0.  '),
    @('F23 7,0 1234567 ampersands',     'e70',  '   &  &  '),
    @('F24 7,0 1230576 constants',      'e70d', '  0/  /  ')
)
for ($i = 0; $i -lt $f.Count; $i++) {
    Probe "F$($i+1)" $f[$i][0] $f[$i][1] '' $f[$i][2]
}

# Splice: keep everything before the O-spec ruler comment, append generated block.
$src = Get-Content $path
$cut = ($src | Select-String -SimpleMatch '//*.. 1 ...+' | Select-Object -First 1).LineNumber
if (-not $cut) { throw "ruler comment not found" }
$head = $src[0..($cut - 2)]
Set-Content -Path $path -Value ($head + $L) -Encoding ASCII

Write-Host "Wrote $($L.Count) O-spec lines"
