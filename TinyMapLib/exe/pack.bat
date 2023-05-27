set out="..\src\bin.h"

echo #pragma warning(disable:4838) > %out%
echo #pragma warning(disable:4309) >> %out%

type TinyMap2.exe | bin2h.exe -c tinymap2_exe >> %out%
type TinyMap2.ini | bin2h.exe -c tinymap2_ini >> %out%
