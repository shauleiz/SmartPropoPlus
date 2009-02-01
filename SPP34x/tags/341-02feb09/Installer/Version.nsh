;-----------------------------------------------------------------------------
;           Written by Shaul Eizikovich - November 2008
;-----------------------------------------------------------------------------

;-----------------------------------------------
; Function $ Macro CharStrip - removes a given char
; Author: Afrow UK
;
; Macro:
;${CharStrip} [char]   [in_string] [out_var]
;Example
;${CharStrip} "." "99.21" $R0
; $R0 == "9921"
;-----------------------------------------------
Function CharStrip
Exch $R0 #char
Exch
Exch $R1 #in string
Push $R2
Push $R3
Push $R4
 StrCpy $R2 -1
 IntOp $R2 $R2 + 1
 StrCpy $R3 $R1 1 $R2
 StrCmp $R3 "" +8
 StrCmp $R3 $R0 0 -3
  StrCpy $R3 $R1 $R2
  IntOp $R2 $R2 + 1
  StrCpy $R4 $R1 "" $R2
  StrCpy $R1 $R3$R4
  IntOp $R2 $R2 - 2
  Goto -9
  StrCpy $R0 $R1
Pop $R4
Pop $R3
Pop $R2
Pop $R1
Exch $R0
FunctionEnd


!macro CharStrip Char InStr OutVar
 Push '${InStr}'
 Push '${Char}'
  Call CharStrip
 Pop '${OutVar}'
!macroend
!define CharStrip '!insertmacro CharStrip'


;--------------------------------------------------------------------
; Macro VerAsInt - Convert dotted version string into integer
;
; Macro:
;${VerAsInt} [in_string] [out_var]
;Example
;${VerAsInt} "0.3.4.1" $R0
; $R0 == 341
;--------------------------------------------------------------------
!macro VerAsInt Ver Out
Push $R0
Push $R1
${CharStrip} "." "${Ver}" $R0 ; R0='0341'
StrCpy ${Out} $R0
StrCpy $R1 $R0 1 ; R1='0' being the first char of '0341'
${If} $R1 == '0'
      StrCpy ${Out} $R0 5 1
${EndIf}
Pop $R1
Pop $R0
!macroend
!define VerAsInt '!insertmacro VerAsInt'

;--------------------------------------------------------------------
; Macro CmpVer - Compare two dotted version strings
;
; Macro:
; ${CmpVer} Ver1 Ver2 Result
; If Ver1==Ver2 then Result=0
; If Ver1>Ver2 then Result=1
; If Ver1<Ver2 then Result=-1
; Example:
; ${CmpVer} "0.1.2.2" "0.1.2.3" "$R4"
;  $R4 == -1
;--------------------------------------------------------------------
!macro CmpVer Ver1 Ver2 Out
${VerAsInt} "${Ver1}" "$0"
${VerAsInt} "${Ver2}" "$1"
${If} $0 == $1
      IntOp ${Out} 0 + 0
${Endif}
${If} $0 > $1
      IntOp ${Out} 0 + 1
${Endif}
${If} $0 < $1
      IntOp ${Out} 0 - 1
${Endif}

!macroend
!define CmpVer '!insertmacro CmpVer'
