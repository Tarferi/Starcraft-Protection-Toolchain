set out="..\src\bin.h"

echo #pragma warning(disable:4838) > %out%
echo #pragma warning(disable:4309) >> %out%

type comdlg32.ocx | bin2h.exe -c comdlg32_ocx >> %out%
type MpqCtl.ocx | bin2h.exe -c mpqctl_ocx >> %out%
type msinet.ocx | bin2h.exe -c msinet_ocx >> %out%
type "Special Protector2.exe" | bin2h.exe -c special_protector2_exe >> %out%
type vb6ko.dll | bin2h.exe -c vb6ko_dll >> %out%

