set out="..\src\bin.h"

echo #pragma warning(disable:4838) > %out%
echo #pragma warning(disable:4309) >> %out%

type Comdlg32.ocx | bin2h.exe -c comdlg_ocx >> %out%
type MpqCtl.ocx |bin2h.exe -c mpqctl_ocx >> %out%
type MSCOMCTL.OCX |bin2h.exe -c mscomctl_ocx >> %out%
type PROEdit.exe |bin2h.exe -c proedit_exe >> %out%
type PROEdit.ini |bin2h.exe -c proedit_ini >> %out%
type richtx32.ocx |bin2h.exe -c richtx32_ocx >> %out%
type UnitList.lst |bin2h.exe -c unitlist_lst >> %out%
