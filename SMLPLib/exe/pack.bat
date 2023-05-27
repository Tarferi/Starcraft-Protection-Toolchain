set out="..\src\bin.h"

echo #pragma warning(disable:4838) > %out%
echo #pragma warning(disable:4309) >> %out%

type "StarcraftMapLockerProject-2.5.00.exe" | bin2h.exe -c smlp_exe >> %out%